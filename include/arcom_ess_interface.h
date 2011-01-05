/* arcom_ess_interface.h
** Arcom ESS serial interface library.
** $Header: /home/cjm/cvs/arcom_ess/include/arcom_ess_interface.h,v 1.3 2011-01-05 14:30:09 cjm Exp $
*/
#ifndef ARCOM_ESS_INTERFACE_H
#define ARCOM_ESS_INTERFACE_H

/**
 * Maximum length of the device name.
 */
#define ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH   (256)

/**
 * Enum for the whether we are talking to the serial device over a serial link, or over a socket connection
 * via a Arcom ESS.
 * <ul>
 * <li>ARCOM_ESS_INTERFACE_DEVICE_NONE Interface device number, showing that commands will currently be sent nowhere.
 * <li>ARCOM_ESS_INTERFACE_DEVICE_SERIAL Interface device number, showing that commands will be sent using
 * the serial interface.
 * <li>ARCOM_ESS_INTERFACE_DEVICE_SOCKET Interface device number, showing that commands will currently be sent to the 
 * serial device via an Arcom ESS (Ethernet Serial Server).
 * </ul>
 * This enum should be the same as the constants defined in ngat.serial.arcomess.ArcomESS.java.
 */
enum ARCOM_ESS_INTERFACE_DEVICE_ID
{
	ARCOM_ESS_INTERFACE_DEVICE_NONE,ARCOM_ESS_INTERFACE_DEVICE_SERIAL,ARCOM_ESS_INTERFACE_DEVICE_SOCKET
};

/**
 * Macro to check whether the interface device number is in range.
 */
#define ARCOM_ESS_INTERFACE_IS_INTERFACE_DEVICE(interface_device)	(((interface_device) == ARCOM_ESS_INTERFACE_DEVICE_NONE)|| \
	((interface_device) == ARCOM_ESS_INTERFACE_DEVICE_SERIAL)||((interface_device) == ARCOM_ESS_INTERFACE_DEVICE_SOCKET))

/**
 * Typedef for the interface handle pointer, which is an instance of Arcom_ESS_Interface_Handle_Struct.
 * @see #Arcom_ESS_Interface_Handle_Struct
 */
typedef struct Arcom_ESS_Interface_Handle_Struct Arcom_ESS_Interface_Handle_T;

extern int Arcom_ESS_Interface_Handle_Create(Arcom_ESS_Interface_Handle_T **handle);
extern int Arcom_ESS_Interface_Open(char *class,char *source,enum ARCOM_ESS_INTERFACE_DEVICE_ID device_id,
				    char *device_name,int port_number,Arcom_ESS_Interface_Handle_T *handle);
extern int Arcom_ESS_Interface_Close(char *class,char *source,Arcom_ESS_Interface_Handle_T *handle);
extern int Arcom_ESS_Interface_Handle_Destroy(Arcom_ESS_Interface_Handle_T **handle);
extern int Arcom_ESS_Interface_Write(char *class,char *source,Arcom_ESS_Interface_Handle_T *handle,void* message,
				     size_t message_length);
extern int Arcom_ESS_Interface_Read(char *class,char *source,Arcom_ESS_Interface_Handle_T *handle,void* message,
				    int message_length,int* bytes_read);
extern int Arcom_ESS_Interface_Flush(char *class,char *source,Arcom_ESS_Interface_Handle_T *handle);
extern int Arcom_ESS_Interface_Mutex_Lock(Arcom_ESS_Interface_Handle_T *handle);
extern int Arcom_ESS_Interface_Mutex_Unlock(Arcom_ESS_Interface_Handle_T *handle);

#endif
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2008/10/29 14:45:35  cjm
** Added flush operation to interface.
**
** Revision 1.1  2008/03/18 17:04:30  cjm
** Initial revision
**
*/
