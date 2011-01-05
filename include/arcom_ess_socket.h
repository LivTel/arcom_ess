/* arcom_ess_socket.h
** $Header: /home/cjm/cvs/arcom_ess/include/arcom_ess_socket.h,v 1.3 2011-01-05 14:30:09 cjm Exp $
*/

#ifndef ARCOM_ESS_SOCKET_H
#define ARCOM_ESS_SOCKET_H

/* to get ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH */
#include "arcom_ess_interface.h"

/* structures */
/**
 * Structure holding local data pertinent to the socket module. This consists of:
 * <ul>
 * <li><b>Address</b> The address string of the machine/IP number to connect to. 
 *        A fixed length array of length ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH.
 * <li><b>Port_Number</b> The port number of the server socket to connect to.
 * <li><b>Socket_Fd</b> The opened socket port's file descriptor.
 * </ul>
 * @see arcom_ess_interface.html#ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH
 */
typedef struct Arcom_ESS_Socket_Handle_Struct
{
	char Address[ARCOM_ESS_INTERFACE_DEVICE_NAME_STRING_LENGTH];
	int Port_Number;
	int Socket_Fd;
} Arcom_ESS_Socket_Handle_T;

extern int Arcom_ESS_Socket_Open(char *class,char *source,Arcom_ESS_Socket_Handle_T *handle);
extern int Arcom_ESS_Socket_Close(char *class,char *source,Arcom_ESS_Socket_Handle_T *handle);
extern int Arcom_ESS_Socket_Write(char *class,char *source,Arcom_ESS_Socket_Handle_T handle,void *message,
				  size_t message_length);
extern int Arcom_ESS_Socket_Read(char *class,char *source,Arcom_ESS_Socket_Handle_T handle,void *message,
				 int message_length,int *bytes_read);
extern int Arcom_ESS_Socket_Flush(char *class,char *source,Arcom_ESS_Socket_Handle_T handle);

#endif
