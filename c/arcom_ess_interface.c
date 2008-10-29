/* arcom_ess_interface.c
** Arcom ESS interface library
** $Header: /home/cjm/cvs/arcom_ess/c/arcom_ess_interface.c,v 1.2 2008-10-29 14:43:54 cjm Exp $
*/
/**
 * Serial device independant interface routines.
 * Set the interface to be either Serial or Socket and Open/Close/Read/Write calls are directed as appropriate.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>   /* Error number definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef ARCOM_ESS_MUTEXED
#include <pthread.h>
#endif
#include "arcom_ess_general.h"
#include "arcom_ess_serial.h"
#include "arcom_ess_socket.h"

/* data types */
/**
 * Interface Handle structure.
 * <ul>
 * <li><b>Interface_Device</b> Enumeration of type ARCOM_ESS_INTERFACE_DEVICE_ID, used to determine which type of
 *        handle (serial or socket) we are talking to.
 * <li><b>Handle</b> A union of:
 *     <ul>
 *     <li><b>Serial</b> Of type Arcom_ESS_Serial_Handle_T, holding serial specific data.
 *     <li><b>Socket</b> Of type Arcom_ESS_Socket_Handle_T, holding socket specific data.
 *     </ul>
 * <li><b>Mutex</b> Optionally compiled mutex locking over sending commands down the comms link 
 *                and receiving a reply.
 * </ul>
 * @see #ARCOM_ESS_INTERFACE_DEVICE_ID
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Handle_T
 */
struct Arcom_ESS_Interface_Handle_Struct
{
	enum ARCOM_ESS_INTERFACE_DEVICE_ID Interface_Device;
	union
	{
		Arcom_ESS_Serial_Handle_T Serial;
		Arcom_ESS_Socket_Handle_T Socket;
	} Handle;
#ifdef ARCOM_ESS_MUTEXED
	pthread_mutex_t Mutex;
#endif
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: arcom_ess_interface.c,v 1.2 2008-10-29 14:43:54 cjm Exp $";

/* =======================================
**  external functions 
** ======================================= */
/**
 * Routine to allocate memeory for the interface handle, and initialise the mutex.
 * @param handle The address of a pointer to allocate the handle.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 */
int Arcom_ESS_Interface_Handle_Create(Arcom_ESS_Interface_Handle_T **handle)
{
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 100;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Handle_Create: handle was NULL.");
		return FALSE;
	}
	/* allocate handle */
	(*handle) = (Arcom_ESS_Interface_Handle_T *)malloc(sizeof(Arcom_ESS_Interface_Handle_T));
	if((*handle) == NULL)
	{
		Arcom_ESS_Error_Number = 103;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Handle_Create: Failed to allocate handle.");
		return FALSE;
	}
	/* initialise mutex - according to man page, pthread_mutex_init always returns 0. */
#ifdef ARCOM_ESS_MUTEXED
	pthread_mutex_init(&((*handle)->Mutex),NULL);
#endif
	return TRUE;
}

/**
 * Routine to open a connection to the specified interface.
 * @param device_id Which sort of device to open, should be one of: ARCOM_ESS_INTERFACE_DEVICE_SERIAL, 
 *        ARCOM_ESS_INTERFACE_DEVICE_SOCKET.
 * @param device_name The name of the device. For serial devices, this is something like "/dev/ttyS0", 
 *        for socket devices this is the IP address or a resolvable name of the Arcom ESS (i.e. 150.204.240.115).
 * @param port_number The port number to communicate over (only valid for ARCOM_ESS_INTERFACE_DEVICE_SOCKET) i.e. 3040.
 * @param handle A Arcom_ESS_Interface_Handle_T pointer to store the opening information.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH
 * @see #ARCOM_ESS_INTERFACE_DEVICE_ID
 * @see #Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Open
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Open
 */
