/* test_get_host_by_name.c
** $Header: /home/cjm/cvs/arcom_ess/test/test_get_host_by_name.c,v 1.1 2023-03-21 13:17:51 cjm Exp $
** cc test_get_host_by_name.c -o test_get_host_by_name
*/
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * This is a test program to prototype the use of gethostbyname_r.
 * @author $Author: cjm $
 * @version $Revision: 1.1 $
 */
/* hash defines */
/**
 * TRUE is the value usually returned from routines to indicate success.
 */
#ifndef TRUE
#define TRUE 1
#endif
/**
 * FALSE is the value usually returned from routines to indicate failure.
 */
#ifndef FALSE
#define FALSE 0
#endif
/**
 * Length of error string.
 */
#define ERROR_STRING_LENGTH (1024)
/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id: test_get_host_by_name.c,v 1.1 2023-03-21 13:17:51 cjm Exp $";
/**
 * Copy of libarcom_ess error number to simplify testing.
 */
static int Arcom_ESS_Error_Number = 0;
/**
 * Copy of libarcom_ess error string to simplify testing.
 * @see #ERROR_STRING_LENGTH
 */
static char Arcom_ESS_Error_String[ERROR_STRING_LENGTH];

/* internal functions */
static int Socket_Get_Host_By_Name(const char *name,char **host_addr_zero);

/* external functions */
/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 */
int main(int argc, char *argv[])
{
	char *hostname = NULL;
	char *host_addr_zero = NULL;
	char *ascii_host_addr = NULL;
	int i;
	if(argc != 2)
	{
		fprintf(stderr,"test_get_host_by_name:Requires hostname.\n");
		fprintf(stderr,"test_get_host_by_name <hostname>.\n");
		return 1;
	}
	hostname = argv[1];
	if(!Socket_Get_Host_By_Name(hostname,&host_addr_zero))
	{
		fprintf(stderr,"test_get_host_by_name:Socket_Get_Host_By_Name failed (%d):%s\n",
			Arcom_ESS_Error_Number,Arcom_ESS_Error_String);
		return 1;
			
	}
	fprintf(stdout,"test_get_host_by_name:Socket_Get_Host_By_Name returned '%s'.\n",host_addr_zero);
	ascii_host_addr = inet_ntoa(*(struct in_addr *)(host_addr_zero));
	fprintf(stdout,"test_get_host_by_name:ASCII hostname '%s'.\n",ascii_host_addr);
	if(host_addr_zero != NULL)
		free(host_addr_zero);
	return 0;
}

/**
 * Internal routine to get a host address from it's name. This is traditionally handled by a call
 * to gethostbyname. Unfortunately that routine is not re-entrant because the pointer it returns
 * is to a block of reusable memory in glibc, so a second call to gethostbyname from another thread
 * in the process can lead to the pointers returned from the first call being freed leading to SIGSEGV.
 * This routine wraps gethostbyname_r, the re-entrant version of that routine.
 * @param name The hostname to translate. This should be allocated, zero-terminated and non-null.
 * @param host_addr_zero The address of a pointer  to an array of chars. This routine will allocate
 *       some memory and fill it with a null-terminated, network byte ordered copy of the first hostent host address
 *       list entry returned by gethostbyname_r. NULL can be returned on failure.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Socket_Get_Host_By_Name(const char *name,char **host_addr_zero)
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
	while ((retval = gethostbyname_r(name,&hostbuf,tmphstbuf,hstbuflen,&hp,&herr)) == ERANGE)
	{
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
		tmphstbuf = realloc(tmphstbuf, hstbuflen);
		/* diddly check realloc succeeds */
		if(tmphstbuf == NULL)
		{
			Arcom_ESS_Error_Number = 418;
			sprintf(Arcom_ESS_Error_String,"Socket_Get_Host_By_Name:"
				"memory reallocation of tmphstbuf failed(%d).",hstbuflen);
			return FALSE;
		}
	}
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
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
