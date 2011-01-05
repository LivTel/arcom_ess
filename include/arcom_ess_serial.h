/* arcom_ess_serial.h
** $Header: /home/cjm/cvs/arcom_ess/include/arcom_ess_serial.h,v 1.3 2011-01-05 14:30:09 cjm Exp $
*/

#ifndef ARCOM_ESS_SERIAL_H
#define ARCOM_ESS_SERIAL_H

#include <termios.h>
#include <unistd.h>
/* to get ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH */
#include "arcom_ess_interface.h"

/* structures */
/**
 * Structure holding local data pertinent to the serial module. This consists of:
 * <ul>
 * <li><b>Device_Name</b> The device name string of the serial port (e.g. /dev/ttyS0). 
 *     Maximum length ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH.
 * <li><b>Serial_Options_Saved</b> The saved set of serial options.
 * <li><b>Serial_Options</b> The set of serial options configured.
 * <li><b>Serial_Fd</b> The opened serial port's file descriptor.
 * </ul>
 * @see arcom_ess_interface.html#ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH
 */
typedef struct Arcom_ESS_Serial_Handle_Struct
{
	char Device_Name[ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH];
	struct termios Serial_Options_Saved;
	struct termios Serial_Options;
	int Serial_Fd;
} Arcom_ESS_Serial_Handle_T;

/* config functions */
extern int Arcom_ESS_Serial_Baud_Rate_Set(int baud_rate);
extern int Arcom_ESS_Serial_Input_Flags_Set(int flags);
extern int Arcom_ESS_Serial_Output_Flags_Set(int flags);
extern int Arcom_ESS_Serial_Control_Flags_Set(int flags);
extern int Arcom_ESS_Serial_Local_Flags_Set(int flags);

extern int Arcom_ESS_Serial_Open(char *class,char *source,Arcom_ESS_Serial_Handle_T *handle);
extern int Arcom_ESS_Serial_Close(char *class,char *source,Arcom_ESS_Serial_Handle_T *handle);
extern int Arcom_ESS_Serial_Write(char *class,char *source,Arcom_ESS_Serial_Handle_T handle,void *message,
				  size_t message_length);
extern int Arcom_ESS_Serial_Read(char *class,char *source,Arcom_ESS_Serial_Handle_T handle,void *message,
				 int message_length,int *bytes_read);

#endif
