/******************************************************************************/
/*  Header      : bxrerrno.h                                                  */
/*  Description : Common Error Code Header                                    */
/*  Rev. History: Ver   Date    Description                                   */
/*                ----  ------- ----------------------------------------------*/
/*                0.5   2020-06 Initial version                               */
/******************************************************************************/
#ifndef BLUEXRAY_H
#define BLUEXRAY_H

/*****************************************************************************/
/* Include Headers                                                           */
/*****************************************************************************/
#define TRUE    0
#define FALSE   -1

/*---------------------------------------------------------------------------*/
/* Error Code Define                                                         */
/*---------------------------------------------------------------------------*/
#define BXR_RESULT_OK					TRUE    /* This return success      */
#define BXR_RESULT_ERROR				FALSE   /* This return success      */

#define BXR_INVALID_ERROR				0		/* Unknown Error code       */
#define BXR_APPUPDATE_ERROR				1		/* Unknown Error code       */
#define BXR_USERNOTFOUND_ERROR			2		/* Unknown Error code       */
#define BXR_MSGFORMAT_ERROR				3		/* Unknown Error code       */
#define BXR_DBMS_ERROR					4		/* Unknown Error code       */
#define BXR_MULTIUSER_ERROR				5		/* Unknown Error code       */

#endif

