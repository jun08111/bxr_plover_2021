/*---------------------------------------------------------------------------*/
/* Name : bxrc.h                                                             */
/* Info : Blue X-rayG Server Header file                                     */
/* Version                                                                   */
/*  0.5 - New Create                                                         */
/*---------------------------------------------------------------------------*/
#ifndef BLUEXRAY_G_SERVER_H
#define BLUEXRAY_G_SERVER_H

#include <mysql.h>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <json-c/json.h>
#include <signal.h>

#define	BXR_START	1
#define	BXR_END		2

int		SEND_SOCK_BUF_SZ = 1024*10;

char *	DBMS_USER="bxrgsvr";
char *	DBMS_PASS="Whdms9500!";

enum	TR {

    JFSRINIT = 1,
	JFSCUSCK,
	JFSCPSCK,
	JFSCUWCK,
	JFSCPWCK

}TR_CODE_DEF;

#endif