int Arcom_ESS_Interface_Open(enum ARCOM_ESS_INTERFACE_DEVICE_ID device_id,char *device_name,int port_number,
			      Arcom_ESS_Interface_Handle_T *handle)
{
	if(!ARCOM_ESS_INTERFACE_IS_INTERFACE_DEVICE(device_id))
	{
		Arcom_ESS_Error_Number = 101;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open: Illegal interface device ID %d.",device_id);
		return FALSE;
	}
	if(device_name == NULL)
	{
		Arcom_ESS_Error_Number = 102;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open: device_name was NULL.");
		return FALSE;
	}
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 120;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open: handle was NULL.");
		return FALSE;
	}
	/* set the device type */
	handle->Interface_Device = device_id;
	/* call the device specific open routine */
	switch(handle->Interface_Device)
	{
		case ARCOM_ESS_INTERFACE_DEVICE_SERIAL:
			if(strlen(device_name) > ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH)
			{
				Arcom_ESS_Error_Number = 104;
				sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open: Device name was too long (%d).",
					strlen(device_name));
				return FALSE;
			}
			strcpy(handle->Handle.Serial.Device_Name,device_name);
			return Arcom_ESS_Serial_Open(&(handle->Handle.Serial));
		case ARCOM_ESS_INTERFACE_DEVICE_SOCKET:
			if(strlen(device_name) > ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH)
			{
				Arcom_ESS_Error_Number = 105;
				sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open: Device name was too long (%d).",
					strlen(device_name));
				return FALSE;
			}
			strcpy(handle->Handle.Socket.Address,device_name);
			handle->Handle.Socket.Port_Number = port_number;
			return Arcom_ESS_Socket_Open(&(handle->Handle.Socket));
		default:
			Arcom_ESS_Error_Number = 106;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Open failed:Illegal device selected(%d).",
				handle->Interface_Device);
			return FALSE;
	}
}

/**
 * Routine to close a connection to the specified interface.
 * @param handle The connection information spcfying which connection to close.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Close
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Close
 */
int Arcom_ESS_Interface_Close(Arcom_ESS_Interface_Handle_T *handle)
{
	/* check parameters */
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 107;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Close: handle was NULL.");
		return FALSE;
	}
	/* call the device specific close routine */
	switch(handle->Interface_Device)
	{
		case ARCOM_ESS_INTERFACE_DEVICE_SERIAL:
			if(!Arcom_ESS_Serial_Close(&(handle->Handle.Serial)))
				return FALSE;
			break;
		case ARCOM_ESS_INTERFACE_DEVICE_SOCKET:
			if(!Arcom_ESS_Socket_Close(&(handle->Handle.Socket)))
				return FALSE;
			break;
		default:
			Arcom_ESS_Error_Number = 109;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Close failed:Illegal device selected(%d).",
				handle->Interface_Device);
			return FALSE;
	}
	return TRUE;
}

/**
 * Routine to destroy the specified handle.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 */
int Arcom_ESS_Interface_Handle_Destroy(Arcom_ESS_Interface_Handle_T **handle)
{
	/* check parameters */
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 121;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Handle_Destroy: handle was NULL.");
		return FALSE;
	}
	if((*handle) == NULL)
	{
		Arcom_ESS_Error_Number = 108;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Handle_Destroy: handle pointer was NULL.");
		return FALSE;
	}
	/* free alocated handle */
	free((*handle));
	(*handle) = NULL;
	return TRUE;

}

/**
 * Routine to write data to an open connection to the specified interface.
 * @param handle The handle specifying which connection to write to.
 * @param message A buffer containing some data to be written.
 * @param message_length The length of the buffer.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Write
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Write
 */
int Arcom_ESS_Interface_Write(Arcom_ESS_Interface_Handle_T *handle,void* message,size_t message_length)
{
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 110;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Write: handle was NULL.");
		return FALSE;
	}
	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 111;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Write: message was NULL.");
		return FALSE;
	}
	/* call the device specific close routine */
	switch(handle->Interface_Device)
	{
		case ARCOM_ESS_INTERFACE_DEVICE_SERIAL:
			if(!Arcom_ESS_Serial_Write(handle->Handle.Serial,message,message_length))
				return FALSE;
			break;
		case ARCOM_ESS_INTERFACE_DEVICE_SOCKET:
			if(!Arcom_ESS_Socket_Write(handle->Handle.Socket,message,message_length))
				return FALSE;
			break;
		default:
			Arcom_ESS_Error_Number = 112;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Write failed:Illegal device selected(%d).",
				handle->Interface_Device);
			return FALSE;
	}
	return TRUE;
}

/**
 * Routine to read data to an open connection to the specified interface.
 * @param handle The handle specifying which connection to write to.
 * @param message A pointer to the buffer to write read data into.
 * @param message_length The length of the buffer.
 * @param bytes_read The address of an integer to fill in how many bytes were read. This can be NULL,
 *        in which case the number of bytes read is not returned.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Read
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Read
 */
