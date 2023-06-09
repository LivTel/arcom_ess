/* arcom_ess_socket.c
** Arcom ESS interface library
** $Header: /home/cjm/cvs/arcom_ess/c/arcom_ess_socket.c,v 1.6 2011-01-05 14:29:20 cjm Exp $
*/
/**
 * Basic operations, open close etc. For driving the serial device using an Arcom ethernet-RS232 ESS 
 * serial socket server.
 * @author Chris Mottram
 * @version $Revision: 1.6 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L
/**
 * Define BSD Source to get BSD prototypes, including FNDELAY.
 */
#define _BSD_SOURCE
#include <arpa/inet.h>
#include <errno.h>   /* Error number definitions */
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h> /* TCP_NODELAY constant */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <unistd.h> /* UNIX standard function definitions */
#include "log_udp.h"
#include "arcom_ess_general.h"
#include "arcom_ess_socket.h"

/* internal hash defines */
/**
 * How long to pause between read attempts in milliseconds.
 * Note this time depends on the controlled device response time, 
 * but also on the Demark/Timeout config in the Arcom ESS
 * (currently 100/200). 
 * @see #SOCKET_READ_TIMEOUT_COUNT
 * @see #Arcom_ESS_Socket_Read
 */
#define SOCKET_READ_PAUSE_MS         (100)

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: arcom_ess_socket.c,v 1.6 2011-01-05 14:29:20 cjm Exp $";

/* internal functions */
static int Socket_Get_Host_By_Name(char *class,char *source,const char *name,char **host_addr_zero);

/* ----------------------------------------------------------------
** external functions 
** ---------------------------------------------------------------- */

/**
 * Open the socket device, and configure accordingly. FInish with a call to Arcom_ESS_Socket_Flush to flush
 * out any residual data on the socket.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle A pointer to an instance of Arcom_ESS_Socket_Handle_T containing the address and port number to open.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Socket_Handle_T
 * @see #Arcom_ESS_Socket_Flush
 * @see #Socket_Get_Host_By_Name
 */
int Arcom_ESS_Socket_Open(char *class,char *source,Arcom_ESS_Socket_Handle_T *handle)
{
	char host_ip[256];
	struct sockaddr_in address;
	char *host_address_ptr = NULL;
	int open_errno,retval;
#ifdef ARCOM_ESS_TCP_NODELAY
	int flag;
#endif
	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 412;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Open: handle was NULL.");
		return FALSE;
	}
	/* Open socket. */
#if LOGGING > 0
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Arcom_ESS_Socket_Open(%s,%d).",handle->Address,
		       handle->Port_Number);
#endif /* LOGGING */
	handle->Socket_Fd = socket(PF_INET,SOCK_STREAM,0);
	if(handle->Socket_Fd < 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 401;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Open: Address %s, port %d failed to open (%d).",
			handle->Address,handle->Port_Number,open_errno);
		return FALSE;
	}
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,"Arcom_ESS_Socket_Open with FD %d.",
		       handle->Socket_Fd);
#endif /* LOGGING */
	/* find host address */
#if LOGGING > 1
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Arcom_ESS_Socket_Open:Find host address.");
#endif /* LOGGING */
	if(!Socket_Get_Host_By_Name(class,source,handle->Address,&host_address_ptr))
		return FALSE;
	strcpy(host_ip,inet_ntoa(*(struct in_addr *)(host_address_ptr)));
	if(host_address_ptr != NULL)
		free(host_address_ptr);
	address.sin_family = PF_INET;
	address.sin_addr.s_addr = inet_addr(host_ip);
	address.sin_port = htons(handle->Port_Number);
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_INTERMEDIATE,
			     "Arcom_ESS_Socket_Open:trying to connect to %s[%s:%d].",
			     handle->Address,host_ip,handle->Port_Number);
