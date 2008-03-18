/* ngat_serial_arcomess_ArcomESS.c
** implementation of Java Class ngat.serial.arcomess.ArcomESS native interfaces.
** $Header: /home/cjm/cvs/arcom_ess/c/ngat_serial_arcomess_ArcomESS.c,v 1.1 2008-03-18 17:04:22 cjm Exp $
*/
/**
 * ngat_serial_arcomess_ArcomESS.c is the 'glue' between libarcom_ess, 
 * the C library used to interface to an Arcom Ethernet Serial Server, and ArcomESS.java, 
 * a Java Class to drive the server. ArcomESS specifically
 * contains all the native C routines corresponding to native methods in Java.
 * @author Chris Mottram LJMU
 * @version $Revision: 1.1 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <time.h>
#include <termios.h>
#include "arcom_ess_general.h"
#include "arcom_ess_interface.h"
#include "arcom_ess_serial.h"

/* hash definitions */
/**
 * Hash define for the size of the array holding ArcomESS Instance (jobject) maps to Arcom_ESS_Interface_Handle_T.
 * Set to 5.
 */
#define HANDLE_MAP_SIZE         (5)

/* internal structures */
/**
 * Structure holding mapping between ArcomESS Instances (jobject) to Arcom_ESS_Interface_Handle_T.
 * This means each ArcomESS object talks to one Ethernet Serial Server.
 * <dl>
 * <dt>ArcomESS_Instance_Handle</dt> <dd>jobject reference for the ArcomESS instance.</dd>
 * <dt>Interface_Handle</dt> <dd>Pointer to the Arcom_ESS_Interface_Handle_T for that ArcomESS instance.</dd>
 * </dl>
 */
