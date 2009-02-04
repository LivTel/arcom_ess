/* arcom_ess_test.c
** $Header: /home/cjm/cvs/arcom_ess/test/arcom_ess_test.c,v 1.3 2009-02-04 11:24:22 cjm Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "log_udp.h"
#include "arcom_ess_general.h"
#include "arcom_ess_interface.h"

/**
 * This program tests writing to and reading from a serial connection, which may go via 
 * an Arcom ESS.
 * @author $Author: cjm $
 * @version $Revision: 1.3 $
 */
/**
 * Default bit-wise log level.
 */
#define DEFAULT_LOG_LEVEL       (LOG_VERBOSITY_VERY_VERBOSE)

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id: arcom_ess_test.c,v 1.3 2009-02-04 11:24:22 cjm Exp $";
/**
 * Variable holding which type of device we are using to communicate with the PLC.
 * @see ../cdocs/arcom_ess_interface.html#ARCOM_ESS_INTERFACE_DEVICE_ID
 */
static enum ARCOM_ESS_INTERFACE_DEVICE_ID Device_Id = ARCOM_ESS_INTERFACE_DEVICE_NONE;
/**
 * The name of the serial device to open, or the IP Address/hostnmae of the socket device.
 * @see #Device_Id
 */
static char Device_Name[256];
/**
 * The port number of the Arcomm Ethernet Serial Server to open.
 */
static int Port_Number = 0;
/**
 * A string to send over the opened serial connection.
 */
static char Command_String[256];
/**
 * A boolean, if true, keep reading from the serial conenction until Read_Until_Char is found.
 * @see #Read_Until_Char
 */
static int Read_Until = FALSE;
/**
 * Used if Read_Until is TRUE - keeping reading from the serial link until this character is read.
 */
