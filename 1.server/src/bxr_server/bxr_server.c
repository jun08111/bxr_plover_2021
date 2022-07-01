/*---------------------------------------------------------------------------*/
/* Name : bxr_server.c                                                       */
/* Info : Blue X-ray Server                                                  */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "bxrs.h"

#define SVR_VER		"1.0 (" __TIMESTAMP__ ")"				/* Program Ver	*/

int Log_Trans_Check(char *, int, char *);
int Log_DBMS_Insert(MYSQL  *, char *, int , char *, int );

/*---------------------------------------------------------------------------*/
/* Process Stoping Signal                                                    */
/*---------------------------------------------------------------------------*/
void Proc_Stop(int signum)
{
	if (signum == SIGUSR1)
	{
		B_Log( INF, "Blue X-ray Server Stop ...[%s %s]\n",__DATE__, __TIME__ );
		BXR_StartEnd( BXR_END );
		kill( getpid(), SIGQUIT );
	}

}/* End of Proc_Stop() */


/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
void main()
{
	nng_socket	sock;
	char		SERV_URL[  64];
	char		PATH_CFG[1024];
	int			rtn;
	char 		sbuf[SEND_SOCK_BUF_SZ];

	if( getenv("BXRG_HOME") == NULL )
	{
		syslog(LOG_WARNING, "[BxrG] $(BXRG_HOME) Not Found... " );
		return;
	}

	sprintf( PATH_CFG, "%s/conf", getenv("BXRG_HOME") );
	BXR_Init( PATH_CFG );
	B_Log( INF, "BXR_Init End...\n" );
	
	BXR_StartEnd( BXR_START );
	
	BXR_Daemonize();
	B_Log( INF, "BXR_Daemonize End...\n" );
	B_Log( INF, "Blue X-ray Server Start...[%s %s]\n",__DATE__, __TIME__ );

	signal(SIGUSR1, Proc_Stop);

	if ((rtn = nng_rep0_open(&sock)) != 0) 
	{
		B_Log( ERR, "nng_open : [%s]\n", nng_strerror(rtn));
		return;
	}

//	sprintf( SERV_URL, "tcp://%s:%d", SERV_IP, SERV_PORT );
	sprintf( SERV_URL, "tcp://0.0.0.0:%d", SERV_PORT );
	if ((rtn = nng_listen(sock, SERV_URL, NULL, 0)) != 0) 
	{
		B_Log( ERR, "nng_listen : [%s][%s]\n", SERV_URL, nng_strerror(rtn));
		return;
	}

	memset( sbuf, 0x00, sizeof(sbuf) );
	for (;;) 
	{
		char *		rbuf = NULL;
		size_t		rlen = 0;
	
		B_Log( DBG, "SERVER: RECV start\n" );
		if ((rtn = nng_recv(sock, &rbuf, &rlen, NNG_FLAG_ALLOC)) != 0) 
		{
			B_Log( ERR, "nng_recv : [%s]\n", nng_strerror(rtn));
			nng_free(rbuf, rlen);
			break;
		}

		B_Log( DBG, "SERVER: RECV DATA : [%d][%s]\n", rlen, rbuf);

		if (rlen != 0 ) 
		{
			/* Recv Log Data Check */
			rtn = Log_Trans_Check(rbuf, rlen, sbuf);
			B_Log( DBG, "SERVER: SEND DATA : [%d][%s]\n", strlen(sbuf), sbuf);
			
//			rtn = nng_send(sock, rbuf, strlen(rbuf), NNG_FLAG_ALLOC);
			if(( rtn = nng_send(sock, sbuf, strlen(sbuf), NNG_FLAG_NONBLOCK) ) != 0 ) 
			{
				B_Log( ERR, "nng_send : [%s]\n", nng_strerror(rtn));
				nng_free(rbuf, rlen);
				break;
			}

			B_Log( DBG, "SERVER: SEND E n d : [%d]\n", rtn );
			
			continue;
		}
	
		// Unrecognized command, so toss the buffer.
		B_Log( WRN, "Recv Data Not Found : [%s]\n", nng_strerror(rtn) );
		nng_free(rbuf, rlen);
	}
	B_Log( ERR, "Blue X-ray Server Process Stopping : errno[%d] nng[%s]\n", 
				errno, nng_strerror(rtn) );
	
	return;

}/* End of Main() */