struct Handle_Map_Struct
{
	jobject ArcomESS_Instance_Handle;
	Arcom_ESS_Interface_Handle_T* Interface_Handle;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ngat_serial_arcomess_ArcomESS.c,v 1.1 2008-03-18 17:04:22 cjm Exp $";

/**
 * Copy of the java virtual machine pointer, used for logging back up to the Java layer from C.
 */
static JavaVM *java_vm = NULL;
/**
 * Cached global reference to the "ngat.serial.arcomess.ArcomESS" logger, 
 * used to log back to the Java layer from C routines.
 */
static jobject logger = NULL;
/**
 * Cached reference to the "ngat.util.logging.Logger" class's log(int level,String message) method.
 * Used to log C layer log messages, in conjunction with the logger's object reference logger.
 * @see #logger
 */
static jmethodID log_method_id = NULL;
/**
 * Internal list of maps between ArcomESS jobject's (i.e. ArcomESS references), and
 * Arcom_ESS_Interface_Handle_T handles (which control which /dev/astropci port we talk to).
 * @see #Handle_Map_Struct
 * @see #HANDLE_MAP_SIZE
 */
static struct Handle_Map_Struct Handle_Map_List[HANDLE_MAP_SIZE] = 
{
	{NULL,NULL},
	{NULL,NULL},
	{NULL,NULL},
	{NULL,NULL},
	{NULL,NULL}
};

/* internal routines */
static void ArcomESS_Throw_Exception(JNIEnv *env,jobject obj,char *function_name);
static void ArcomESS_Throw_Exception_String(JNIEnv *env,jobject obj,char *function_name,char *error_string);
static void ArcomESS_Log_Handler(int level,char *string);
static int ArcomESS_Handle_Map_Add(JNIEnv *env,jobject instance,Arcom_ESS_Interface_Handle_T* interface_handle);
static int ArcomESS_Handle_Map_Delete(JNIEnv *env,jobject instance);

int ArcomESS_Handle_Map_Find(JNIEnv *env,jobject instance,Arcom_ESS_Interface_Handle_T** interface_handle);

/* ------------------------------------------------------------------------------
** 		External routines
** ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------
** 		ArcomESS C layer initialisation
** ------------------------------------------------------------------------------ */
/**
 * This routine gets called when the native library is loaded. We use this routine
 * to get a copy of the JavaVM pointer of the JVM we are running in. This is used to
 * get the correct per-thread JNIEnv context pointer in ArcomESS_Log_Handler.
 * @see #java_vm
 * @see #ArcomESS_Log_Handler
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	java_vm = vm;
	return JNI_VERSION_1_2;
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    initialiseLoggerReference<br>
 * Signature: (Lngat/util/logging/Logger;)V<br>
 * Java Native Interface implementation of ArcomESS's initialiseLoggerReference.
 * This takes the supplied logger object reference and stores it in the logger variable as a global reference.
 * The log method ID is also retrieved and stored.
 * The libarcom_ess's log handler is set to the JNI routine ArcomESS_Log_Handler.
 * The libarcom_ess's log filter function is set bitwise.
 * @param l The ArcomESS's "ngat.serial.arcomess.ArcomESS" logger.
 * @see #ArcomESS_Log_Handler
 * @see #logger
 * @see #log_method_id
 * @see arcom_ess_general.html#Arcom_ESS_Log_Filter_Level_Bitwise
 * @see arcom_ess_general.html#Arcom_ESS_Set_Log_Handler_Function
 * @see arcom_ess_general.html#Arcom_ESS_Set_Log_Filter_Function
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_initialiseLoggerReference(JNIEnv *env,jobject obj,
										    jobject l)
{
	jclass cls = NULL;

/* save logger instance */
	logger = (*env)->NewGlobalRef(env,l);
/* get the ngat.util.logging.Logger class */
	cls = (*env)->FindClass(env,"ngat/util/logging/Logger");
	/* if the class is null, one of the following exceptions occured:
	** ClassFormatError,ClassCircularityError,NoClassDefFoundError,OutOfMemoryError */
	if(cls == NULL)
		return;
/* get relevant method id to call */
/* log(int level,java/lang/String message) */
	log_method_id = (*env)->GetMethodID(env,cls,"log","(ILjava/lang/String;)V");
	if(log_method_id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
	/* Make the C layer log back to the Java logger, using ArcomESS_Log_Handler JNI routine.  */
	Arcom_ESS_Set_Log_Handler_Function(ArcomESS_Log_Handler);
	/* Make the filtering bitwise, as expected by the C layer */
	Arcom_ESS_Set_Log_Filter_Function(Arcom_ESS_Log_Filter_Level_Bitwise);
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    finaliseLoggerReference<br>
 * Signature: ()V<br>
 * This native method is called from ArcomESS's finaliser method. It removes the global reference to
 * logger.
 * @see #logger
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_finaliseLoggerReference(JNIEnv *env, jobject obj)
{
	(*env)->DeleteGlobalRef(env,logger);
}

/* ------------------------------------------------------------------------------
** 		arcom_ess_general.c
** ------------------------------------------------------------------------------ */
/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Set_Log_Filter_Level<br>
 * Signature: (I)V<br>
 * @see arcom_ess_general.html#Arcom_ESS_Set_Log_Filter_Level
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Set_1Log_1Filter_1Level(JNIEnv *env,jobject obj,
										       jint level)
{
	Arcom_ESS_Set_Log_Filter_Level(level);
}

/* ------------------------------------------------------------------------------
** 		arcom_ess_interface.c
** ------------------------------------------------------------------------------ */
/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Handle_Create<br>
 * Signature: ()V<br>
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_Create
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Handle_Map_Add
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Handle_1Create(JNIEnv *env, 
												jobject obj)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	int retval;

	retval = Arcom_ESS_Interface_Handle_Create(&handle);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Handle_Create");
		return;
	}
	/* map this (obj) to handle */
	retval = ArcomESS_Handle_Map_Add(env,obj,handle);
	if(retval == FALSE)
	{
		/* An error should have been thrown by ArcomESS_Handle_Map_Add */
		return;
	}
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Handle_Destroy<br>
 * Signature: ()V<br>
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_Destroy
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Handle_Map_Find
 * @see #ArcomESS_Handle_Map_Delete
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Handle_1Destroy(JNIEnv *env, 
												 jobject obj)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	int retval;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	retval = Arcom_ESS_Interface_Handle_Destroy(&handle);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Handle_Destroy");
		return;
	}
	/* remove mapping from ArcomESS instance to interface handle */
	retval = ArcomESS_Handle_Map_Delete(env,obj);
	if(retval == FALSE)
	{
		/* ArcomESS_Handle_Map_Delete should have thrown an error if it fails */
		return;
	}
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Open<br>
 * Signature: (ILjava/lang/String;I)V<br>
 * @see arcom_ess_interface.html#ARCOM_ESS_INTERFACE_DEVICE_ID
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Open
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Handle_Map_Find
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Open(JNIEnv *env, jobject obj, 
					     jint device_id, jstring device_name_jstring, jint port_number)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	const char *device_name = NULL;
	int retval;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	/* Get the device name from a java string to a c null terminated string
	** If the java String is null the device_name should be null as well */
	if(device_name_jstring != NULL)
		device_name = (*env)->GetStringUTFChars(env,device_name_jstring,0);
	retval = Arcom_ESS_Interface_Open((enum ARCOM_ESS_INTERFACE_DEVICE_ID)device_id,(char*)device_name,
				    (int)port_number,handle);
	/* If we created the device_name string we need to free the memory it uses */
	if(device_name_jstring != NULL)
		(*env)->ReleaseStringUTFChars(env,device_name_jstring,device_name);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Open");
		return;
	}
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Close<br>
 * Signature: ()V<br>
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Handle_Map_Find
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Close
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Close(JNIEnv *env, jobject obj)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	int retval;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	retval = Arcom_ESS_Interface_Close(handle);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Close");
		return;
	}
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Read<br>
 * Signature: ()Ljava/lang/String;<br>
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Handle_Map_Find
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Read
 */
