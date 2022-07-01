/*---------------------------------------------------------------------------*/
/* Name : bxr_parser.c                                                       */
/* Info : Blue X-ray Log Parser                                              */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#include "bxrcommon.h"
#include "bxrutils.h"
#include "bxrs.h"

#define SVR_VER		"1.0 (" __TIMESTAMP__ ")"				/* Program Ver	*/

int Log_DBMS_Insert(char *, int );

/*---------------------------------------------------------------------------*/
/* Process Stoping Signal                                                    */
/*---------------------------------------------------------------------------*/
void Proc_Stop(int signum)
{
	if (signum == SIGUSR1)
	{
		B_Log( INF, "Blue X-ray Parser Stop ...[%s %s]\n",__DATE__, __TIME__ );
		BXR_StartEnd( BXR_END );
		kill( getpid(), SIGQUIT );
	}

}/* End of Proc_Stop() */


/*---------------------------------------------------------------------------*/
/* Main()                                                                    */
/*---------------------------------------------------------------------------*/
void main()
{
	MYSQL *		DBConn;
    MYSQL_ROW   Sql_Row;
    MYSQL_RES *	Sql_Rtn; 
	char		SERV_URL[   64];
	char		PATH_CFG[ 1024];
	char		Sql_Buf [1024*10];
	int			rtn;

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
	
	B_Log( INF, "Blue X-ray Parser Start...[%s %s]\n",__DATE__, __TIME__ );
	signal(SIGUSR1, Proc_Stop);

	DBConn = mysql_init(NULL);
	if( mysql_real_connect( DBConn, DBMS_IP, DBMS_USER, DBMS_PASS, DBMS_NAME, DBMS_PORT, NULL, 0) == NULL)
	{
		B_Log( ERR, "The authentication failed with the following message:[%s]\n", mysql_error(DBConn));
		return;
	}

	for (;;) 
	{
		sprintf( Sql_Buf, "SELECT idx, state, cDate, func, iData \
				FROM `%s`.`tb_law_result` WHERE state = 'N' ORDER BY cDate;", DBMS_NAME );
		if( mysql_query(DBConn, Sql_Buf ) != 0)
		{
			B_Log( ERR, "Mysql query error : [%s]\n", mysql_error(DBConn));
			return;
		}
		
		Sql_Rtn = mysql_store_result(DBConn);
		
		while ( (Sql_Row = mysql_fetch_row(Sql_Rtn)) != NULL )
		{
			B_Log( DBG, "idx[%3d] flag[%s] cdate[%s] func[%s]\n", 
					atoi(Sql_Row[0]), Sql_Row[1], Sql_Row[2], Sql_Row[3]  );

			/* Log DBMS Insert */
			if( Log_Paser_Trans(DBConn, Sql_Row) == FALSE )
				B_Log( ERR, "Log_DBMS_Insert() Error...\n" );
			else
				B_Log( DBG, "Log_DBMS_Insert() End...\n" );

			usleep(1000);
		}
			
		B_Log( DBG, "Log Paser Complet...\n");

		mysql_free_result(Sql_Rtn);

		sleep(3);
		continue;
	}
	
	return;

}/* End of Main() */


