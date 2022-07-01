#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <pthread.h>
#include <regex.h>
#include <sqlite3.h>
#include <gdk/gdkscreen.h>
#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "iniparser.h"
#include "dictionary.h"
#include "b64.h"


/*- Log define ------------*/
char	*LogName;
#define LOGFILE LogName
#define ERR LOGFILE,1,__LINE__
#define INF LOGFILE,2,__LINE__
#define WRN LOGFILE,3,__LINE__
#define DBG LOGFILE,4,__LINE__

#define DEF "plover_client.log",1,__LINE__

#define BOOL int
#define TRUE 1
#define FALSE 0

#define	MAX_FCNT		10000
#define LOG_MAX_CNT  10000	 //  LogViewer의 최대 행 개수
#define V_ITEM_CNT	10000	// 취약점검사 항목 개수

#pragma pack(push, 1)
typedef struct _Cdata_Storage
{
	char uuid[37];		   	 // UUID
	char uname[20];	 	  // USER NAME
	char urank[20];		    // USER RANK
	char sip[16];	 		 // SERVER IP
	char sport[5];			// SERVER PORT
	char wport[5];		  // WEB PORT
	
}Cdata_Storage; // Conf
Cdata_Storage cDs;


typedef struct _Udata_Storage
{
	char uuid[37];		   // UUID
	char version[12];	 // Version
	char name[20];		// 사용자 이름
	char id[20];		    // 사용자 ID
	char pwd[10];		 // 사용자 PWD
	char rank[20];		  // 사용자 직급
	char dept[30];		  // 사용자 부서
	
}Udata_Storage;  //User
Udata_Storage uDs;


typedef struct _Fdata_Storage
{
    char fname[255];			// 파일 이름
	unsigned int jcnt;			  // 주민번호 개수
	unsigned int dcnt;			 // 운전면허 개수
	unsigned int fgcnt;			// 외국인등록번호 개수
	unsigned int pcnt;			// 여권번호 개수
	unsigned int fsize;			 // 파일 크기
	char fstat[20];				   // 파일 상태
	char fpath[4096];			// 파일 경로
	char ftype[255];			// 파일 포맷
	unsigned int round;			 // 회차
	char start[24];				   // 작업 시작
	char end[24];				 // 작업 종료

}Fdata_Storage;  //File
Fdata_Storage fDs[MAX_FCNT];		// 파일기준의 data구조체


typedef struct _SFdata_Storage
{
    char fname[255];			// 파일 이름
	unsigned int jcnt;			  // 주민번호 개수
	unsigned int dcnt;			 // 운전면허 개수
	unsigned int fgcnt;			// 외국인등록번호 개수
	unsigned int pcnt;			// 여권번호 개수
	unsigned int fsize;			 // 파일 크기
	char fstat[20];				   // 파일 상태
	char fpath[4096];			// 파일 경로 
	char ftype[255];			// 파일 포맷
	char start[24];				   // 작업 시작
	char end[24];				 // 작업 종료
	char ffpath[4096];			// 파일 경로 + 이름

}SFdata_Storage;
SFdata_Storage sfDs;		// 선택파일 data구조체


typedef struct _DFdata_Storage
{
	char fname[255];			// 파일 이름
	unsigned int fsize;			// 파일 크기
	char fstat[20];			      // 파일 상태
	char fpath[4096];		   // 파일 경로
	char start[24];				   // 작업 시작
	char end[24];				 // 작업 종료/
	char ffpath[4096];		   // 파일 경로

}DFdata_Storage; // Decrypt File
DFdata_Storage dfDs[MAX_FCNT]; // 암호파일기준의 data구조체


typedef struct _SDFdata_Storage
{
	char fname[255];			// 파일 이름
	unsigned int fsize;			// 파일 크기
	char fstat[20];			      // 파일 상태
	char fpath[4096];		   // 파일 경로
	char start[24];				   // 작업 시작
	char end[24];				 // 작업 종료
	char ffpath[4096];		   // 파일 경로 + 이름

}SDFdata_Storage;
SDFdata_Storage sdfDs;		// 선택암호파일 data구조체