JNIEXPORT jstring JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Read(JNIEnv *env, jobject obj)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	char buff[256];
	int retval,bytes_read;
	jstring retstring;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return NULL; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	retval = Arcom_ESS_Interface_Read(handle,buff,255,&bytes_read);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Read");
		return NULL;
	}
	/* construct a jstring from the read data */
	buff[bytes_read] = '\0';
	retstring = (*env)->NewStringUTF(env,buff);
	return retstring;
}

/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Interface_Write<br>
 * Signature: (Ljava/lang/String;)V<br>
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Throw_Exception_String
 * @see #ArcomESS_Handle_Map_Find
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Write
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Interface_1Write(JNIEnv *env, jobject obj, 
										       jstring s_jstring)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	int retval;
	const char *c_string = NULL;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	/* Get the string from a java string to a c null terminated string
	** If the java String is null the string should be null as well */
	if(s_jstring != NULL)
		c_string = (*env)->GetStringUTFChars(env,s_jstring,0);
	if(c_string == NULL)
	{
		ArcomESS_Throw_Exception_String(env,obj,"Arcom_ESS_Interface_Write",
						"C String was NULL.");
		return;
	}
	/* do write */
	retval = Arcom_ESS_Interface_Write(handle,(void *)c_string,strlen(c_string));
	/* If we created the string we need to free the memory it uses */
	if(s_jstring != NULL)
		(*env)->ReleaseStringUTFChars(env,s_jstring,c_string);
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Interface_Write");
		return;
	}
}

/* ------------------------------------------------------------------------------
** 		arcom_ess_serial.c
** ------------------------------------------------------------------------------ */
/**
 * Class:     ngat_serial_arcomess_ArcomESS<br>
 * Method:    Arcom_ESS_Serial_Baud_Rate_Set<br>
 * Signature: (I)V<br>
 * @see #ArcomESS_Throw_Exception
 * @see #ArcomESS_Throw_Exception_String
 * @see #ArcomESS_Handle_Map_Find
 * @see arcom_ess_interface.html#Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Baud_Rate_Set
 */
