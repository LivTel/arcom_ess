/* arcom_ess_serial.c
** Arcom ESS interface library
** $Header: /home/cjm/cvs/arcom_ess/c/arcom_ess_serial.c,v 1.1 2008-03-18 17:04:22 cjm Exp $
*/
/**
 * Basic operations, open close etc.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
 */
/**
 * Define BSD Source to get BSD prototypes, including cfmakeraw.
 */
#define _BSD_SOURCE
#include <errno.h>   /* Error number definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "arcom_ess_general.h"
#include "arcom_ess_serial.h"

/* hash defines */

/* data types */
/**
 * Data structure containing default baud rates and flags, an instance of which is used
 * to initialise new serial connections.
 * <dl>
 * <dt>Baud_Rate</dt> <dd>The baud rate of the serial connection : usually B19200 or B9600 from termios.h.</dd>
 * <dt>Input_Flags</dt> <dd>Flags to or into c_iflag.</dd>
 * <dt>Output_Flags</dt> <dd>Flags to or into c_oflag.</dd>
 * <dt>Control_Flags</dt> <dd>Flags to or into c_cflag.</dd>
 * <dt>Local_Flags</dt> <dd>Flags to or into c_lflag.</dd>
 * </dl>
 */
struct Serial_Attribute_Struct
{
	int Baud_Rate;
	int Input_Flags;
	int Output_Flags;
	int Control_Flags;
	int Local_Flags;
};


/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: arcom_ess_serial.c,v 1.1 2008-03-18 17:04:22 cjm Exp $";
/**
 * Instance of Serial_Attribute_Struct containing attributes to initialise opened serial connections with:
 * <dl>
 * <dt>Baud_Rate</dt> <dd>B19200</dd>
 * <dt>Input_Flags</dt> <dd>IGNPAR</dd>
 * <dt>Output_Flags</dt> <dd>0</dd>
 * <dt>Control_Flags</dt> <dd>CS8 | CLOCAL | CREAD</dd>
 * <dt>Local_Flags</dt> <dd>0</dd>
 * </dl>
 * @see #Serial_Attribute_Struct
 */
static struct Serial_Attribute_Struct Serial_Attribute_Data = 
{
	B19200,IGNPAR,0,CS8 | CLOCAL | CREAD, 0

};

/* external functions */
/**
 * Set the baud rate to be set for subsequently opened serial handles.
 * @param baud_rate The baud rate to use, usually B9600 or B19200 from termios.h - man tcsetattr to see all of them.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Serial_Attribute_Data
 */
int Arcom_ESS_Serial_Baud_Rate_Set(int baud_rate)
{
	/* check baud rate.
	** This is a subset of all allowable values, man tcsetattr to see all of them */
	if((baud_rate != B1200)&&(baud_rate != B1800)&&(baud_rate != B2400)&&(baud_rate != B4800)&&
	   (baud_rate != B9600)&&(baud_rate != B19200)&&(baud_rate != B38400)&&(baud_rate != B57600)&&
	   (baud_rate != B115200))
	{
		Arcom_ESS_Error_Number = 15;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Baud_Rate_Set: Illegal baud rate %d.",baud_rate);
		return FALSE;
	}
	Serial_Attribute_Data.Baud_Rate = baud_rate;
	return TRUE;
}
/**
 * Open the serial device, and configure accordingly,using default in Serial_Attribute_Data.
 * @param handle The address of a Arcom_ESS_Serial_Handle_T structure to fill in.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Serial_Handle_T
 * @see #Serial_Attribute_Data
 */
int Arcom_ESS_Serial_Open(Arcom_ESS_Serial_Handle_T *handle)
{
	int open_errno,retval;

	if(handle == NULL)
	{
		Arcom_ESS_Error_Number = 1;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: Device handle was NULL.");
		return FALSE;
	}
	/* Open serial device, read/write, not a controlling terminal, don't wait for DCD signal line. */
#if LOGGING > 0
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Open(%s).",handle->Device_Name);
#endif /* LOGGING */
	handle->Serial_Fd = open(handle->Device_Name, O_RDWR | O_NOCTTY | O_NDELAY);
	if(handle->Serial_Fd < 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 2;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: Device %s failed to open (%d,%d).",
			handle->Device_Name,handle->Serial_Fd,open_errno);
		return FALSE;
	}