#endif /* LOGGING */
	if(connect(handle->Socket_Fd,(struct sockaddr *)&(address),sizeof(address)) == -1)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 403;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Open: Connect %s:%d failed (%s,%d).",
			host_ip,handle->Port_Number,strerror(open_errno),open_errno);
		return FALSE;
	}
	/* make non-blocking */
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Open:Set non-blocking.");
#endif /* LOGGING */
	retval = fcntl(handle->Socket_Fd, F_SETFL, FNDELAY);
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 410;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Open: fcntl failed (%d).",open_errno);
		return FALSE;
	}
	/* disable Nagle's algorithm - might stop timeouts */
#ifdef ARCOM_ESS_TCP_NODELAY
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Open:Set TCP NODELAY.");
#endif /* LOGGING */
	flag = 1;
	retval = setsockopt(Socket_Data.Socket_Fd,            /* socket affected */
			    IPPROTO_TCP,     /* set option at TCP level */
			    TCP_NODELAY,     /* name of option */
			    (char *) &flag,  /* the cast is historical cruft */
			    sizeof(int));    /* length of option value */
	if(retval < 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 411;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Open: setsockopt failed (%d).",open_errno);
		return FALSE;
	}
#endif
	/* flush any unread data from the Arcom ESS */
	if(!Arcom_ESS_Socket_Flush(class,source,*handle))
		return FALSE;
#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_INTERMEDIATE,"Arcom_ESS_Socket_Open:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to close a previously open socket device. 
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle A pointer to an instance of Arcom_ESS_Socket_Handle_T containing the socket to close.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Socket_Handle_T
 */
int Arcom_ESS_Socket_Close(char *class,char *source,Arcom_ESS_Socket_Handle_T *handle)
{
	int retval,close_errno;

#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Close:Started.");
#endif /* LOGGING */
#if LOGGING > 1
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Close:Closing file descriptor.");
#endif /* LOGGING */
	retval = close(handle->Socket_Fd);
	if(retval < 0)
	{
		close_errno = errno;
		Arcom_ESS_Error_Number = 404;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Close: failed (%d,%d,%d).",
			handle->Socket_Fd,retval,close_errno);
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Close:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to write a message to the opened socket link.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle An instance of Arcom_ESS_Socket_Handle_T containing the connection info.
 * @param message A buffer containing the bytes to write.
 * @param message_length The size of the buffer in bytes.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Socket_Handle_T
 */
int Arcom_ESS_Socket_Write(char *class,char *source,Arcom_ESS_Socket_Handle_T handle,void *message,
			   size_t message_length)
{
	int write_errno,retval;

	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 405;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Write:Message was NULL.");
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Write(%d bytes).",
			     message_length);
#endif /* LOGGING */
	retval = write(handle.Socket_Fd,message,message_length);
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Write returned %d.",retval);
#endif /* LOGGING */
	if(retval != message_length)
	{
		write_errno = errno;
		Arcom_ESS_Error_Number = 406;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Write: failed (%d,%d,%d).",
			handle.Socket_Fd,retval,write_errno);
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Write:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to read a message from the opened socket link. 
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle An instance of Arcom_ESS_Socket_Handle_T containing the connection info.
 * @param message A buffer of message_length bytes, to fill with any socket data returned.
 * @param message_length The length of the message buffer.
 * @param bytes_read The address of an integer. On return this will be filled with the number of bytes read from
 *        the socket interface. The address can be NULL, if this data is not needed.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Socket_Data
 */
int Arcom_ESS_Socket_Read(char *class,char *source,Arcom_ESS_Socket_Handle_T handle,void *message,int message_length,
			  int *bytes_read)
{
	int read_errno,retval;

	/* check input parameters */
	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 407;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Read:Message was NULL.");
		return FALSE;
	}
	if(message_length < 0)
	{
		Arcom_ESS_Error_Number = 408;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Read:Message length was too small:%d.",
			message_length);
		return FALSE;
	}
	/* initialise bytes_read */
	if(bytes_read != NULL)
		(*bytes_read) = 0;
#if LOGGING > 0
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Read:Max length %d.",
			     message_length);
#endif /* LOGGING */
	retval = read(handle.Socket_Fd,message,message_length);