typedef struct _VIdata_Storage
{
	char select[1];			// 취약점 항목 선택 여부 Y, N (Y: 사용, N: 미사용)
	char item[5];
	char result[32];		// 취약점 검사 결과: 성공, 실패, 미수행
	char stat[32];			// 취약점 검사 상태 S: 안전, 위험, 미수행
	int round;				// 회차
	char start[24];		  // 작업 시작
	char end[24];		 // 작업 종료

}VIdata_Storage;
VIdata_Storage viDs[V_ITEM_CNT]; // 취약점검사 상태, 결과 data구조체


typedef struct _Vdata_Storage
{
	char start[24];		  // 작업 시작
	char end[24];		 // 작업 종료
	int item;				  // 선택된 취약점 항목 총 개수
	int safe;				  // 총 안전 개수
	int vurnelability;	  // 총 취약 개수
	int imposiible;		  // 총 미수행  개수 

}Vdata_Storage;
Vdata_Storage vDs;		// 취약점검사 상태, 결과 총합 data구조체 


typedef struct _psregex_Pdata
{
	int name; // 민감정보 종류
	char*use[1]; // 민감정보 사용 여부 (Y: 사용, N: 미사용)
	int count; // N개 이상일 경우 검출
	char regex[1024]; // 민감정보 정규표현식

}psregex_Pdata; // Sensitive Regex Policy Data


typedef struct _psfile_Pdata
{
	int name; // 민감정보 검사 파일 종류
	char *use[1]; // 민감정보 검사 파일 사용 여부 (Y: 사용, N: 미사용)

}psfile_Pdata; // Sensitive File Policy Data


typedef struct _pwitem_Pdata
{
	int number; // 취약점 점검 항목
	char *use[1]; // 취약점 점검 항목 사용 여부 (Y: 사용, N: 미사용)

}pwitem_Pdata;  // Warning Item Policy Data


typedef struct _Pdata_Storage
{
	char *pstype[1];									 // 정책 종류 (개인, PC, 부서)
	char *pscheck[1];									// 민감정보 정기검사 사용여부 (Y: 사용, N: 미사용)
	int psround;											 // 민감정보 회차
	char psdate[24];	  								// 민감정보 날짜
	struct _psregex_Pdata srPd[12];				// 민감정보 정보 배열
	struct _psfile_Pdata sfPd[30];					// 민감정보  파일 정보 배열
	char detectpath[4096]; 							// 민감정보 검출 기본 경로
	char movepath[4096];							 // 민감정보 검출된 파일 이동 기본 경로
	char encpath[4096]; 								// 민감정보 검출된 암호화 파일 기본 경로
	char decpath[4096];									 // 민감정보 검출된 복호화 파일 기본 경로
	int maxfsize;			  									// 민감정보 검사 파일 최대 크기
	char *maxfable[1];										// 민감정보 검사 파일 최대 크기 허용 여부(Y: 사용, N: 미사용)
	char *forceenc[1];										// 정기검사 후 검출된 파일 암호화 사용 여부 (Y: 사용, N: 미사용)
	char *delencfile[1];										 // 복호화 시 암호화파일 자동삭제 사용 여부 (Y: 사용, N: 미사용)
	char *pwcheck[1];									// 취약점 정기검사 사용여부 (Y: 사용, N: 미사용)
	int pwround;			 								// 취약점 회차
	char pwdate[24];	 									// 취약점 날짜
	struct _pwitem_Pdata wiPd[11];				// 취약점 정보 배열
	char *detectpriority[1];			 					// 정기검사 우선순위 (S: 민감정보, W: 취약점)
	char *realtime[1];			 							// 실시간 검사 사용 여부 (Y: 사용, N: 미사용)
	char *popupmode[1];									// 팝업모드 (A: 검사 화면/결과 전부, R: 결과 만, E: 끝 확인 만)
	char *shutdown[1];										// 정기검사 후 시스템 자동종료 사용여부 (Y: 사용, N: 미사용)
	int retry;														// 오프라인 모드 시 재접속 시도 횟수
	char updatefolder[4096];						// 자동업데이트 폴더 경로
	char *autoupdate[1];									// 자동업데이트 사용 여부 사용여부 (Y: 사용, N: 미사용)
	char gladepath[4096];								// Client UI 파일 경로

}Pdata_Storage;
Pdata_Storage pDs; // Policy data구조체 


