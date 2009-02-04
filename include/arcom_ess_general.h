/* arcom_ess_general.h
** $Header: /home/cjm/cvs/arcom_ess/include/arcom_ess_general.h,v 1.3 2009-02-04 11:24:11 cjm Exp $
*/

#ifndef ARCOM_ESS_GENERAL_H
#define ARCOM_ESS_GENERAL_H

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
 * How long the error string is.
 */
#define ARCOM_ESS_ERROR_LENGTH (1024)
/**
 * One millosecond in nanoseconds (1000000).
 */
#define ARCOM_ESS_ONE_MILLISECOND_NS   (1000000)

/**
 * Macro to check whether the parameter is either TRUE or FALSE.
 */
#define ARCOM_ESS_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))
/**
 * Macro to check whether the parameter is a sign character, i.e. either '+' or '-'.
 */
#define ARCOM_ESS_IS_SIGN(value)	(((value) == '+')||((value) == '-'))

/* external functions */
extern void Arcom_ESS_Error(void);
extern void Arcom_ESS_Error_To_String(char *error_string);
extern int Arcom_ESS_Get_Error_Number(void);
extern void Arcom_ESS_Get_Current_Time_String(char *time_string,int string_length);
extern void Arcom_ESS_Log_Format(int level,char *format,...);
extern void Arcom_ESS_Log(int level,char *string);
extern void Arcom_ESS_Set_Log_Handler_Function(void (*log_fn)(int level,char *string));
extern void Arcom_ESS_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string));
extern void Arcom_ESS_Log_Handler_Stdout(int level,char *string);
extern void Arcom_ESS_Set_Log_Filter_Level(int level);
extern int Arcom_ESS_Log_Filter_Level_Absolute(int level,char *string);
extern int Arcom_ESS_Log_Filter_Level_Bitwise(int level,char *string);
extern char *Arcom_ESS_Replace_String(char *string,char *find_string,char *replace_string);

/* external variables */
extern int Arcom_ESS_Error_Number;
extern char Arcom_ESS_Error_String[];

#endif