/*---------------------------------------------------------------------------*/
/* Log_Trans_Check()                                                         */
/*---------------------------------------------------------------------------*/
int Log_Trans_Check(char *RcvMsg, int RcvLen, char *SndMsg)
{
	MYSQL	  * DBConn;
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	char		APP_Ver[16];
	char		APP_Uid[64];
	char		TR_Code[16];
	int			rtn = TRUE;


	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Rep_Obj, *Rep_Obj_Data, *Val_Obj;

    Req_Obj = json_tokener_parse(RcvMsg);
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
            json_object_to_json_string_ext( Req_Obj, 
                                            JSON_C_TO_STRING_SPACED | 
                                            JSON_C_TO_STRING_PRETTY ) );

	// Request Data Parse
	if( json_object_object_get_ex( Req_Obj, "RQ", &Req_Obj_Data ) == json_type_null )
	{
		B_Log( ERR, "Req_Obj from Head Get Error...\n" );
		return( BXR_MSGFORMAT_ERROR );
	}

	json_object_object_get_ex( Req_Obj_Data, "Func",     &Val_Obj );
	sprintf( TR_Code, "%s", json_object_get_string(Val_Obj) );	
	B_Log( DBG, "Req_Obj from Func : [%s]\n", TR_Code );
	
	json_object_object_get_ex( Req_Obj_Data, "RqRp",     &Val_Obj );
	B_Log( DBG, "Req_Obj from RqRp : [%s]\n", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Uuid",     &Val_Obj );
	sprintf( APP_Uid, "%s", json_object_get_string(Val_Obj) );	
	B_Log( DBG, "Req_Obj from Uuid : [%s]\n", APP_Uid );
	
	json_object_object_get_ex( Req_Obj_Data, "AppVer",   &Val_Obj );
	sprintf( APP_Ver, "%s", json_object_get_string(Val_Obj) );	
	B_Log( DBG, "Req_Obj from AppV : [%s]\n", APP_Ver );

	// DBMS Connect 
	DBConn = mysql_init(NULL);
	if( mysql_real_connect( DBConn, DBMS_IP, DBMS_USER, DBMS_PASS, DBMS_NAME, DBMS_PORT, NULL, 0) == NULL)
	{
		B_Log( ERR, "The authentication failed with the following message:[%s]\n", mysql_error(DBConn));
		return( BXR_DBMS_ERROR );
	}

	if( memcmp( TR_Code, "JFSRINIT", strlen(TR_Code) ) == 0 ) TR_CODE_DEF = JFSRINIT;
	else
	if( memcmp( TR_Code, "JFSCUSCK", strlen(TR_Code) ) == 0 ) TR_CODE_DEF = JFSCUSCK;
	else
	if( memcmp( TR_Code, "JFSCPSCK", strlen(TR_Code) ) == 0 ) TR_CODE_DEF = JFSCPSCK;
	else
	if( memcmp( TR_Code, "JFSCUWCK", strlen(TR_Code) ) == 0 ) TR_CODE_DEF = JFSCUWCK;
	else
	if( memcmp( TR_Code, "JFSCPWCK", strlen(TR_Code) ) == 0 ) TR_CODE_DEF = JFSCPWCK;
	else
		B_Log( ERR, "TR Code Not Define : [%s]\n", TR_Code );

	// Reply Data Create
    Rep_Obj = json_object_new_object();
    Rep_Obj_Data = json_object_new_object();
	
	// TR CODE Process
    switch (TR_CODE_DEF)
    {
    case JFSRINIT :  
 		/*--------------------------*/
		/* Client App Version Check */
		/*--------------------------*/
		if(( rtn=Log_App_Check( DBConn, APP_Ver )) != TRUE )
			break;
		
		/*--------------------------*/
		/* Client User Info Check   */
		/*--------------------------*/
		if(( rtn=Log_User_Check( DBConn, APP_Uid, Rep_Obj_Data )) != TRUE )
			break;
       
		break;
	case JFSCUSCK :  
		/*--------------------------*/
		/* TR : JFSCUSCK Process    */
		/*--------------------------*/
		if(( rtn=Log_JFSCUSCK( DBConn, Rep_Obj_Data )) != TRUE )
			break;
 
        break;
	case JFSCPSCK :  
		/*--------------------------*/
		/* TR : JFSCUSCK Process    */
		/*--------------------------*/
		if(( rtn=Log_JFSCPSCK( DBConn, Rep_Obj_Data )) != TRUE )
			break;
 
        break;
	case JFSCUWCK :  
		/*--------------------------*/
		/* TR : JFSCUSCK Process    */
		/*--------------------------*/
		if(( rtn=Log_JFSCUWCK( DBConn, Rep_Obj_Data )) != TRUE )
			break;
 
        break;
	case JFSCPWCK :  
		/*--------------------------*/
		/* TR : JFSCUSCK Process    */
		/*--------------------------*/
		if(( rtn=Log_JFSCPWCK( DBConn, Rep_Obj_Data )) != TRUE )
			break;
 
        break;
    default:
		B_Log( WRN, "Log_Trans_Check Plz : [%s]\n", TR_Code );
		rtn = BXR_MSGFORMAT_ERROR;
        break;
    }

	/*--------------------------*/
	/* Recv Log DBMS Insert     */
	/*--------------------------*/
	if((rtn=Log_DBMS_Insert(DBConn, RcvMsg, RcvLen, TR_Code, rtn )) != TRUE )
		B_Log( WRN, "Log DBMS Insert Error : [%s]\n", TR_Code );
	
	mysql_close(DBConn);

	json_object_object_add(Rep_Obj_Data, "Func",	json_object_new_string( TR_Code ));
	json_object_object_add(Rep_Obj_Data, "RqRp",	json_object_new_string( "SC" ));
//	json_object_object_add(Rep_Obj_Data, "DateTime",	json_object_new_int( BXR_GetDateTime() ));
	json_object_object_add(Rep_Obj_Data, "DateTime",	json_object_new_string( BXR_GetDateTime() ));
	
	if( rtn != TRUE )
	{
		json_object_object_add(Rep_Obj_Data, "eType",	json_object_new_string( "E" ));
		json_object_object_add(Rep_Obj_Data, "eCode",	json_object_new_int( rtn ));
		json_object_object_add(Rep_Obj_Data, "eMesg",	json_object_new_string( "Error" ));
	}
	else
	{
		json_object_object_add(Rep_Obj_Data, "eType",	json_object_new_string( "S" ));
		json_object_object_add(Rep_Obj_Data, "eCode",	json_object_new_int( rtn ));
		json_object_object_add(Rep_Obj_Data, "eMesg",	json_object_new_string( "Sucess" ));
	}

	/* Data 영역 Reply 등록 */
	json_object_object_add(Rep_Obj, "RP",	Rep_Obj_Data);
 
    memset( SndMsg, 0x00, strlen(SndMsg) );

	sprintf(SndMsg, "%s",
			json_object_to_json_string_ext( Rep_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ) );

	B_Log( DBG, "Rep_Obj from str:\n---\n%s\n---\n", 
			json_object_to_json_string_ext( Rep_Obj, 
											JSON_C_TO_STRING_SPACED | 
											JSON_C_TO_STRING_PRETTY ) );
	return( rtn );

}/* End of Log_Trans_Check() */