#if LOGGING > 1
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Read:returned %d.",retval);
#endif /* LOGGING */
	if(retval < 0)
	{
		read_errno = errno;
		/* if the errno is EAGAIN, a non-blocking read has failed to return any data. */
		if(read_errno != EAGAIN)
		{
			Arcom_ESS_Error_Number = 409;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Read: failed (%d,%d,%d).",
				handle.Socket_Fd,retval,read_errno);
			return FALSE;
		}
		else
		{
			if(bytes_read != NULL)
				(*bytes_read) = 0;
		}
	}
	else
	{
		if(bytes_read != NULL)
			(*bytes_read) = retval;
	}
#if LOGGING > 0
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Read:returned %d bytes.",
			     retval);
#endif /* LOGGING */
	return TRUE;
}

/**
 * Try to flush any unread data from the file descriptor by repeated read's until read returns 0.
 * We wait a small delay before reading in case  some data is in the process of arriving at the Arcom ESS.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param handle A pointer to an instance of DArcom_ESS_Socket_Handle_T containing the connection info.
 * @see #Arcom_ESS_Socket_Handle_T
 * @see #SOCKET_READ_PAUSE_MS
 * @see arcom_ess_general.html#ARCOM_ESS_ONE_MILLISECOND_NS
 * @see arcom_ess_general.html#Arcom_ESS_Error
 * @see arcom_ess_general.html#Arcom_ESS_Log
 */
int Arcom_ESS_Socket_Flush(char *class,char *source,Arcom_ESS_Socket_Handle_T handle)
{
	struct timespec sleep_time;
	char message_buff[255];
	int retval,read_errno,sleep_errno;

#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Flush:Started.");
#endif /* LOGGING */
	/* keep reading until read returns 0 */
	retval = 1;
	while(retval > 0)
	{
		/* sleep a while */
		sleep_time.tv_sec = (SOCKET_READ_PAUSE_MS/1000);
		sleep_time.tv_nsec = (SOCKET_READ_PAUSE_MS%1000)*ARCOM_ESS_ONE_MILLISECOND_NS;
		retval = nanosleep(&sleep_time,NULL);
		if(retval != 0)
		{
			sleep_errno = errno;
			Arcom_ESS_Error_Number = 413;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Flush:nanosleep failed(%d):%s.",retval,
				strerror(sleep_errno));
			Arcom_ESS_Error();
			/* not a fatal error, don't return */
		}
		/* try and read something from the file descriptor */
		retval = read(handle.Socket_Fd,message_buff,255);
		if(retval < 0)
		{
			read_errno = errno;
			/* if the errno is EAGAIN, a non-blocking read has failed to return any data. */
			if(read_errno != EAGAIN)
			{
				Arcom_ESS_Error_Number = 414;
				sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Socket_Flush: failed (%d,%d,%d):%s.",
					handle.Socket_Fd,retval,read_errno,strerror(read_errno));
				return FALSE;
			}
			else
				retval = 0; /* terminate the flush - no more data to read */
		}
#if LOGGING > 0
		Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERY_VERBOSE,
				     "Arcom_ESS_Socket_Flush: read returned %d bytes.",retval);
#endif /* LOGGING */
	}/* end while */
