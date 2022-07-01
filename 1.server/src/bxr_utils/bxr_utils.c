/*---------------------------------------------------------------------------*/
/* Name : bxr_utils.c                                                        */
/* Info : Blue X-ray G Utils Func                                            */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "iniparser.h"

#define BXR_PGM		"Blue X-ray G Utils"                    /* Program Name	*/
#define BXR_VER		"1.12 (" __TIMESTAMP__ ")"				/* Program Ver	*/

/*---------------------------------------------------------------------------*/
/* B_Log() - Blue X-ray Log Func                                             */
/*---------------------------------------------------------------------------*/
void B_Log(const char *logfile, int logflag, int logline, const char *fmt, ...)
{
	int fd, len;
	struct	timeval	t;
	struct tm *tm;
	static char	fname[128];
	static char sTmp[1024*2], sFlg[5];

	va_list ap;

//	syslog(LOG_INFO, "[BxrG] %s(), [%s/%d/%d]", __FUNCTION__, logfile, logflag, logline);
	if( LOGMODE < logflag ) return;

	switch( logflag )
	{
		case	0 :
		case	1 :
			sprintf( sFlg, "E" );
			break;
		case	2 :
			sprintf( sFlg, "I" );
			break;
		case	3 :
			sprintf( sFlg, "W" );
			break;
		case	4 :
		default   :
#ifndef _BXDBG
			return;
#endif
			sprintf( sFlg, "D" );

			break;
	}

	memset( sTmp, 0x00, sizeof(sTmp) );
	gettimeofday(&t, NULL);
	tm = localtime(&t.tv_sec);

	/* [HHMMSS ssssss flag __LINE__] */
	len = sprintf(sTmp, "[%5d:%08x/%02d%02d%02d %06d/%s:%4d:%4d]",
			getpid(), (unsigned int)pthread_self(),
			tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec, 
			sFlg, errno, logline );

	va_start(ap, fmt);
	vsprintf((char *)&sTmp[len], fmt, ap);
	va_end(ap);

	sprintf(fname, "%s/%s.%02d%02d", LOGPATH, logfile, tm->tm_mon+1, tm->tm_mday);
//	syslog(LOG_INFO, "[BxrG] %s(), logfile[%s]", __FUNCTION__, fname );
	if (access(fname, 0) != 0)
	{
#if 0
		char dir[1024];
		char *ptr = strrchr((char *)logfile, '/');

		sprintf( dir, "mkdir -p %.*s ", strlen(logfile)-strlen(ptr), logfile );
		system( dir );
#endif
		fd = open(fname, O_WRONLY|O_CREAT, 0660);
	}
	else
		fd = open(fname, O_WRONLY|O_APPEND, 0660);

	if (fd >= 0)
	{
//		syslog(LOG_INFO, "[BxrG] %s(), [%.*s]", __FUNCTION__, sTmp, strlen(sTmp));
		write(fd, sTmp, strlen(sTmp));
		close(fd);
	}

	return;

}/* End of B_Log() */

/*---------------------------------------------------------------------------*/
/* Blue X-ray G Init Function                                                */
/*---------------------------------------------------------------------------*/
int BXR_Init(char *cfgpath)
{
    struct stat sb;
	char	cfgfile[512];
	int		rtn, len;

	/* Blue X-ray G Profile Path Check */
	len = sprintf( cfgfile, "%s", cfgpath );
	len = sprintf( cfgfile+len, "/%s", "BlueXrayG.conf" );
	
	if((rtn=access(cfgfile, R_OK)) < 0 )
	{
		/* Blue X-ray G Profile Defautl Data Create */
		syslog(LOG_WARNING, "[BxrG] %s(), Conf File Not Found... '%s'", __FUNCTION__, cfgfile );
		return( BXR_RESULT_ERROR );
	}

    if( stat( cfgfile, &sb ) == FALSE )
	{
		syslog(LOG_ERR, "[BxrG] %s(), Conf File Check Error... '%s':(%d)", __FUNCTION__, cfgfile, errno);
		return( BXR_RESULT_ERROR );
	}

	/* 환경 파일 읽어 적용 */
	if( CONFTIME != sb.st_mtime )
	{
		syslog(LOG_INFO, "[BxrG] %s(), Load Conf File Start...", __FUNCTION__);

		if( (rtn = BXR_LoadConf( cfgfile )) == FALSE ) 
		{
			syslog(LOG_INFO, "[BxrG] %s(), Conf File Load Error...", __FUNCTION__);
			return( BXR_RESULT_ERROR );
		}
	}
	
	CONFTIME = sb.st_mtime;
		
	syslog(LOG_INFO, "[BxrG] %s(), Load Conf File End...", __FUNCTION__);

	/* Module Information */
	B_Log( INF, "INF >> Program : [%s]\n", BXR_program() );
	B_Log( INF, "INF >> Version : [%s]\n", BXR_version() );
	
    return( BXR_RESULT_OK );

}/* End of BXR_Init() */