/*---------------------------------------------------------------------------*/
/* Log_DBMS_Insert()                                                         */
/*---------------------------------------------------------------------------*/
int Log_DBMS_Insert(MYSQL  *DBConn, char *RcvMsg, int RcvLen, char *TR_Code, int rtn)
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

/*---------------------------------------------------------------------------*/
/* Log_App_Check()                                                           */
/*---------------------------------------------------------------------------*/
int Log_App_Check(MYSQL  *DBConn, char *APP_Ver)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;


	/*--------------------------*/
	/* Client App Version Check */
	/*--------------------------*/
	sprintf( Sql_Buf, "SELECT IFNULL(optValue, \"NULL\") \
						FROM %s.tb_cfg_bxrg \
						WHERE optKey = 'CLIENT_VER' \
						and optFlag = 'Y'", 
						DBMS_NAME );
	B_Log( DBG, "Mysql query : [%s]\n", Sql_Buf );
	if( mysql_query( DBConn, Sql_Buf ) )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	Sql_Rtn = mysql_store_result(DBConn);
	if( Sql_Rtn == NULL )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	if( mysql_num_rows(Sql_Rtn) != 0 )
	{
		Row = mysql_fetch_row(Sql_Rtn);
		if( memcmp( APP_Ver, Row[0], strlen(APP_Ver) ) != 0 )
		{
			B_Log( WRN, "Client APP Version Update Return : [C/%s, S/%s]\n", APP_Ver, Row[0] );
			return( BXR_APPUPDATE_ERROR );
		}
	}

	return( TRUE );

}/* End of Log_App_Check() */