static char Read_Until_Char = '\0';

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 */
int main(int argc, char *argv[])
{
	Arcom_ESS_Interface_Handle_T *handle = NULL;
	char buff[256];
	int bytes_read,done;

	fprintf(stdout,"Test connection to Arcom ESS.\n");
	/* initialise logging */
	Arcom_ESS_Set_Log_Handler_Function(Arcom_ESS_Log_Handler_Stdout);
	Arcom_ESS_Set_Log_Filter_Function(Arcom_ESS_Log_Filter_Level_Bitwise);
	Arcom_ESS_Set_Log_Filter_Level(DEFAULT_LOG_LEVEL);
	fprintf(stdout,"Parsing Arguments.\n");
	/* parse arguments */
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* open the interface */
	if(!Arcom_ESS_Interface_Handle_Create(&handle))
	{
		Arcom_ESS_Error();
		return 4;
	}
	if(!Arcom_ESS_Interface_Open(Device_Id,Device_Name,Port_Number,handle))
	{
		Arcom_ESS_Error();
		return 3;
	}
	/* write command */
	if(strlen(Command_String) > 0)
	{
		if(Command_String[strlen(Command_String)-1] != '\n')
			strcat(Command_String,"\r\n");
		fprintf(stdout,"arcom_ess_test:Writing(%d):%s.\n",strlen(Command_String),Command_String);
		if(!Arcom_ESS_Interface_Write(handle,Command_String,strlen(Command_String)))
			Arcom_ESS_Error();
	}
	/* read any reply */
	if(!Arcom_ESS_Interface_Read(handle,buff,255,&bytes_read))
	{
		Arcom_ESS_Error();
		return 4;
	}
	buff[bytes_read] = '\0';
	fprintf(stdout,"arcom_ess_test:Read(%d):%s.\n",bytes_read,buff);
	if(Read_Until)
	{
		done = FALSE;
		/* check if read_until char has already been read */
		if(strchr(buff,Read_Until_Char) != NULL)
		{
			done = TRUE;
		}
		while(done == FALSE)
		{
			if(!Arcom_ESS_Interface_Read(handle,buff,255,&bytes_read))
			{
				Arcom_ESS_Error();
				return 4;
			}
			buff[bytes_read] = '\0';
			fprintf(stdout,"arcom_ess_test:Read(%d):%s.\n",bytes_read,buff);
			if(strchr(buff,Read_Until_Char) != NULL)
			{
				done = TRUE;
			}
		}
	}
	/* close interface */
	if(!Arcom_ESS_Interface_Close(handle))
	{
		Arcom_ESS_Error();
		return 3;
	}
	if(!Arcom_ESS_Interface_Handle_Destroy(&handle))
	{
		Arcom_ESS_Error();
		return 5;
	}
	fprintf(stdout,"Arcom ESS Test:Finished Test ...\n");
	return 0;
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Device_Name
 * @see #Port_Number
 * @see #Device_Id
 * @see #Command_String
 * @see #Read_Until
 * @see #Read_Until_Char
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,ivalue;

	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-baud_rate")==0)
		{
			if((i+1)<argc)
			{
				if(strcmp(argv[i+1],"B9600")==0)
					Arcom_ESS_Serial_Baud_Rate_Set(B9600);
				else if(strcmp(argv[i+1],"B19200")==0)
					Arcom_ESS_Serial_Baud_Rate_Set(B19200);
				else
				{
					fprintf(stderr,"Arcom ESS Test :Parse_Arguments:"
						"Illegal baud rate : %s.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test :Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else if(strcmp(argv[i],"-read_until")==0)
		{
			if((i+1)<argc)
			{
				if(strlen(argv[i+1]) != 1)
				{
					fprintf(stderr,"Arcom ESS Test :Parse_Arguments:"
						"Read Until Char was too long.\n");
					return FALSE;
				}
				Read_Until = TRUE;
				Read_Until_Char = argv[i+1][0];
				i++;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test :Parse_Arguments:"
					"Read Until Char requires a character.\n");
				return FALSE;
			}
		}
		else if(strcmp(argv[i],"-serial_device")==0)
		{
			if((i+1)<argc)
			{
				strcpy(Device_Name,argv[i+1]);
				Device_Id = ARCOM_ESS_INTERFACE_DEVICE_SERIAL;
				i++;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test :Parse_Arguments:"
					"Device filename requires a filename.\n");
				return FALSE;
			}
		}
		else if(strcmp(argv[i],"-socket_device")==0)
		{
			if((i+2)<argc)
			{
				strcpy(Device_Name,argv[i+1]);
				retval = sscanf(argv[i+2],"%d",&Port_Number);
				if(retval != 1)
				{
					fprintf(stderr,"Arcom ESS Test:Parse_Arguments:"
						"Illegal Socket Port %s.\n",argv[i+2]);
					return FALSE;
				}
				Device_Id = ARCOM_ESS_INTERFACE_DEVICE_SOCKET;
				i+= 2;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test:Parse_Arguments:"
					"Socket Device requires an address and port number.\n");
				return FALSE;
			}
		}
		else if(strcmp(argv[i],"-command")==0)
		{
			if((i+1)<argc)
			{
				strncpy(Command_String,argv[i+1],255);
				i++;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test:Parse_Arguments:"
					"Address requires an address.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-linefeed")==0)||(strcmp(argv[i],"-lf")==0))
		{
			strcat(Command_String,"\r\n");
		}
		else if(strcmp(argv[i],"-log_level")==0)
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&ivalue);
				if(retval != 1)
				{
					fprintf(stderr,"Arcom ESS Test:Parse_Arguments:"
						"Illegal log level %s.\n",argv[i+1]);
					return FALSE;
				}
				Arcom_ESS_Set_Log_Filter_Level(ivalue);
				i++;
			}
			else
			{
				fprintf(stderr,"Arcom ESS Test:Parse_Arguments:"
					"Log Level requires a number.\n");
				return FALSE;
			}
		}
		else if(strcmp(argv[i],"-help")==0)
		{
			Help();
			exit(0);
		}
		else
		{
			fprintf(stderr,"Arcom ESS Test:Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}			
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Arcom ESS Test:Help.\n");
	fprintf(stdout,"Arcom ESS Test .\n");
	fprintf(stdout,"arcom_ess_test [-serial_device <filename>][-socket_device <address> <port>]\n");
	fprintf(stdout,"\t[-command <string>][-log_level <number>][-help]\n");
	fprintf(stdout,"\t[-lf|-linefeed][-baud_rate <B9600|B19200>][-read_until <character>]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-serial_device specifies the serial device name.\n");
	fprintf(stdout,"\t\tTry /dev/ttyS0 for Linux, try /dev/ttyb for Solaris.\n");
	fprintf(stdout,"\t-socket_device specifies the socket device name.\n");
	fprintf(stdout,"\t-command specifies a string to send to the connected device.\n");
	fprintf(stdout,"\t-linefeed , if specified after -command, appends a line feed to the command.\n");
	fprintf(stdout,"\t-baud_rate changes the serial devices configured baud rate (serial connection only).\n");
	fprintf(stdout,"\t-read_until keeps reading after the command has been sent, until the specified character (prompt) has been received.\n");
	fprintf(stdout,"\t-log_level specifies the logging. See arcom_ess_general.h for details.\n");
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2008/07/22 13:42:17  cjm
** Added stdlib.h/string.h.
**
** Revision 1.1  2008/03/18 17:04:36  cjm
** Initial revision
**
*/