JNIEXPORT void JNICALL Java_ngat_serial_arcomess_ArcomESS_Arcom_1ESS_1Serial_1Baud_1Rate_1Set(JNIEnv *env, jobject obj,
											      jint baud_rate)
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	char buff[256];
	int retval;

	/* get interface handle from ArcomESS instance map */
	if(!ArcomESS_Handle_Map_Find(env,obj,&handle))
		return; /* ArcomESS_Handle_Map_Find throws an exception on failure */
	/* set baud rate */
	if(baud_rate == 1200)
		retval = Arcom_ESS_Serial_Baud_Rate_Set(B1200);
	else if(baud_rate == 4800)
		retval = Arcom_ESS_Serial_Baud_Rate_Set(B4800);
	else if(baud_rate == 9600)
		retval = Arcom_ESS_Serial_Baud_Rate_Set(B9600);
	else if (baud_rate == 19200)
		retval = Arcom_ESS_Serial_Baud_Rate_Set(B19200);
	else
	{
		sprintf(buff,"Baud Rate %d unsupported.",baud_rate);
		ArcomESS_Throw_Exception_String(env,obj,"Arcom_ESS_Serial_Baud_Rate_Set",buff);
		return;
	}
	/* if an error occured throw an exception. */
	if(retval == FALSE)
	{
		ArcomESS_Throw_Exception(env,obj,"Arcom_ESS_Serial_Baud_Rate_Set");
		return;
	}
}

/* ------------------------------------------------------------------------------
** 		Internal routines
** ------------------------------------------------------------------------------ */
/**
 * This routine throws an exception. The error generated is from the error codes in libarcom_ess, 
 * it assumes another routine has generated an error and this routine packs this error into an exception to return
 * to the Java code, using ArcomESS_Throw_Exception_String. The total length of the error string should
 * not be longer than ARCOM_ESS_ERROR_LENGTH. A new line is added to the start of the error string,
 * so that the error string returned from libarcom_ess is formatted properly.
 * @param env The JNI environment pointer.
 * @param function_name The name of the function in which this exception is being generated for.
 * @param obj The instance of ArcomESS that threw the error.
 * @see arcom_ess_general.html#Arcom_ESS_Error_To_String
 * @see #ArcomESS_Throw_Exception_String
 * @see #ARCOM_ESS_ERROR_LENGTH
 */
static void ArcomESS_Throw_Exception(JNIEnv *env,jobject obj,char *function_name)
{
	char error_string[ARCOM_ESS_ERROR_LENGTH];

	strcpy(error_string,"\n");
	Arcom_ESS_Error_To_String(error_string+strlen(error_string));
	ArcomESS_Throw_Exception_String(env,obj,function_name,error_string);
}

/**
 * This routine throws an exception of class ngat/serial/arcomess/ArcomESSNativeException. 
 * This is used to report all libarcom_ess error messages back to the Java layer.
 * @param env The JNI environment pointer.
 * @param obj The instance of ArcomESS that threw the error.
 * @param function_name The name of the function in which this exception is being generated for.
 * @param error_string The string to pass to the constructor of the exception.
 */