/*---------------------------------------------------------------------------*/
/* load configure values form 'BlueXrayG.conf' file.                         */
/*---------------------------------------------------------------------------*/
int BXR_LoadConf( char *cfgfile )
{
	dictionary  *ini;
	const char	*s;

	ini = iniparser_load(cfgfile);
	if( ini == NULL )
	{
		syslog(LOG_ERR, "[BxrG] %s(), Conf File Load Error... '%s'", __FUNCTION__, cfgfile );
       	return( BXR_RESULT_ERROR );
	}

//#ifdef _BXDBG_INI
	iniparser_dump(ini, stdout);
//#endif

	/* Log File Path */
	s = iniparser_getstring(ini, "debug:logpath", NULL);
	sprintf( LOGPATH, "%s", s ? s : getenv("HOME") );
	//sprintf( LOGPATH, "%s", s ? s : "/var/log/bluexg" );

	/* Log printf mode */	
	LOGMODE = iniparser_getint(ini, "debug:logmode", 0);

	/* Blue X-ray G Server IP, PORT */
	s = iniparser_getstring(ini, "config:bxrgip", NULL);
	sprintf( SERV_IP, "%s", s ? s : "0.0.0.0" );
	SERV_PORT = iniparser_getint(ini, "config:bxrgport", 9500);

	/* Blue X-ray G DBMS IP, PORT, NAME */
	s = iniparser_getstring(ini, "config:dbmsip", NULL);
	sprintf( DBMS_IP, "%s", s ? s : "127.0.0.1" );

	DBMS_PORT = iniparser_getint(ini, "config:dbmsport", 3306);

	s = iniparser_getstring(ini, "config:dbmsname", NULL);
	sprintf( DBMS_NAME, "%s", s ? s : "bluexg_v2" );
	
	/* Log File Name */
	sprintf( LOGFILE, "%s.log", BXR_GetProcName() );
	syslog(LOG_INFO, "[BxrG] %s(), Log file... [%s]", __FUNCTION__, LOGFILE );

	iniparser_freedict(ini);

    return( BXR_RESULT_OK );

}/* End of BXR_LoadConf() */

/*---------------------------------------------------------------------------*/
/* Get Process name Function                                                 */
/*---------------------------------------------------------------------------*/
char *BXR_GetProcName()
{
    FILE *fp;
    static char pbuff[256];  
    static char pname[128]; 
    char fname[128];  
    int i;

    sprintf( pname, "BlueXrayG" );
    sprintf( fname, "/proc/%d/status", getpid() );  
    
    fp = fopen(fname, "r");  
    if(fp == NULL) 
    {   
		syslog(LOG_ERR, "[BxrG] %s(), Process Name Not Found... '%s'", 
				__FUNCTION__, fname );
		return( (char *)pname );  
    }   

    fgets(pbuff, sizeof(pbuff), fp);
    pbuff[strlen(pbuff)-1] = 0x00;
    for( i=5 ; i < strlen(pbuff) ; i++ )
    {   
        if( pbuff[i] >= 'a' && pbuff[i] <= 'z' ) break;
        if( pbuff[i] >= 'A' && pbuff[i] <= 'Z' ) break;
    }

    fclose(fp);  
        
    if( i < strlen(pbuff) )
		return( (char *)&pbuff[i] );
    else
		return( (char *)pname );  
    
}/* End of BXR_GetProcName() */