/*---------------------------------------------------------------------------*/
/* Log_User_Check()                                                           */
/*---------------------------------------------------------------------------*/
int Log_User_Check(MYSQL  *DBConn, char *APP_Uid, json_object *Rep_Obj_Data)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;

	/*--------------------------*/
	/* Client User Info Check   */
	/*--------------------------*/
	sprintf( Sql_Buf, 
"SELECT	C.`cType`, \
		U.`userID`, \
		U.`userPwd`, \
		U.`userName`, \
		IFNULL((SELECT  kValue  FROM `bluexg_v2`.tb_info_code WHERE pKey = 'RNK1' AND cKey = U.rankCode ), '없음'), \
		IFNULL((SELECT  kValue  FROM `bluexg_v2`.tb_info_code WHERE pKey = 'POS1' AND cKey = U.positionCode ), '없음'), \
		D.`deptName` \
FROM	`%s`.`tb_info_join` as J,   \
		`%s`.`tb_info_client` as C, \
		`%s`.`tb_info_user` as U,   \
		`%s`.`tb_info_dept` as D   \
WHERE 	J.uuid = '%s'              \
	AND	J.uuid = C.uuid            \
	AND J.userId = U.userId        \
	AND U.deptCode = D.deptCode    \
	AND J.state != 'D'", 
			DBMS_NAME, DBMS_NAME, DBMS_NAME, DBMS_NAME, APP_Uid );

	B_Log( DBG, "Mysql query : [%s]\n", Sql_Buf );
	if( mysql_query( DBConn, Sql_Buf ) )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	Sql_Rtn = mysql_store_result(DBConn);
	if( Sql_Rtn == NULL )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	if( mysql_num_rows(Sql_Rtn) != 0 )
	{
		Row = mysql_fetch_row(Sql_Rtn);
		
		if( memcmp( Row[0], "M", sizeof("M") ) == 0 )
		{
			B_Log( WRN, "Multi User Client: [%.1s]\n", Row[0] );
			return( BXR_MULTIUSER_ERROR );
		} 

		json_object_object_add(Rep_Obj_Data, "cType",   json_object_new_string( Row[0]==NULL?"N":Row[0] ));
		json_object_object_add(Rep_Obj_Data, "uID",     json_object_new_string( Row[1] ));
		json_object_object_add(Rep_Obj_Data, "uPwd",    json_object_new_string( Row[2] ));
		json_object_object_add(Rep_Obj_Data, "uName",   json_object_new_string( Row[3] ));
		json_object_object_add(Rep_Obj_Data, "uRankNm", json_object_new_string( Row[4]==NULL?"없음":Row[4] ));
		json_object_object_add(Rep_Obj_Data, "uPosiNm", json_object_new_string( Row[5]==NULL?"없음":Row[5] ));
		json_object_object_add(Rep_Obj_Data, "uDeptNm", json_object_new_string( Row[6]==NULL?"없음":Row[6] ));
	}
	else
	{
		B_Log( WRN, "User Not Found...[%d]\n", BXR_USERNOTFOUND_ERROR );
		return( BXR_USERNOTFOUND_ERROR );
	}

	return( TRUE );

}/* End of Log_User_Check() */


/*---------------------------------------------------------------------------*/
/* Log_JFSCUSCK()                                                            */
/*---------------------------------------------------------------------------*/
int Log_JFSCUSCK(MYSQL  *DBConn, json_object *Rep_Obj_Data)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;

#if 0