/*---------------------------------------------------------------------------*/
/* Log_Paser_Trans()                                                         */
/*---------------------------------------------------------------------------*/
int Log_Paser_Trans(MYSQL *DBConn, MYSQL_ROW Sql_Row)
{
    MYSQL_RES *	Sql_Rtn; 
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	int			rtn = TRUE;
	
	if( memcmp( Sql_Row[3], "JFSRINIT", strlen(Sql_Row[3]) ) == 0 ) TR_CODE_DEF = JFSRINIT;
	else
	if( memcmp( Sql_Row[3], "JFSCUSCK", strlen(Sql_Row[3]) ) == 0 ) TR_CODE_DEF = JFSCUSCK;
	else
	if( memcmp( Sql_Row[3], "JFSCPSCK", strlen(Sql_Row[3]) ) == 0 ) TR_CODE_DEF = JFSCPSCK;
	else
	if( memcmp( Sql_Row[3], "JFSCUWCK", strlen(Sql_Row[3]) ) == 0 ) TR_CODE_DEF = JFSCUWCK;
	else
	if( memcmp( Sql_Row[3], "JFSCPWCK", strlen(Sql_Row[3]) ) == 0 ) TR_CODE_DEF = JFSCPWCK;
	else
		B_Log( ERR, "TR Code Not Define : [%s]\n", Sql_Row[3] );

	// TR CODE Process
    switch (TR_CODE_DEF)
    {
    case JFSRINIT :  
		if( Log_JFSRINIT_Insert(  DBConn, Sql_Row[4] ) == FALSE )
			B_Log( ERR, "Log_JFSRINIT_Insert() Error...\n" );
		
		break;
	case JFSCUSCK :  
		if( Log_JFSCUSCK_Insert(  DBConn, Sql_Row[4] ) == FALSE )
			B_Log( ERR, "Log_JFSCUSCK_Insert() Error...\n" );
		
        break;
	case JFSCPSCK :  
		if( Log_JFSCPSCK_Insert(  DBConn, Sql_Row[4] ) == FALSE )
			B_Log( ERR, "Log_JFSCPSCK_Insert() Error...\n" );
		
        break;
	case JFSCUWCK :  
		if( Log_JFSCUWCK_Insert(  DBConn, Sql_Row[4] ) == FALSE )
			B_Log( ERR, "Log_JFSCUWCK_Insert() Error...\n" );
		
        break;
	case JFSCPWCK :  
		if( Log_JFSCPWCK_Insert(  DBConn, Sql_Row[4] ) == FALSE )
			B_Log( ERR, "Log_JFSCPWCK_Insert() Error...\n" );
		
        break;
    default:
		B_Log( WRN, "Log_Trans_Check Plz func : [%s]\n", Sql_Row[3] );
		rtn = BXR_MSGFORMAT_ERROR;
        break;
    }

	if( Log_TBL_Update( DBConn, Sql_Row[0], Sql_Row[2], Sql_Row[3] ) == FALSE )
		return( FALSE );

	return( TRUE );

}/* End of Log_Paser_Trans() */

/*---------------------------------------------------------------------------*/
/* Log Table Update                                                          */
/*---------------------------------------------------------------------------*/
int Log_TBL_Update( MYSQL *DBConn, char *idx, char *cDate, char *func )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	/* Raw Event Log Update */
    Sql_Len = sprintf(Sql_Buf, "UPDATE `%s`.`tb_law_result` \
						SET		`state` = 'C', \
								`uDate` = now(3) \
						WHERE	`idx` = %s AND `state` = 'N' AND `cDate` = '%s' AND `func` = '%s'", 
								DBMS_NAME, idx, cDate, func );

	B_Log( DBG, "Sql:[%d][%s]\n", Sql_Len, Sql_Buf );
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }

	return( TRUE );

}/* End of Log_TBL_Update() */


/*---------------------------------------------------------------------------*/
/* JFSRINIT Log Table Insert                                                 */
/*---------------------------------------------------------------------------*/
int Log_JFSRINIT_Insert( MYSQL *DBConn, char *iData )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse( iData );
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
	
	Sql_Len = sprintf(Sql_Buf, 
				"INSERT INTO `%s`.`tb_log_history` ( \
						`mType`, `mInfo`, `mVersion`, `mIp`, `inOutFlag`, `inOutTime`,  \
						`userId`, `userName`, `deptName`, `uuid`, `cDate` ) VALUES ( ", DBMS_NAME );
	
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", "SP01" );

	json_object_object_get_ex( Req_Obj_Data, "Func",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "AppVer",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "AppIp",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%d', ", BXR_START );
	
	json_object_object_get_ex( Req_Obj_Data, "DateTime",  &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

//	json_object_object_get_ex( Req_Obj_Data, "UserID",     &Val_Obj );
//	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

//	json_object_object_get_ex( Req_Obj_Data, "UserName",     &Val_Obj );
//	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

//	json_object_object_get_ex( Req_Obj_Data, "DeptName",     &Val_Obj );
//	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Uuid",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "(SELECT userId FROM bluexg_v2.v_tb_info_user where uuid = '%s'), ", json_object_get_string(Val_Obj));
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "(SELECT userName FROM bluexg_v2.v_tb_info_user where uuid = '%s'), ", json_object_get_string(Val_Obj));
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "(SELECT deptName FROM bluexg_v2.v_tb_info_user where uuid = '%s'), ", json_object_get_string(Val_Obj));
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%s )", "now(3)" );

	B_Log( DBG, "Mysql query : [%s]\n", Sql_Buf );
	if( mysql_query( DBConn, Sql_Buf ) != 0 )
	{
		B_Log( ERR, "Query failed  with the following message:[%s]\n", mysql_error(DBConn));
		return( BXR_DBMS_ERROR );
	}

	B_Log( DBG, "Log inserted into the database success...\n" );

	/* Event Log Insert */
	B_Log( DBG, "Sql:[%d][%s]\n", Sql_Len, Sql_Buf );

    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }

	return( TRUE );

}/* End of Log_JFSRINIT_Insert() */