typedef struct _dPdata_Storage
{
	char *pscheck[1];									// 민감정보 정기검사 사용여부 (Y: 사용, N: 미사용)
	struct _psregex_Pdata srPd[11];				// 민감정보 정보 배열
	struct _psfile_Pdata sfPd[30];					// 민감정보  파일 정보 배열
	char detectpath[4096]; 							// 민감정보 검출 기본 경로
	char movepath[4096];							 // 민감정보 검출된 파일 이동 기본 경로
	char encpath[4096]; 								// 민감정보 검출된 암호화 파일 기본 경로
	char decpath[4096];									 // 민감정보 검출된 복호화 파일 기본 경로
	int maxfsize;			  									// 민감정보 검사 파일 최대 크기
	char *maxfable[1];										// 민감정보 검사 파일 최대 크기 허용 여부(Y: 사용, N: 미사용)
	char *forceenc[1];										// 정기검사 후 검출된 파일 암호화 사용 여부 (Y: 사용, N: 미사용)
	char *delencfile[1];										 // 복호화 시 암호화파일 자동삭제 사용 여부 (Y: 사용, N: 미사용)
	char *pwcheck[1];									// 취약점 정기검사 사용여부 (Y: 사용, N: 미사용)
	struct _pwitem_Pdata wiPd[10];				// 취약점 정보 배열
	char *realtime[1];			 							// 실시간 검사 사용 여부 (Y: 사용, N: 미사용)
	char *popupmode[1];									// 팝업모드 (A: 검사 화면/결과 전부, R: 결과 만, E: 끝 확인 만)
	char *shutdown[1];										// 정기검사 후 시스템 자동종료 사용여부 (Y: 사용, N: 미사용)
	int retry;														// 오프라인 모드 시 재접속 시도 횟수
	char updatefolder[4096];						// 자동업데이트 폴더 경로
	char *autoupdate[1];									// 자동업데이트 사용 여부 사용여부 (Y: 사용, N: 미사용)
	char gladepath[4096];								// Client UI 파일 경로

}dPdata_Storage;
dPdata_Storage dpDs; // Default Policy data구조체 


#pragma pack(pop)


int  BXLog (const char *, int , int , const char *, ...);

gboolean	d_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath     		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);
										
static GtkTreeModel	*d_create_and_fill_model (void);
static GtkWidget	*d_create_view_and_model (void);

static GtkTreeModel	*v_create_and_fill_model (void);
static GtkWidget	*v_create_view_and_model (void);

gboolean	dec_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath     		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);
										
static GtkTreeModel	*dec_create_and_fill_model (void);
static GtkWidget	*dec_create_view_and_model (void);

gboolean	dl_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath     		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);
										
static GtkTreeModel	*dl_create_and_fill_model (void);
static GtkWidget	*dl_create_view_and_model (void);

gboolean	vl_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath     		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);
										
static GtkTreeModel	*vl_create_and_fill_model (void);
static GtkWidget	*vl_create_view_and_model (void);

void *func_heartbeat_thread (void);
void *func_sensitive_thread (void);
void *func_warning_thread (void);
char *func_SendPSFile(void);
char *func_SendPWFile(void);