#if 0
/*---------------------------------------------------------------------------*/
/* Get IP Address Function                                                  */
/*---------------------------------------------------------------------------*/
char *BXR_GetIpAddress()
{
	int sock;
	struct ifreq ifr;
	struct sockaddr_in *sin;
    static ip_addr[16];

	strcpy(ip_addr, inet_ntoa(sin->sin_addr));
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
	{
		dp(4, "socket");
		return 0;
	}

	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(sock, SIOCGIFADDR, &ifr)< 0)    
	{
		dp(4, "ioctl() - get ip");
		close(sock);
		return 0;
	}

	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	memset( ip_addr, 0x00, sizeof(ip_addr) );
	strcpy(ip_addr, inet_ntoa(sin->sin_addr));

	close(sock);
	return( (char *)ip_addr );  

}/* End of BXR_GetIpAddress() */
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
char *BXR_program()
{
    return( (char *)BXR_PGM );

}/* End of BXR_program() */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
char *BXR_version()
{
    return( (char *)BXR_VER );

}/* End of BXR_version() */

/*---------------------------------------------------------------------------*/
/* Create Daemon Process Function                                            */
/*---------------------------------------------------------------------------*/
int	BXR_Daemonize()
{
	int pid;
 
	B_Log( INF, "Process Daemonize Start...\n" );
	
	// fork 생성
	pid = fork();
	B_Log( DBG, "pid = [%d] \n", pid);
 
	// fork 에러 발생 시 로그 남긴 후 종료
	if(pid < 0){
		printf("fork Error... : return is [%d] \n", pid );
		perror("fork error : ");
		exit(0);
	// 부모 프로세스 로그 남긴 후 종료
	}else if (pid > 0){
		printf("child process : [%d] - parent process : [%d] \n", pid, getpid());
		exit(0);
	// 정상 작동시 로그
	}else if(pid == 0){
		B_Log( INF, "process : [%d]\n", getpid());
	}
 
	// 터미널 종료 시 signal의 영향을 받지 않도록 처리
	signal(SIGHUP, SIG_IGN);
	close(0);
	close(1);
	close(2);
 
	// 실행위치를 Root로 변경
	chdir("/");
 
	// 새로운 세션 부여
	setsid();

	B_Log( INF, "Process Daemonize End...\n" );

    return( BXR_RESULT_OK );
	
}/* End of BXR_Daemonize() */


/*---------------------------------------------------------------------------*/
/* BXR_GetDateTime() - Get SysDateTime + milisec                             */
/*---------------------------------------------------------------------------*/
char *BXR_GetDateTime()
{
	int  len;
	struct	timeval	t;
	struct tm *tm;
	static char sTmp[64];

	memset( sTmp, 0x00, sizeof(sTmp) );
	gettimeofday(&t, NULL);
	tm = localtime(&t.tv_sec);

	/* 2021-08-02 18:24:41.000 */
	/* [HHMMSS ssssss flag __LINE__] */
	len = sprintf(sTmp, "%4d-%02d-%02d %02d:%02d:%02d.%03d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
			tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec/1000 );
	
	B_Log( DBG, "BXR_GetDateTime : [%s]\n", sTmp );

	return( sTmp );

}/* End of BXR_GetDateTime() */

#if 0
/*---------------------------------------------------------------------------*/
/* BXR_StartEnd()                                                            */
/*---------------------------------------------------------------------------*/
int (MYSQL  *DBConn, char *RcvMsg, int RcvLen, char *TR_Code, int rtn)
{
	char	Sql_Buf[1024*10*2];

	// Construct the SQL statement
	sprintf(Sql_Buf, 
			"INSERT INTO `%s`.`tb_law_result` (`state`,`cDate`, `func`, `iData`, `eCode`) VALUES('N', now(3), '%s', '%.*s', %03d )", 
			DBMS_NAME, TR_Code, RcvLen, RcvMsg, rtn );

	if( mysql_query( DBConn, Sql_Buf ) != 0 )
	{
		B_Log( ERR, "Query failed  with the following message:[%s]\n", mysql_error(DBConn));
		return( BXR_DBMS_ERROR );
	}

	B_Log( DBG, "Log inserted into the database success...\n" );

	return( rtn );

}/* End of Log_DBMS_Insert() */

#endif