#if LOGGING > 1
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Open with FD %d.",handle->Serial_Fd);
#endif /* LOGGING */
	/* make non-blocking */
	/* diddly */
	retval = fcntl(handle->Serial_Fd, F_SETFL, FNDELAY);
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 10;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: fcntl failed (%d).",open_errno);
		return FALSE;
	}	
	retval = fcntl(handle->Serial_Fd, F_SETFL, FASYNC);
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 14;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: fcntl failed (%d).",open_errno);
		return FALSE;
	}	
	/* get current serial options */
	retval = tcgetattr(handle->Serial_Fd,&(handle->Serial_Options_Saved));
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 3;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: tcgetattr failed (%d).",open_errno);
		return FALSE;
	}
	/* initialise new serial options */
	bzero(&(handle->Serial_Options), sizeof(handle->Serial_Options));
	/* set control flags and baud rate */
	handle->Serial_Options.c_cflag |= Serial_Attribute_Data.Baud_Rate | Serial_Attribute_Data.Control_Flags;
#ifdef CNEW_RTSCTS
	handle->Serial_Options.c_cflag &= ~CNEW_RTSCTS;/* disable flow control */
#endif
#ifdef CRTSCTS
	handle->Serial_Options.c_cflag &= ~CRTSCTS;/* disable flow control */
#endif
	/* select raw input, not line input. */
	handle->Serial_Options.c_lflag = Serial_Attribute_Data.Local_Flags;
	/* ignore parity errors */
	handle->Serial_Options.c_iflag = Serial_Attribute_Data.Input_Flags;
	/* set raw output */
	handle->Serial_Options.c_oflag = Serial_Attribute_Data.Output_Flags;
	/* blocking read until 0 char arrives */
	handle->Serial_Options.c_cc[VMIN]=0;
	handle->Serial_Options.c_cc[VTIME]=0;
#if LOGGING > 2
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,
	      "Arcom_ESS_Serial_Open:New Attr:Input:%#x,Output:%#x,Local:%#x,Control:%#x,Min:%c,Time:%c.",
		       handle->Serial_Options.c_iflag,handle->Serial_Options.c_oflag,
		       handle->Serial_Options.c_lflag,handle->Serial_Options.c_cflag,
		       handle->Serial_Options.c_cc[VMIN],handle->Serial_Options.c_cc[VTIME]);
#endif /* LOGGING */
 	/* set new options */
#if LOGGING > 1
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Open:Setting serial options.");
#endif /* LOGGING */
	tcflush(handle->Serial_Fd, TCIFLUSH);
	retval = tcsetattr(handle->Serial_Fd,TCSANOW,&(handle->Serial_Options));
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 11;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: tcsetattr failed (%d).",open_errno);
		return FALSE;
	}
	/* re-get current serial options to see what was set */
#if LOGGING > 1
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Open:Re-Getting new serial options.");
#endif /* LOGGING */
	retval = tcgetattr(handle->Serial_Fd,&(handle->Serial_Options));
	if(retval != 0)
	{
		open_errno = errno;
		Arcom_ESS_Error_Number = 12;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Open: re-get tcgetattr failed (%d).",
			open_errno);
		return FALSE;
	}
#if LOGGING > 2
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,
	      "Arcom_ESS_Serial_Open:New Get Attr:Input:%#x,Output:%#x,Local:%#x,Control:%#x,Min:%c,Time:%c.",
		       handle->Serial_Options.c_iflag,handle->Serial_Options.c_oflag,
		       handle->Serial_Options.c_lflag,handle->Serial_Options.c_cflag,
		       handle->Serial_Options.c_cc[VMIN],handle->Serial_Options.c_cc[VTIME]);
#endif /* LOGGING */
	/* clean I & O device */
	tcflush(handle->Serial_Fd,TCIOFLUSH);
#if LOGGING > 0
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Open:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to close a previously open serial device. The serial options are first reset.
 * @param handle The address of a Arcom_ESS_Serial_Handle_T structure to close.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Serial_Handle_T
 */