#if LOGGING > 0
	Arcom_ESS_Log(class,source,LOG_VERBOSITY_VERY_VERBOSE,"Arcom_ESS_Socket_Flush:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/* ----------------------------------------------------------------
** internal functions 
** ---------------------------------------------------------------- */
/**
 * Internal routine to get a host address from it's name. This is traditionally handled by a call
 * to gethostbyname. Unfortunately that routine is not re-entrant because the pointer it returns
 * is to a block of reusable memory in glibc, so a second call to gethostbyname from another thread
 * in the process can lead to the pointers returned from the first call being freed leading to SIGSEGV.
 * This routine wraps gethostbyname_r, the re-entrant version of that routine.
 * @param class The class parameter for logging.
 * @param source The source parameter for logging.
 * @param name The hostname to translate. This should be allocated, zero-terminated and non-null.
 * @param host_addr_zero The address of a pointer  to an array of chars. This routine will allocate
 *       some memory and fill it with a null-terminated, network byte ordered copy of the first hostent host address
 *       list entry returned by gethostbyname_r. NULL can be returned on failure.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Socket_Get_Host_By_Name(char *class,char *source,const char *name,char **host_addr_zero)
{
	struct hostent hostbuf,*hp = NULL;
	size_t hstbuflen;
	char *tmphstbuf = NULL;
	int retval;
	int herr;

	Arcom_ESS_Error_Number = 0;
	if(name == NULL)
	{
		Arcom_ESS_Error_Number = 415;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:name was NULL.");
		return FALSE;
	}
	if(host_addr_zero == NULL)
	{
		Arcom_ESS_Error_Number = 416;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:host_addr_zero was NULL.");
		return FALSE;
	}
#if LOGGING > 5
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Socket_Get_Host_By_Name(%s) Started.",name);
#endif /* LOGGING */
	hstbuflen = 1024;
	/* Allocate buffer, remember to free it to avoid memory leakage.  */
	tmphstbuf = malloc(hstbuflen);
	if(tmphstbuf == NULL)
	{
		Arcom_ESS_Error_Number = 417;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:memory allocation of tmphstbuf failed(%d).",
			hstbuflen);
		return FALSE;

	}
	while((retval = gethostbyname_r(name,&hostbuf,tmphstbuf,hstbuflen,&hp,&herr)) == ERANGE)
	{
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
		tmphstbuf = realloc(tmphstbuf, hstbuflen);
		/* check realloc succeeds */
		if(tmphstbuf == NULL)
		{
			Arcom_ESS_Error_Number = 418;
			sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:"
				"memory reallocation of tmphstbuf failed(%d).",hstbuflen);
			return FALSE;
		}
#if LOGGING > 5
		Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,
				     "Socket_Get_Host_By_Name:gethostbyname_r returned ERANGE:"
				     "Increasing buffer size to %d.",hstbuflen);
#endif /* LOGGING */
	}/* while */
#if LOGGING > 5
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Socket_Get_Host_By_Name:"
			     "gethostbyname_r loop exited with retval %d and hp %p.",retval,hp);
#endif /* LOGGING */
	if(retval != 0)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		Arcom_ESS_Error_Number = 419;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:"
			"gethostbyname_r failed to find host %s (%d).",name,herr);
		return FALSE;
	}
	if(hp == NULL)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		Arcom_ESS_Error_Number = 420;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:"
			"gethostbyname_r returned NULL return pointer for hostname %s (%d).",name,herr);
		return FALSE;
	}
	/* copy result */
	(*host_addr_zero) = strdup(hp->h_addr_list[0]);
	if((*host_addr_zero) == NULL)
	{
		if(tmphstbuf != NULL)
			free(tmphstbuf);
		Arcom_ESS_Error_Number = 421;
		sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:"
			"Failed to copy gethostbyname_r result string (%s).",hp->h_addr_list[0]);
		return FALSE;
	}
	/* free buffer*/
	if(tmphstbuf != NULL)
		free(tmphstbuf);
#if LOGGING > 5
	Arcom_ESS_Log_Format(class,source,LOG_VERBOSITY_VERBOSE,"Socket_Get_Host_By_Name(%s) Finished.",name);
#endif /* LOGGING */
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.5  2010/11/24 11:47:17  cjm
** Replaced gethostbyname with Socket_Get_Host_By_Name thats calls gethostbyname_r.
** This is stop SIGSEGV in FrodoSpec, when gethostbyname is called simultaneously by the
** Arcom ESS library and EIP PLC library when getting statii. gethostbyname is not re-entrant.
**
** Revision 1.4  2009/02/04 11:23:50  cjm
** Changed log bits to log_udp verbosities.
**
** Revision 1.3  2008/10/29 14:43:54  cjm
** Added socket flushing.
**
** Revision 1.2  2008/06/02 16:55:49  cjm
** Added conditionally compiled TCP_NODELAY software.
**
** Revision 1.1  2008/03/18 17:04:22  cjm
** Initial revision
**
*/