static void ArcomESS_Throw_Exception_String(JNIEnv *env,jobject obj,char *function_name,char *error_string)
{
	jclass exception_class = NULL;
	jobject exception_instance = NULL;
	jstring error_jstring = NULL;
	jmethodID mid;
	int retval;

	exception_class = (*env)->FindClass(env,"ngat/serial/arcomess/ArcomESSNativeException");
	if(exception_class != NULL)
	{
	/* get ArcomESSNativeException constructor */
		mid = (*env)->GetMethodID(env,exception_class,"<init>",
					  "(Ljava/lang/String;Lngat/serial/arcomess/ArcomESS;)V");
		if(mid == 0)
		{
			/* One of the following exceptions has been thrown:
			** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
			fprintf(stderr,"ArcomESS_Throw_Exception_String:GetMethodID failed:%s:%s\n",function_name,
				error_string);
			return;
		}
	/* convert error_string to JString */
		error_jstring = (*env)->NewStringUTF(env,error_string);
	/* call constructor */
		exception_instance = (*env)->NewObject(env,exception_class,mid,error_jstring,obj);
		if(exception_instance == NULL)
		{
			/* One of the following exceptions has been thrown:
			** InstantiationException, OutOfMemoryError */
			fprintf(stderr,"ArcomESS_Throw_Exception_String:NewObject failed %s:%s\n",
				function_name,error_string);
			return;
		}
	/* throw instance */
		retval = (*env)->Throw(env,(jthrowable)exception_instance);
		if(retval !=0)
		{
			fprintf(stderr,"ArcomESS_Throw_Exception_String:Throw failed %d:%s:%s\n",retval,
				function_name,error_string);
		}
	}
	else
	{
		fprintf(stderr,"ArcomESS_Throw_Exception_String:FindClass failed:%s:%s\n",function_name,
			error_string);
	}
}

/**
 * libarcom_ess Log Handler for the Java layer interface. 
 * This calls the ngat.serial.arcomess.ArcomESS logger's 
 * log(int level,String message) method with the parameters supplied to this routine.
 * If the logger instance is NULL, or the log_method_id is NULL the call is not made.
 * Otherwise, A java.lang.String instance is constructed from the string parameter,
 * and the JNI CallVoidMEthod routine called to call log().
 * @param level The log level of the message.
 * @param string The message to log.
 * @see #java_vm
 * @see #logger
 * @see #log_method_id
 */
static void ArcomESS_Log_Handler(int level,char *string)
{
	JNIEnv *env = NULL;
	jstring java_string = NULL;

	if(logger == NULL)
	{
		fprintf(stderr,"ArcomESS_Log_Handler:logger was NULL (%d,%s).\n",level,string);
		return;
	}
	if(log_method_id == NULL)
	{
		fprintf(stderr,"ArcomESS_Log_Handler:log_method_id was NULL (%d,%s).\n",level,string);
		return;
	}
	if(java_vm == NULL)
	{
		fprintf(stderr,"ArcomESS_Log_Handler:java_vm was NULL (%d,%s).\n",level,string);
		return;
	}
/* get java env for this thread */
	(*java_vm)->AttachCurrentThread(java_vm,(void**)&env,NULL);
	if(env == NULL)
	{
		fprintf(stderr,"ArcomESS_Log_Handler:env was NULL (%d,%s).\n",level,string);
		return;
	}
	if(string == NULL)
	{
		fprintf(stderr,"ArcomESS_Log_Handler:string (%d) was NULL.\n",level);
		return;
	}
/* convert C to Java String */
	java_string = (*env)->NewStringUTF(env,string);
/* call log method on logger instance */
	(*env)->CallVoidMethod(env,logger,log_method_id,(jint)level,java_string);
}

/**
 * Routine to add a mapping from the ArcomESS instance instance to the opened Arcom_ESS Interface Handle
 * interface_handle, in the Handle_Map_List.
 * @param instance The ArcomESS instance.
 * @param interface_handle The interface handle.
 * @return The routine returns TRUE if the map is added (or updated), FALSE if there was no room left
 *         in the mapping list. 
 *         ArcomESS_Throw_Exception_String is used to throw a Java exception if the routine returns FALSE.
 * @see #HANDLE_MAP_SIZE
 * @see #Handle_Map_List
 * @see #ArcomESS_Throw_Exception_String
 */
static int ArcomESS_Handle_Map_Add(JNIEnv *env,jobject instance,Arcom_ESS_Interface_Handle_T* interface_handle)
{
	int i,done;
	jobject global_instance = NULL;

	/* does the map already exist? */
	i = 0;
	done = FALSE;
	while((i < HANDLE_MAP_SIZE)&&(done == FALSE))
	{
		if((*env)->IsSameObject(env,Handle_Map_List[i].ArcomESS_Instance_Handle,instance))
			done = TRUE;
		else
			i++;
	}
	if(done == TRUE)/* found an existing interface handle for this ArcomESS instance */
	{
		/* update handle */
		Handle_Map_List[i].Interface_Handle = interface_handle;
	}
	else
	{
		/* look for a blank index to put the map */
		i = 0;
		done = FALSE;
		while((i < HANDLE_MAP_SIZE)&&(done == FALSE))
		{
			if(Handle_Map_List[i].ArcomESS_Instance_Handle == NULL)
				done = TRUE;
			else
				i++;
		}
		if(done == FALSE)
		{
			ArcomESS_Throw_Exception_String(env,instance,"ArcomESS_Handle_Map_Add",
							  "No empty slots in handle map.");
			return FALSE;
		}
		/* index i is free, add handle map here */
		global_instance = (*env)->NewGlobalRef(env,instance);
		if(global_instance == NULL)
		{
			ArcomESS_Throw_Exception_String(env,instance,"ArcomESS_Handle_Map_Add",
							  "Failed to create Global reference of instance.");
			return FALSE;
		}
		fprintf(stdout,"ArcomESS_Handle_Map_Add:Adding instance %p with handle %p at map index %d.\n",
			(void*)global_instance,(void*)interface_handle,i);
		Handle_Map_List[i].ArcomESS_Instance_Handle = global_instance;
		Handle_Map_List[i].Interface_Handle = interface_handle;
	}
	return TRUE;
}

/**
 * Routine to delete a mapping from the ArcomESS instance instance to the opened Arcom_ESS Interface Handle
 * interface_handle, in the Handle_Map_List.
 * @param instance The ArcomESS instance to remove from the list.
 * @return The routine returns TRUE if the map is deleted (or updated), FALSE if the mapping could not be found
 *         in the mapping list.
 *         ArcomESS_Throw_Exception_String is used to throw a Java exception if the routine returns FALSE.
 * @see #HANDLE_MAP_SIZE
 * @see #Handle_Map_List
 * @see #ArcomESS_Throw_Exception_String
 */
static int ArcomESS_Handle_Map_Delete(JNIEnv *env,jobject instance)
{
	int i,done;

  	/* does the map already exist? */
	i = 0;
	done = FALSE;
	while((i < HANDLE_MAP_SIZE)&&(done == FALSE))
	{
		if((*env)->IsSameObject(env,Handle_Map_List[i].ArcomESS_Instance_Handle,instance))
			done = TRUE;
		else
			i++;
	}
	if(done == FALSE)
	{
		ArcomESS_Throw_Exception_String(env,instance,"ArcomESS_Handle_Map_Delete",
						  "Failed to find ArcomESS instance in handle map.");
		return FALSE;
	}
	/* found an existing interface handle for this ArcomESS instance at index i */
	/* delete this map at index i */
	fprintf(stdout,"ArcomESS_Handle_Map_Delete:Deleting instance %p with handle %p at map index %d.\n",
		(void*)Handle_Map_List[i].ArcomESS_Instance_Handle,(void*)Handle_Map_List[i].Interface_Handle,i);
	(*env)->DeleteGlobalRef(env,Handle_Map_List[i].ArcomESS_Instance_Handle);
	Handle_Map_List[i].ArcomESS_Instance_Handle = NULL;
	Handle_Map_List[i].Interface_Handle = NULL;
	return TRUE;
}

/**
 * Routine to find a mapping from the ArcomESS instance instance to the opened Arcom_ESS Interface Handle
 * interface_handle, in the Handle_Map_List.
 * This routine is now external - so libraryies linking to this one can also access a Arcom_ESS_Interface_Handle_T
 * from an ArcomESS instance.
 * @param instance The ArcomESS instance.
 * @param interface_handle The address of an interface handle, to fill with the interface handle for
 *        this ArcomESS instance, if one is successfully found.
 * @return The routine returns TRUE if the mapping is found and returned,, FALSE if there was no mapping
 *         for this ArcomESS instance, or the interface_handle pointer was NULL.
 *         ArcomESS_Throw_Exception_String is used to throw a Java exception if the routine returns FALSE.
 * @see #HANDLE_MAP_SIZE
 * @see #Handle_Map_List
 * @see #ArcomESS_Throw_Exception_String
 */
int ArcomESS_Handle_Map_Find(JNIEnv *env,jobject instance,Arcom_ESS_Interface_Handle_T** interface_handle)
{
	int i,done;

	if(interface_handle == NULL)
	{
		ArcomESS_Throw_Exception_String(env,instance,"ArcomESS_Handle_Map_Find",
						  "interface handle was NULL.");
		return FALSE;
	}
	i = 0;
	done = FALSE;
	while((i < HANDLE_MAP_SIZE)&&(done == FALSE))
	{
		if((*env)->IsSameObject(env,Handle_Map_List[i].ArcomESS_Instance_Handle,instance))
			done = TRUE;
		else
			i++;
	}
	if(done == FALSE)
	{
		fprintf(stdout,"ArcomESS_Handle_Map_Find:Failed to find instance %p.\n",(void*)instance);
		ArcomESS_Throw_Exception_String(env,instance,"ArcomESS_Handle_Map_Find",
						  "ArcomESS instance handle was not found.");
		return FALSE;
	}
	(*interface_handle) = Handle_Map_List[i].Interface_Handle;
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
*/