{
  "RQ": {
    "Func": "JFSCFPCK",
    "RqRp": "CS",
    "DateTime": "2021-08-09 09:47:15.997",
    "Uuid": "8f3748b3-d3f3-4ea6-9539-4dcb49e1ea08",
    "AppVer": "1.0.0803.0",
    "FileCount": 4,
    "Jumin": 33,
    "Driver": 0,
    "Foreign": 0,
    "PassPort": 0,
    "WorkStart": "2021-08-09 09:47:15.889",
    "WorkEnd": "2021-08-09 09:47:15.966"
  }
}

	/*--------------------------*/
	/* Client User Info Check   */
	/*--------------------------*/
	sprintf( Sql_Buf, "SELECT C.`cType`,                 \
							U.`userID`,                  \
							U.`userPwd`,                 \
							U.`userName`,                \
							U.`rankCode`,                \
							U.`positionCode`,            \
							D.`deptName`                 \
						FROM `%s`.`tb_info_join` as J,   \
							 `%s`.`tb_info_client` as C, \
							 `%s`.`tb_info_user` as U,   \
							 `%s`.`tb_info_dept` as D    \
						WHERE J.uuid = '%s'              \
						  AND J.uuid = C.uuid            \
						  AND J.userId = U.userId        \
						  AND U.deptCode = D.deptCode    \
						  AND J.state != 'D'", DBMS_NAME, DBMS_NAME, DBMS_NAME, DBMS_NAME, APP_Uid );

	B_Log( DBG, "Mysql query : [%s]\n", Sql_Buf );
	if( mysql_query( DBConn, Sql_Buf ) )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	Sql_Rtn = mysql_store_result(DBConn);
	if( Sql_Rtn == NULL )
	{
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
		return( BXR_DBMS_ERROR );
	}

	if( mysql_num_rows(Sql_Rtn) != 0 )
	{
		Row = mysql_fetch_row(Sql_Rtn);
		
		if( memcmp( Row[0], "M", sizeof("M") ) == 0 )
		{
			B_Log( WRN, "Multi User Client: [%.1s]\n", Row[0] );
			return( BXR_MULTIUSER_ERROR );
		} 

		json_object_object_add(Rep_Obj_Data, "cType",   json_object_new_string( Row[0]==NULL?"N":Row[0] ));
		json_object_object_add(Rep_Obj_Data, "uID",     json_object_new_string( Row[1] ));
		json_object_object_add(Rep_Obj_Data, "uPwd",    json_object_new_string( Row[2] ));
		json_object_object_add(Rep_Obj_Data, "uName",   json_object_new_string( Row[3] ));
		json_object_object_add(Rep_Obj_Data, "uRankNm", json_object_new_string( Row[4]==NULL?"없음":Row[4] ));
		json_object_object_add(Rep_Obj_Data, "uPosiNm", json_object_new_string( Row[5]==NULL?"없음":Row[5] ));
		json_object_object_add(Rep_Obj_Data, "uDeptNm", json_object_new_string( Row[6]==NULL?"없음":Row[6] ));
	}
	else
	{
		B_Log( WRN, "User Not Found...[%d]\n", BXR_USERNOTFOUND_ERROR );
		return( BXR_USERNOTFOUND_ERROR );
	}

#endif

	return( TRUE );

}/* End of Log_JFSCUSCK() */


/*---------------------------------------------------------------------------*/
/* Log_JFSCPSCK()                                                            */
/*---------------------------------------------------------------------------*/
int Log_JFSCPSCK(MYSQL  *DBConn, json_object *Rep_Obj_Data)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;

	return( TRUE );

}/* End of Log_JFSCPSCK() */


/*---------------------------------------------------------------------------*/
/* Log_JFSCUWCK()                                                            */
/*---------------------------------------------------------------------------*/
int Log_JFSCUWCK(MYSQL  *DBConn, json_object *Rep_Obj_Data)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;

	return( TRUE );

}/* End of Log_JFSCUWCK() */


/*---------------------------------------------------------------------------*/
/* Log_JFSCPWCK()                                                            */
/*---------------------------------------------------------------------------*/
int Log_JFSCPWCK(MYSQL  *DBConn, json_object *Rep_Obj_Data)
{
    MYSQL_RES *	Sql_Rtn; 
	MYSQL_ROW	Row;
	char		Sql_Buf[1024*10];
	int			rtn = TRUE;

	return( TRUE );

}/* End of Log_JFSCPWCK() */

/*---------------------------------------------------------------------------*/
/* BXR_StartEnd()                                                            */
/*---------------------------------------------------------------------------*/
int BXR_StartEnd( int InOutFlag )
{
	MYSQL	  * DBConn;
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	int			rtn = TRUE;

	// DBMS Connect 
	DBConn = mysql_init(NULL);
	if( mysql_real_connect( DBConn, DBMS_IP, DBMS_USER, DBMS_PASS, DBMS_NAME, DBMS_PORT, NULL, 0) == NULL)
	{
		B_Log( ERR, "The authentication failed with the following message:[%s]\n", mysql_error(DBConn));
		return( BXR_DBMS_ERROR );
	}
	
	Sql_Len = sprintf(Sql_Buf, 
				"INSERT INTO `%s`.`tb_log_history` ( `mType`, `mInfo`, `mVersion`, `mIp`, \
								 `inOutFlag`, `inOutTime`, `userId`, `cDate` ) VALUES ( ", DBMS_NAME );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", "SP01" );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", BXR_GetProcName() );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", SVR_VER );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", SERV_IP );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%d', ", InOutFlag );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, " %s,  ", "now(3)" );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", getlogin() );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, " %s ) ", "now(3)" );

	B_Log( DBG, "Mysql query : [%s]\n", Sql_Buf );
	if( mysql_query( DBConn, Sql_Buf ) != 0 )
	{
		B_Log( ERR, "Query failed  with the following message:[%s]\n", mysql_error(DBConn));
		return( BXR_DBMS_ERROR );
	}

	B_Log( DBG, "Log inserted into the database success...\n" );

	mysql_close(DBConn);

	return( TRUE );

}/* End of BXR_StartEnd() */