int Arcom_ESS_Serial_Close(Arcom_ESS_Serial_Handle_T *handle)
{
	int retval,close_errno;

#if LOGGING > 0
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Close:Started.");
#endif /* LOGGING */
#if LOGGING > 1
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Close:Flushing serial line.");
#endif /* LOGGING */
	tcflush(handle->Serial_Fd, TCIFLUSH);
#if LOGGING > 1
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Close:Resetting serial options.");
#endif /* LOGGING */
	retval = tcsetattr(handle->Serial_Fd,TCSANOW,&(handle->Serial_Options_Saved));
	if(retval != 0)
	{
		close_errno = errno;
		Arcom_ESS_Error_Number = 13;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Close: tcsetattr failed (%d).",close_errno);
		return FALSE;
	}
#if LOGGING > 1
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Close:Closing file descriptor.");
#endif /* LOGGING */
	retval = close(handle->Serial_Fd);
	if(retval < 0)
	{
		close_errno = errno;
		Arcom_ESS_Error_Number = 4;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Close: failed (%d,%d,%d).",
			handle->Serial_Fd,retval,close_errno);
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Close:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to write a message to the opened serial link.
 * @param handle An instance of Arcom_ESS_Serial_Handle_T containing connection information to write to.
 * @param message A pointer to an allocated buffer containing the bytes to write.
 * @param message_length The length of the message to write.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see #Arcom_ESS_Serial_Handle_T
 */
int Arcom_ESS_Serial_Write(Arcom_ESS_Serial_Handle_T handle,void *message,size_t message_length)
{
	int write_errno,retval;

	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 5;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Write:Message was NULL.");
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Write(%d bytes).",message_length);
#endif /* LOGGING */
	retval = write(handle.Serial_Fd,message,message_length);
#if LOGGING > 1
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Write returned %d.",retval);
#endif /* LOGGING */
	if(retval != message_length)
	{
		write_errno = errno;
		Arcom_ESS_Error_Number = 6;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Write: failed (%d,%d,%d).",
			handle.Serial_Fd,retval,write_errno);
		return FALSE;
	}
#if LOGGING > 0
	Arcom_ESS_Log(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Write:Finished.");
#endif /* LOGGING */
	return TRUE;
}

/**
 * Routine to read a message from the opened serial link. 
 * @param handle An instance of Arcom_ESS_Serial_Handle_T containing connection information to read from.
 * @param message A buffer of message_length bytes, to fill with any serial data returned.
 * @param message_length The length of the message buffer.
 * @param bytes_read The address of an integer. On return this will be filled with the number of bytes read from
 *        the serial interface. The address can be NULL, if this data is not needed.
 * @return TRUE if succeeded, FALSE otherwise.
 * @see filter_wheel_general.html#Arcom_ESS_Replace_String
 */
int Arcom_ESS_Serial_Read(Arcom_ESS_Serial_Handle_T handle,void *message,int message_length,int *bytes_read)
{
	int read_errno,retval;

	/* check input parameters */
	if(message == NULL)
	{
		Arcom_ESS_Error_Number = 7;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Read:Message was NULL.");
		return FALSE;
	}
	if(message_length < 0)
	{
		Arcom_ESS_Error_Number = 8;
		sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Read:Message length was too small:%d.",
			message_length);
		return FALSE;
	}
	/* initialise bytes_read */
	if(bytes_read != NULL)
		(*bytes_read) = 0;
#if LOGGING > 0
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Read:Max length %d.",message_length);
#endif /* LOGGING */
	retval = read(handle.Serial_Fd,message,message_length);
#if LOGGING > 1
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Read:returned %d.",retval);
#endif /* LOGGING */
	if(retval < 0)
	{
		read_errno = errno;
		/* if the errno is EAGAIN, a non-blocking read has failed to return any data. */
		if(read_errno != EAGAIN)
		{
			Arcom_ESS_Error_Number = 9;
			sprintf(Arcom_ESS_Error_String,"Arcom_ESS_Serial_Read: failed (%d,%d,%d).",
				handle.Serial_Fd,retval,read_errno);
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
	Arcom_ESS_Log_Format(ARCOM_ESS_LOG_BIT_SERIAL,"Arcom_ESS_Serial_Read:returned %d of %d.",retval,message_length);
#endif /* LOGGING */
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