int Arcom_ESS_Interface_Read(Arcom_ESS_Interface_Handle_T *handle,void* message,int message_length,int* bytes_read)
{
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 113;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Read: handle was NULL.");
		return FALSE;
	}
	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 114;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Read: message was NULL.");
		return FALSE;
	}
	/* call the device specific close routine */
	switch(handle->Interface_Device)
	{
		case ARCOM_ESS_INTERFACE_DEVICE_SERIAL:
			if(!Arcom_ESS_Serial_Read(handle->Handle.Serial,message,message_length,bytes_read))
				return FALSE;
			break;
		case ARCOM_ESS_INTERFACE_DEVICE_SOCKET:
			if(!Arcom_ESS_Socket_Read(handle->Handle.Socket,message,message_length,bytes_read))
				return FALSE;
			break;
		default:
			Arcom_ESS_Error_Number = 115;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Read failed:Illegal device selected(%d).",
				handle->Interface_Device);
			return FALSE;
	}
	return TRUE;
}

/**
 * Try to flush any unread data from the file descriptor by repeated read's until read returns 0.
 * We wait a small delay before reading in case  some data is in the process of arriving.
 * The flushing is currently only implemented for socket connections.
 * @param handle The handle specifying which connection to flush.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Arcom_ESS_Interface_Handle_T
 * @see arcom_ess_socket.html#Arcom_ESS_Socket_Flush
 * @see arcom_ess_serial.html#Arcom_ESS_Serial_Read
 */
int Arcom_ESS_Interface_Flush(Arcom_ESS_Interface_Handle_T *handle)
{
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 122;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Flush: handle was NULL.");
		return FALSE;
	}
	/* call the device specific routine */
	switch(handle->Interface_Device)
	{
		case ARCOM_ESS_INTERFACE_DEVICE_SERIAL:
			/* do nothing for flush */
			break;
		case ARCOM_ESS_INTERFACE_DEVICE_SOCKET:
			if(!Arcom_ESS_Socket_Flush(handle->Handle.Socket))
				return FALSE;
			break;
		default:
			Arcom_ESS_Error_Number = 123;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Flush failed:Illegal device selected(%d).",
				handle->Interface_Device);
			return FALSE;
	}
	return TRUE;
}

#ifdef ARCOM_ESS_MUTEXED
/**
 * Routine to lock the comms access mutex. This will block until the mutex has been acquired,
 * unless an error occurs. The mutex is held within the interface handle, i.e. each connection to a PLC
 * has it's own mutex. This allows us to talk to several different PLCs at the same time, but each PLC
 * can only have one thread talking to it at any one time.
 * @param handle The handle specifying which connection (PLC) to lock access to.
 * @return Returns TRUE if the mutex has been locked for access by this thread,
 * 	FALSE if an error occured.
 * @see #Arcom_ESS_Interface_Handle_t
 */
int Arcom_ESS_Interface_Mutex_Lock(Arcom_ESS_Interface_Handle_T *handle)
{
	int error_number;

	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 116;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Mutex_Lock: handle was NULL.");
		return FALSE;
	}
	error_number = pthread_mutex_lock(&(handle->Mutex));
	if(error_number != 0)
	{
		Arcom_ESS_Error_Number = 117;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Mutex_Lock:Mutex lock failed '%d'.",error_number);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to unlock the comms access mutex. 
 * @param handle The handle specifying which connection (PLC) to unlock access to.
 * @return Returns TRUE if the mutex has been unlocked, FALSE if an error occured.
 * @see #Arcom_ESS_Interface_Handle_t
 */
int Arcom_ESS_Interface_Mutex_Unlock(Arcom_ESS_Interface_Handle_T *handle)
{
	int error_number;

	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 118;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Mutex_Unlock: handle was NULL.");
		return FALSE;
	}
	error_number = pthread_mutex_unlock(&(handle->Mutex));
	if(error_number != 0)
	{
		Arcom_ESS_Error_Number = 119;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Interface_Mutex_Unlock:Mutex unlock failed '%d'.",error_number);
		return FALSE;
	}
	return TRUE;
}
#endif

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2008/03/18 17:04:22  cjm
** Initial revision
**
*/