/*---------------------------------------------------------------------------*/
/* JFSCUSCK Log Table Insert                                                 */
/*---------------------------------------------------------------------------*/
int Log_JFSCUSCK_Insert( MYSQL *DBConn, char *iData )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse( iData );
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

	Sql_Len = sprintf( Sql_Buf, "INSERT INTO `%s`.`tb_inspect_user` ( `rDate`, \
									`userId`, \
									`sDate`, \
									`eDate`, \
									`uuid`, \
									`targetFileCnt`, \
									`detectFileCnt`, \
									`jumin`, \
									`foreigner`, \
									`passport`, \
									`driver`, \
									`card`, \
									`biz`, \
									`tel`, \
									`cell`, \
									`bank`, \
									`email`, \
									`corp`, \
									`custom1`, \
									`custom2`, \
									`custom3` ", DBMS_NAME );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, ") VALUES ( now(3), " );

	/* User ID Serach */
	json_object_object_get_ex( Req_Obj_Data, "Uuid",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "(select userid from tb_info_join where uuid = '%s'), ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "WorkStart",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "WorkEnd",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "Uuid",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

//	json_object_object_get_ex( Req_Obj_Data, "TargetFileCnt",     &Val_Obj );
	json_object_object_get_ex( Req_Obj_Data, "FileTotal",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

//	json_object_object_get_ex( Req_Obj_Data, "DetectFileCnt",     &Val_Obj );
	json_object_object_get_ex( Req_Obj_Data, "FileDetected",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", json_object_get_string(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "Jumin",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));

	json_object_object_get_ex( Req_Obj_Data, "Foreigner",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "PassPort",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Driver",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Card",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Biz",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Tel",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Cell",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Bank",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Email",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Corp",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Custom1",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Custom2",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d, ", json_object_get_int(Val_Obj));
	
	json_object_object_get_ex( Req_Obj_Data, "Custom3",     &Val_Obj );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%d ) ", json_object_get_int(Val_Obj));
	
	/* Event Log Insert */
	B_Log( DBG, "Sql:[%d][%s]\n", Sql_Len, Sql_Buf );

    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }

	return( TRUE );

}/* End of Log_JFSCUSCK_Insert() */


/*---------------------------------------------------------------------------*/
/* JFSCPSCK Log Table Insert                                                 */
/*---------------------------------------------------------------------------*/
int Log_JFSCPSCK_Insert( MYSQL *DBConn, char *iData )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse( iData );
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
            json_object_to_json_string_ext( Req_Obj, 
                                            JSON_C_TO_STRING_SPACED | 
                                            JSON_C_TO_STRING_PRETTY ) );

#if 0
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }
#endif

	return( TRUE );

}/* End of Log_JFSCPSCK_Insert() */


/*---------------------------------------------------------------------------*/
/* JFSCUWCK Log Table Insert                                                 */
/*---------------------------------------------------------------------------*/
int Log_JFSCUWCK_Insert( MYSQL *DBConn, char *iData )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse( iData );
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
            json_object_to_json_string_ext( Req_Obj, 
                                            JSON_C_TO_STRING_SPACED | 
                                            JSON_C_TO_STRING_PRETTY ) );

#if 0
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }
#endif

	return( TRUE );

}/* End of Log_JFSCUWCK_Insert() */


/*---------------------------------------------------------------------------*/
/* JSFCPWCK Log Table Insert                                                 */
/*---------------------------------------------------------------------------*/
int Log_JFSCPWCK_Insert( MYSQL *DBConn, char *iData )
{
	char		Sql_Buf[1024*10];
	int			Sql_Len;
	
	// json request message parsing
	json_object *Req_Obj, *Req_Obj_Data, *Val_Obj;
	int32_t		Val_Int;
	const char *Val_Str;

    Req_Obj = json_tokener_parse( iData );
	B_Log( DBG, "Req_Obj from str:\n---\n%s\n---\n", 
            json_object_to_json_string_ext( Req_Obj, 
                                            JSON_C_TO_STRING_SPACED | 
                                            JSON_C_TO_STRING_PRETTY ) );

#if 0
    if( mysql_query(DBConn, Sql_Buf) != 0 )
    {
		B_Log( ERR, "Mysql query error : %s \n", mysql_error(DBConn) );
        return( FALSE );
    }
#endif

	return( TRUE );

}/* End of Log_JFSCPWCK_Insert() */


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
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%s, ", "now(3)" );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "'%s', ", getlogin() );
	Sql_Len += sprintf( Sql_Buf+Sql_Len, "%s )", "now(3)" );

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

