/******************************************************************************/
/*  Header      : bxrerrno.h                                                  */
/*  Description : Common Error Code Header                                    */
/*  Rev. History: Ver   Date    Description                                   */
/*                ----  ------- ----------------------------------------------*/
/*                0.5   2020-06 Initial version                               */
/******************************************************************************/
#ifndef BLUEXRAY_UTILS_H
#define BLUEXRAY_UTILS_H

#include <pthread.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

/* Config Ini Define */
int		LOGMODE; 						/* 0:Not, 1:Debug, 2:WRN, 3:ERR */
char 	LOGPATH		[2048];
char 	LOGFILE		[2048];
char 	SERV_IP		[  32];
int	 	SERV_PORT;
char 	DBMS_IP		[  32];
int	 	DBMS_PORT;
char 	DBMS_NAME	[ 128];
time_t	CONFTIME;

void	B_Log(const char *, int , int , const char *, ...);
int		BXR_Init(char *);
int		BXR_LoadConf();
int		BXR_Daemonize();
char	*BXR_program();
char	*BXR_version();
char	*BXR_GetProcName();
char	*BXR_GetDateTime();
// int		BXR_REQ( char *, char *, int );
#endif

