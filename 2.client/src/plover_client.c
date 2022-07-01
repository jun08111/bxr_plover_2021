/******************************************************************************/
/*  Source        : plover_client.c                                          																			*/
/*  Description :                                                           																				  */
/*  Rev. History: Ver      Date    	 Description                              							    									 */
/*  --------------   -----  -----------   -----------------------------------------------------------------------------------------*/
/*                		  1.2   2021-07  Initial version                              															      */
/******************************************************************************/
#include "plover_common.h"
#include "plover_client.h"
#include "plover_msgfmt.h"
#include "plover_errmsg.h"

#define VERSION "1.0.1123.0"
#define CONF "/.bxrG/plover.conf"
#define POLICY "/.bxrG/plover_policy.json"
#define LASTPOLICY "/.bxrG/plover_last_policy.json"
#define DEFAULTPOLICY "/.bxrG/plover_default_policy.json"
#define DBFILE "/.bxrG/plover.data"		// "DB File 위치"
#define SETTING "/.bxrG"

#define PLAN_SENSITIVE "/.bxrG/plover_last_psfile.json"
#define PLAN_WARNING "/.bxrG/plover_last_pwfile.json"

#define CHKRQRP "CS"



#define	PROGRESS_SIZE	102400		//100k
#define	MAX_ERROR_MSG	0x1000
#define	ERASER_SIZE		512
#define	ERASER_ENC_SIZE	896
#define MAX_PATH 4096 // 1024 * 4
#define MAX_NAME 255
#define MAX_NNG_MSG_SIZE 15260 // 1024 * 15 15260

clock_t start, end;

//pthread_mutex_t mutex;
nng_socket sock;
int        		   rv;
size_t     		 sz;

gdouble     chk_afcnt;						// 검사할 파일 총 개수
gdouble     chk_ingfcnt;					// 검사중인 파일 총 개수
int     		 retry_cnt;						// 재접속 시도 횟수
int				 chk_ecode;					  // NNG Check eCode
int				 chk_func;						// NNG Check func
int				 chk_workround;			   // 민감정보 정기검사 회차
int				 chk_vworkroud;			  // 취약점 정기검사 회차
int         	 c_chk_label;		 		  // calendar label 확인 flag (0: 민감정보 정기검사, 1: 취약점 정기검사, 2: 민감정보 로그조회1, 3: 민감정보 로그조회2, 4: 취약점 로그조회1, 5: 취약점 로그조회2)
int 			 chk_window;			   // Setting Window Open  위치 flag (0; main, 1: detect, 2: vdetect, 3: detectlog, 4: vdetectlog, 5: decrypt)
int  			 chk_calwindow;			 // Calendar Open  위치 flag (0: detectlog, 1: vdetectlog, 2: setting)
int 			 chk_dlwindow;			 // DetectLog Window Open 위치 flag (0: detect, 1: decrypt)
int	 			 chk_dcnt;				 	  // 검사 파일 총 개수 1부터 1개
int	 			 chk_fcnt = -1;				 // 검출 파일 총 개수 0부터 1개
int	 			 chk_dfcnt = -1;			// 암호파일 총 개수 0부터 1개
int 	 		 chk_jumin;				 	 // 주민등록번호 패턴 총 개수 
int 	 		 chk_driver;				  // 운전면허번호 패턴 총 개수 
int 	 		 chk_foreigner;				 // 외국인등록번호 패턴 총 개수 
int 	 		 chk_passport;				 // 여권번호 패턴 총 개수 
int 		 	 chk_loop;					   // Detect File Loop Check
int				 chk_detect;				  // 민감정보 0: 수동 검사, 1: 정기 검사
int				 chk_vdetect;				  // 취약점 0: 수동 검사, 1: 정기 검사
int				 chk_fmanage;			  // 0: 파일 이동, 1: 파일 암호화, 2: 파일 복호화, 3: 지난 정책
int				 chk_dldbcol;
int				 chk_vldbcol;
int				 chk_dbtable;			// DB table name
char		   chk_etype[1];			 //  NNG Check eType
char	 	   message[1024];			// Progress Bar Message
char 		 *policy_path[MAX_PATH];		  // 정책파일 경로
char	 	   chk_fname[MAX_NAME];   		// 정규식돌고있는 파일이름
char	  	   chk_ftype[255];			// 파일 포맷
char	 	   set_uuid[40];				// UUID 저장
char    	 *home_path;           		 //사용자 home path
char		 *chk_worktime[24];		// 작업 시간
char    	 *c_date[13];					 // calendar 선택 날짜
char		 *set_nng_server[1024];
char	   	 *chk_dbtnstat[1];				//  Detect Button Status (Y: 활성, N: 비활성)
char	   	 *chk_vdbtnstat[1];				//  Vulneralbility Detect Button Status (Y: 활성, N: 비활성)
char	   	 *chk_psstat[1];				  //  Check Plan Sensitive Stat 민감정보 정기검사 상태
char	   	 *chk_pwstat[1];				//  Check Plan Warning Stat 취약점 정기검사 상태
char		 *chk_online[1];				 //  Check On line/Off line Mode (Y: 온라인 모드, N: 오프라인 모드)
char		 *chk_fheart[1];				 //  Check First HeartBeat (Y: 정상, N: 변경 사항 생김)
char		 *chk_fnonoff[1];				//  Check First N ON/OFFLINE (Y: 첫번째 N 활성, N: 첫번째 N 비활성)

gdouble    percent;						     // Progress Bar
gchar		*path;					 			 // 검사 파일경로
gchar		*fname;
gpointer  	error;
guint 		*c_year, *c_month, *c_day;


/*---------------------------------------------------------------------------*/
/* Gtk/Glade  variable									                               */
/*---------------------------------------------------------------------------*/
GtkWidget				   *main_window,
									*enrollment_window,
									*detect_window,
									*detectlog_window,
									*vdetectlog_window,
									*vdetect_window,
									*decrypt_window,
									*setting_window,
									*plan_dialog,
									*window;

static GtkWidget		*start_window;
static GtkDrawingArea	*st_drawing_area;

WebKitWebView 	  	 *e_webview;
WebKitSettings			 *e_websetting;
WebKitWebContext    *e_context;
WebKitLoadEvent 		load_event;

GtkScrolledWindow	*d_scrolledwindow,
									*dl_scrolledwindow,
									*v_scrolledwindow,
									*vl_scrolledwindow,
									*dec_scrolledwindow;
						
GtkEntry		 			  *d_detect_entry,
									*dec_detect_entry,
									*s_detect_entry,
									*s_move_entry,
									*s_encrypt_entry,
									*s_decrypt_entry,
									*s_ip_entry,
									*s_port_entry;

GtkWidget 					*st_spinner,
				 				   	*m_userinfo_label,
									*m_version_label,
									*m_combobox,
									*dec_progressbar,
									*dec_progressbar_status,
									*dec_view,
									*d_progressbar,
									*d_view,
									*filechooserdialog,
									*s_pagelink_label,
									*s_pagelink_btn,
									*dl_view,
									*dl_date1_label,
									*dl_date2_label,
									*v_status_label,
									*v_item_label,
									*v_safe_label,
									*v_vurnelability_label,
									*v_imposiible_label,
									*v_date_label,
									*v_view,
									*v_progressbar,
									*vl_view,
									*vl_date1_label,
									*vl_date2_label,
									*st_label,
									*st_version_label,
									*plan_dialog_label,
									*calendar_dialog;

GtkCalendar 			   *c_calendar;

GtkTreeStore				*dtreestore,
									 *vtreestore,
									 *vltreestore,
									 *dltreestore,
									 *dectreestore;

GtkBuilder					  *builder;
static GdkPixbufAnimation	 	*pixbuf_animation;
static GdkPixbufAnimationIter *aniter;
/* end of  Gtk/Glade  variable*/

static gboolean timer (gpointer user_data);
static gboolean on_st_drawing_area_draw (GtkWidget *widget,  cairo_t *cr, gpointer userdata);


/***************** Start of NNG *******************/
/*---------------------------------------------------------------------------*/
/* NNG Client Fatial	 							                                    */
/*---------------------------------------------------------------------------*/
void fatal(const char *func, int rv)
{
        fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
        exit(1);
}
/* end of fatal(); function */

/*---------------------------------------------------------------------------*/
/* NNG Client Socket	 							                                  */
/*---------------------------------------------------------------------------*/
int func_nng_socket (const char *url)
{
	struct  timeval t;
	int stime = 0, etime = 0;
	int threaderr;
	pthread_t hb, ps, pw;
	pthread_attr_t hbattr, psattr, pwattr;
	int        rv;
	size_t     sz;
	char *msg[MAX_ERROR_MSG] = {0,};

	if ((rv = nng_req0_open (&sock)) != 0)
	{
		BXLog (ERR, "%-30s	\n", "BXR_SOCKET_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_SOCKET_ERR);
		func_gtk_dialog_modal (2, window, msg);
		fatal ("nng_socket", rv);
		nng_close (sock);
	}

	if ((rv = nng_dial (sock, url, NULL, 0)) != 0)
	{
		if (retry_cnt < 3)
		{
			gtk_widget_show (start_window);
			
			gettimeofday (&t, NULL);
			localtime (&t.tv_sec);
			stime = t.tv_sec;

			sprintf (msg, "    서버가 불안정한 상태입니다.    \n    다시 연결을 시도합니다. (%d/3)    \n", retry_cnt + 1);

			while (1) // 2초 후 넘어감
			{
				gettimeofday (&t, NULL);
				localtime (&t.tv_sec);
				etime = t.tv_sec;
				
				if ((etime - stime) >= 2)
				{
					break;
				}
				
				while (gtk_events_pending()) 
					gtk_main_iteration (); 
			}

			BXLog (ERR, "%-30s	[Code: %03d][%d/3]\n", "BXR_DIAL_ERR", BXR_DIAL_ERR, retry_cnt + 1);
			gtk_label_set_text (GTK_LABEL (st_label), msg);

			while (gtk_events_pending()) 
					gtk_main_iteration (); 

			retry_cnt++;
			func_nng_socket (set_nng_server);
		}
		else
		{
			memset (chk_online, 0x00, strlen (chk_online));
			strcpy (chk_online, "N"); //오프라인 모드로 설정
			strcpy (chk_fnonoff, "Y");

			func_UsrChk();

			sprintf (msg, "    서버가 불안정하여 연결에 실패했습니다.    \n    오프라인 모드로 실행합니다.");
			BXLog (ERR, "%-30s	[Code: %03d][%d/3]\n", "BXR_DIAL_ERR", BXR_DIAL_ERR, retry_cnt);
			func_gtk_dialog_modal (2, window, msg);
			
			gtk_widget_destroy (start_window);
			gtk_widget_show (main_window);

			//pthread_mutex_init (&mutex, NULL);

			pthread_attr_init (&hbattr);
			pthread_attr_setdetachstate (&hbattr, PTHREAD_CREATE_DETACHED);

			if (threaderr = pthread_create(&hb, &hbattr, func_heartbeat_thread, NULL))
			{
				BXLog (DBG, "%-30s	[%s]\n", "BXR_HEART_THREAD_ERROR", strerror (threaderr));
			}

			pthread_attr_init (&psattr);
			pthread_attr_setdetachstate (&psattr, PTHREAD_CREATE_DETACHED);
			if (threaderr = pthread_create (&ps, &psattr, func_sensitive_thread, NULL))
			{
				BXLog (DBG, "%-30s	[%s]\n", "BXR_SENSITIVE_THREAD_ERROR", strerror (threaderr));
			}

			pthread_attr_init (&pwattr);
			pthread_attr_setdetachstate (&pwattr, PTHREAD_CREATE_DETACHED);
			if (threaderr = pthread_create (&pw, &pwattr, func_warning_thread, NULL))
			{
				BXLog (DBG, "%-30s	[%s]\n", "BXR_WARNING_THREAD_ERROR", strerror (threaderr));
			} 

			//pthread_mutex_destroy (&mutex);

			//gtk_main();
		}

		//fatal ("nng_dial", rv);
		//nng_close (sock);
	}

	return rv;
}
/* end of func_nng_socket(); function */

/*---------------------------------------------------------------------------*/
/* NNG Client Send Message	 							                       */
/*---------------------------------------------------------------------------*/
int func_nng_client (int func, int chk_fstat)
{
	char  *sbuf = NULL;
	char *rbuf = malloc (MAX_NNG_MSG_SIZE);
	uint8_t  *sbuf_fmt = NULL;
	char *msg[MAX_ERROR_MSG] = {0,};
	pthread_t hb, ps, pw;
	pthread_attr_t hbattr, psattr, pwattr;
	int threaderr = 0;
	
	// json request message object
	json_object *req_obj;
	json_object *req_rq_obj, *req_data_obj, *req_data_arrobj;
	
	// json request array object 민감정보
	json_object *req_jm_arrobj, *req_fg_arrobj, *req_pp_arrobj, *req_dr_arrobj, *req_cd_arrobj;
	json_object *req_bz_arrobj, *req_te_arrobj, *req_ce_arrobj, *req_bk_arrobj, *req_em_arrobj;
	json_object *req_cp_arrobj, *req_c1_arrobj, *req_c2_arrobj, *req_c3_arrobj;
	json_object *req_fn_arrobj, *req_fp_arrobj, *req_fs_arrobj, *req_fz_arrobj, *req_ft_arrobj;
	json_object *req_ws_arrobj, *req_we_arrobj; // 취약점이랑 같이 사용

	// json request array object 취약점
	json_object *req_wn_arrobj, *req_wr_arrobj, *req_wt_arrobj;

	// json reply message object
    json_object *rep_obj;
	json_object *rep_rp_obj, *rep_etype_obj, *rep_ecode_obj, *rep_emesg_obj, *rep_id_obj, *rep_pwd_obj, *rep_name_obj, *rep_rank_obj, *rep_dept_obj; // Data 

	// json reply message value
    const char *rep_data_val, *rep_etype_val, *rep_emesg_val, *rep_id_val, *rep_pwd_val, *rep_name_val, *rep_rank_val, *rep_dept_val; // Data string
	int32_t *rep_ecode_val;	// Data int

	chk_ecode = 0; // eCdoe 0으로 초기화
	memset (chk_etype, 0x00, strlen (chk_etype)); // eType 0으로 초기화
	memset (msg, 0x00, strlen (msg));

	req_obj = json_object_new_object();
	req_rq_obj = json_object_new_object();

	// Request
	func_GetTime();
	switch (func)
	{
		case 0: // Init 버전 확인 및 사용자 정보 받아오기
			json_object_object_add (req_obj, 		"RQ", 			req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",			  json_object_new_string ("JFSRINIT"));
			json_object_object_add (req_rq_obj, "RqRp",			 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",	  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",			  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",		 json_object_new_string (VERSION));
			
			break;

		case 1: // Heart Beat
			json_object_object_add (req_obj, 		"RQ", 			req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",			  json_object_new_string ("JFSRHERT"));
			json_object_object_add (req_rq_obj, "RqRp",			 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",	  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",			  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",		 json_object_new_string (VERSION));
			
			break;

		case 2: // 민감정보 수동 검사
			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCUSCK"));
			json_object_object_add (req_rq_obj, "RqRp",				 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",		  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",				  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",			json_object_new_string (VERSION));		
			json_object_object_add (req_rq_obj, "Jumin",		 	 json_object_new_int (chk_jumin));
			json_object_object_add (req_rq_obj, "Foreigner",	 	json_object_new_int (chk_foreigner));
			json_object_object_add (req_rq_obj, "PassPort",	 		 json_object_new_int (chk_passport));
			json_object_object_add (req_rq_obj, "Driver",	 		  json_object_new_int (chk_driver));
			json_object_object_add (req_rq_obj, "Card",	 			  json_object_new_int (1));
			json_object_object_add (req_rq_obj, "Biz",	 			 	json_object_new_int (2));
			json_object_object_add (req_rq_obj, "Tel",	 		 		 json_object_new_int (3));
			json_object_object_add (req_rq_obj, "Cell",	 			 	json_object_new_int (4));
			json_object_object_add (req_rq_obj, "Bank",	 		 	  json_object_new_int (5));
			json_object_object_add (req_rq_obj, "Email",	 		   json_object_new_int (6));
			json_object_object_add (req_rq_obj, "Corp",	 		 	   json_object_new_int (7));
			json_object_object_add (req_rq_obj, "Custom1",	 		json_object_new_int (8));
			json_object_object_add (req_rq_obj, "Custom2",	 		json_object_new_int (9));
			json_object_object_add (req_rq_obj, "Custom3",	 		json_object_new_int (10));
			json_object_object_add (req_rq_obj, "DetectFileCnt",   json_object_new_int (chk_fcnt + 1));
			json_object_object_add (req_rq_obj, "TargetFileCnt",	json_object_new_int (chk_dcnt));
			json_object_object_add (req_rq_obj, "WorkStart",		json_object_new_string (fDs[0].start));
			json_object_object_add (req_rq_obj, "WorkEnd",		   json_object_new_string (fDs[chk_fcnt].end));

			break;

		case 3: // 민감정보 정기 검사
			req_data_arrobj = json_object_new_array();
			req_data_obj = json_object_new_object();

			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCPSCK"));
			json_object_object_add (req_rq_obj, "RqRp",				 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",		  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",				  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",			json_object_new_string (VERSION));
			json_object_object_add (req_rq_obj, "WorkRound",	json_object_new_int (chk_workround));
			json_object_object_add (req_rq_obj, "Data", 				req_data_arrobj);

			for (int i = 0; i <= chk_fcnt; i++)
			{	
				req_data_obj = json_object_new_object();

				json_object_object_add (req_data_obj, "Jumin",		json_object_new_int (fDs[i].jcnt));
				json_object_object_add (req_data_obj, "Foreigner", json_object_new_int (fDs[i].fgcnt));
				json_object_object_add (req_data_obj, "PassPort",	 json_object_new_int (fDs[i].pcnt));
				json_object_object_add (req_data_obj, "Driver",		 json_object_new_int (fDs[i].dcnt));
				json_object_object_add (req_data_obj, "Card",		json_object_new_int (1));
				json_object_object_add (req_data_obj, "Biz",		json_object_new_int (2));
				json_object_object_add (req_data_obj, "Tel",		json_object_new_int (3));
				json_object_object_add (req_data_obj, "Cell",		json_object_new_int (4));
				json_object_object_add (req_data_obj, "Bank",		json_object_new_int (5));
				json_object_object_add (req_data_obj, "Email",		json_object_new_int (6));
				json_object_object_add (req_data_obj, "Corp",		json_object_new_int (7));
				json_object_object_add (req_data_obj, "Custom1",	json_object_new_int (8));
				json_object_object_add (req_data_obj, "Custom2",	json_object_new_int (9));
				json_object_object_add (req_data_obj, "Custom3",	json_object_new_int (10));
				json_object_object_add (req_data_obj, "FileName",	 json_object_new_string (fDs[i].fname));
				json_object_object_add (req_data_obj, "FilePath",	 json_object_new_string (fDs[i].fpath));
				json_object_object_add (req_data_obj, "FileStat",	 json_object_new_string (fDs[i].fstat));
				json_object_object_add (req_data_obj, "FileSize",	 json_object_new_int (fDs[i].fsize));
				json_object_object_add (req_data_obj, "FileType",	 json_object_new_string (fDs[i].ftype));
				json_object_object_add (req_data_obj, "WorkStart", json_object_new_string (fDs[i].start));
				json_object_object_add (req_data_obj, "WorkEnd",	json_object_new_string (fDs[i].end));

				json_object_array_add (req_data_arrobj, req_data_obj);
			}

			break;
		
		case 4: // 삭제, 이동, 암호화
			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCPSCK"));
			snprintf (sbuf, 1024 * 8, sbuf_fmt, chk_worktime, uDs.uuid, VERSION, 1, fDs[chk_fstat].fname,
																																	fDs[chk_fstat].jcnt,
																																	fDs[chk_fstat].dcnt,
																																	fDs[chk_fstat].pcnt,
																																	fDs[chk_fstat].fgcnt,
																																	fDs[chk_fstat].fstat,
																																	fDs[chk_fstat].fsize,
																																	fDs[chk_fstat].ftype,
																																	fDs[chk_fstat].start,
																																	fDs[chk_fstat].end);

			break;

		case 5: // 복호화
			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCPSCK"));
			snprintf (sbuf, 1024 * 8, sbuf_fmt, chk_worktime, uDs.uuid, VERSION, 1, dfDs[chk_fstat].fname,
																																	NULL,
																																	NULL,
																																	NULL,
																																	NULL,
																																	dfDs[chk_fstat].fstat,
																																	dfDs[chk_fstat].fsize,
																																	NULL,
																																	dfDs[chk_fstat].start,
																																	dfDs[chk_fstat].end);

			break;

		case 6: // 취약점 수동 검사
			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCUWCK"));
			json_object_object_add (req_rq_obj, "RqRp",				 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",		  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",				  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",			json_object_new_string (VERSION));		
			json_object_object_add (req_rq_obj, "WorkTotal",	   json_object_new_int (vDs.item));
			json_object_object_add (req_rq_obj, "WorkSafe",		    json_object_new_int (vDs.safe));
			json_object_object_add (req_rq_obj, "WorkWarn",		  json_object_new_int (vDs.vurnelability));
			json_object_object_add (req_rq_obj, "WorkImpossible", json_object_new_int (vDs.imposiible));		
			json_object_object_add (req_rq_obj, "WorkStart",		json_object_new_string (vDs.start));
			json_object_object_add (req_rq_obj, "WorkEnd",		   json_object_new_string (vDs.end));

			break;

		case 7: // 취약점 정기 검사
			req_data_arrobj = json_object_new_array();

			json_object_object_add (req_obj, 		"RQ", 				req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",				  json_object_new_string ("JFSCPWCK"));
			json_object_object_add (req_rq_obj, "RqRp",				 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",		  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",				  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",			json_object_new_string (VERSION));		
			json_object_object_add (req_rq_obj, "WorkRound",	json_object_new_int (chk_workround));
			json_object_object_add (req_rq_obj, "Data", 				req_data_arrobj);

			for (int i = 0; i < 10; i++)
			{
				if (strncmp (&pDs.wiPd[i].use, "Y", 1) == 0)
				{
					req_data_obj = json_object_new_object();

					json_object_object_add (req_data_obj, "WorkNum",	  json_object_new_int (i + 1));					
					json_object_object_add (req_data_obj, "WorkStat",		 json_object_new_string (viDs[i].stat));
					json_object_object_add (req_data_obj, "WorkStart",		 json_object_new_string (viDs[i].start));
					json_object_object_add (req_data_obj, "WorkEnd",		json_object_new_string (viDs[i].end));

					json_object_array_add (req_data_arrobj, req_data_obj);
				}
			}

			break;

		case 8: // Policy
			json_object_object_add (req_obj, 		"RQ", 			req_rq_obj);
			json_object_object_add (req_rq_obj, "Func",			  json_object_new_string ("JFSRPLCY"));
			json_object_object_add (req_rq_obj, "RqRp",			 json_object_new_string (CHKRQRP));
			json_object_object_add (req_rq_obj, "DateTime",	  json_object_new_string (chk_worktime));
			json_object_object_add (req_rq_obj, "Uuid",			  json_object_new_string (uDs.uuid));
			json_object_object_add (req_rq_obj, "AppVer",		 json_object_new_string (VERSION));
			
			break;

		case 9: // Plan Sensitive File 
			sbuf = func_SendPSFile();
			BXLog (DBG, "%-30s	[%d][%s]\n", "BXR_REQUEST_MESSGE_CHECK", strlen (sbuf), sbuf);

			break;

		case 10: // Plan Warning File 
			sbuf = func_SendPWFile();
			BXLog (DBG, "%-30s	[%d][%s]\n", "BXR_REQUEST_MESSGE_CHECK", strlen (sbuf), sbuf);

			break;
	}

	if ((func != 9) && (func != 10))
	{
		sbuf = json_object_to_json_string_ext (req_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
		BXLog (DBG, "%-30s	[%d][%s]\n", "BXR_REQUEST_MESSGE_CHECK", strlen (sbuf), sbuf);
	}
	
	//printf (" 1 @ \n");

	/* if ((strncmp (chk_online, "N", 1) == 0))
	{
		if ((func == 3) || (func == 7))
		{
			func_CreateLastPlan (req_obj , func);
		}
		nng_free(rbuf, sz);

		return 0;
	} */

	//printf (" [%s] \n", chk_online);
	if ((rv = nng_send (sock, sbuf, strlen (sbuf) + 1, NNG_FLAG_NONBLOCK)) != 0)
	{
		//printf (" 2 @ \n");
		if ((strncmp (chk_online, "N", 1) == 0))
		{
			if ((func == 3) || (func == 7))
			{
				func_CreateLastPlan (req_obj , func);
			}
			nng_free(rbuf, sz);

			return 0;
		}
		//printf (" 3 @ \n");
		memset (chk_online, 0x00, strlen (chk_online));
		//printf (" 4 @ \n");
		strcpy (chk_online, "N");
		//printf (" 5 @ \n");
		BXLog (ERR, "%-30s	\n", "BXR_REQUEST_ERR");
		//printf (" 6 @ \n");
		//printf (" 7 @ \n");
		//printf (" 8 @ \n");

		//fatal ("nng_send", rv);
		nng_free(rbuf, sz);
		//printf (" 9 @ \n");

		return 0;
	}
	//printf (" 10 @ \n");
	memset (chk_online, 0x00, strlen (chk_online));
	strcpy (chk_online, "Y");

	if ((rv = nng_recv (sock, &rbuf, &sz, NNG_FLAG_ALLOC)) != 0)
	{
		memset (chk_online, 0x00, strlen (chk_online));
		strcpy (chk_online, "N");
		BXLog (ERR, "%-30s	\n", "BXR_REPLY_ERR");
		sprintf (msg, "    네트워크가 불안정하여 서버와 연결이 끊어졌습니다.    \n    오프라인 모드로 전환합니다.[Code: %03d]    \n", BXR_REPLY_ERR);
		func_gtk_dialog_modal (2, window, msg);
		//fatal ("nng_send", rv);
		nng_free(rbuf, sz);

		return 0;
	}

	memset (chk_online, 0x00, strlen (chk_online));
	strcpy (chk_online, "Y");

	if (sz > 0 )
	{
		rep_obj = json_tokener_parse (rbuf);

		BXLog (DBG, "%-30s	[%s]\nrv:[%d]sz:[%d]\n", "BXR_REPLY_MESSGE_CHECK", json_object_to_json_string_ext (rep_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY), rv, sz);

		json_object_object_get_ex (rep_obj, 	  "RP", 			 &rep_rp_obj);
		json_object_object_get_ex (rep_rp_obj, "eType",    		&rep_etype_obj);
		json_object_object_get_ex (rep_rp_obj, "eCode",    		&rep_ecode_obj);
		json_object_object_get_ex (rep_rp_obj, "eMesg",    		&rep_emesg_obj);

		rep_etype_val 	= json_object_get_string (rep_etype_obj);
		rep_ecode_val 	= json_object_get_int (rep_ecode_obj);
		rep_emesg_val 	= json_object_get_int (rep_emesg_obj);
		
		chk_ecode = rep_ecode_val;
		strncpy (chk_etype, rep_etype_val, 1);
		chk_func = func;

		if ((strncmp (chk_etype, "S", 1) == 0))
		{
			switch (chk_func)
			{
				case 0: // INIT
					switch (chk_ecode)
					{
						case 0: // INI SUCCESS
							BXLog (DBG, "INIT SUCCESS\n");
							json_object_object_get_ex (rep_rp_obj, "uId",			 &rep_id_obj);
							json_object_object_get_ex (rep_rp_obj, "uPwd",		 &rep_pwd_obj);
							json_object_object_get_ex (rep_rp_obj, "uName",  	 &rep_name_obj);
							json_object_object_get_ex (rep_rp_obj, "uRankNm",  &rep_rank_obj);
							json_object_object_get_ex (rep_rp_obj, "uDeptNm",  &rep_dept_obj);

							rep_id_val 		 		= json_object_get_string (rep_id_obj);
							rep_pwd_val   		= json_object_get_string (rep_pwd_obj);
							rep_name_val	   = json_object_get_string (rep_name_obj);
							rep_rank_val   		 = json_object_get_string (rep_rank_obj);
							rep_dept_val   		 = json_object_get_string (rep_dept_obj);

							sprintf (uDs.version, VERSION);
							sprintf (uDs.id, "%s", rep_id_val =! NULL ? rep_id_val : " ");
							sprintf (uDs.pwd, "%s", rep_pwd_val =! NULL ? rep_pwd_val : " ");
							sprintf (uDs.name, "%s", rep_name_val =! NULL ? rep_name_val : " ");
							sprintf (uDs.rank, "%s", rep_rank_val =! NULL ? rep_rank_val : " ");
							sprintf (uDs.dept, "%s", rep_dept_val =! NULL ? rep_dept_val : " ");

							func_UsrChk();

							BXLog (DBG, "%-30s	[%s] [%s]\n", "GOOD DAY", cDs.uname, cDs.urank);

							gtk_widget_destroy (start_window);
							gtk_widget_show (main_window);			// 메인 창

							// (&mutex, NULL);

							pthread_attr_init (&hbattr);
							pthread_attr_setdetachstate (&hbattr, PTHREAD_CREATE_DETACHED);

							if (threaderr = pthread_create(&hb, &hbattr, func_heartbeat_thread, NULL))
							{
								BXLog (DBG, "%-30s	[%s]\n", "BXRC_HEART_THREAD_ERROR", strerror (threaderr));
							}

							pthread_attr_init (&psattr);
							pthread_attr_setdetachstate (&psattr, PTHREAD_CREATE_DETACHED);
							if (threaderr = pthread_create(&ps, &psattr, func_sensitive_thread, NULL))
							{
								BXLog (DBG, "%-30s	[%s]\n", "BXRC_SENSITIVE_THREAD_ERROR", strerror (threaderr));
							}

							pthread_attr_init (&pwattr);
							pthread_attr_setdetachstate (&pwattr, PTHREAD_CREATE_DETACHED);
							if (threaderr = pthread_create(&pw, &pwattr, func_warning_thread, NULL))
							{
								BXLog (DBG, "%-30s	[%s]\n", "BXR_WARNING_THREAD_ERROR", strerror (threaderr));
							} 

							//pthread_mutex_destroy (&mutex);
							
							//gtk_main();

							break;
						
						case 2: // 버전 업데이트 필요
							BXLog (DBG, "%-30s	[%s]\n", "BXR_UPDATE_VERSION", uDs.version);
							sprintf (msg, "\n    최신 버전으로 업데이트 해주세요. eCode: [%03d]\n", chk_ecode);
							func_gtk_dialog_modal (2, window, msg);

							break;

						case 3: // 유저 정보 없음
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "BXR_NONE_USER", chk_ecode);
							func_UsrAdd();		// 사용자 등록

							break;

						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "INIT_ERR", chk_ecode);

							break;
					}

					break;

				case 1: // HEARTBEAT
					switch (chk_ecode)
					{
						case 0: //  성공
							BXLog (DBG, "HEART BEAT SUCCESS [%d]\n", chk_ecode);
							if (strncmp (chk_fheart, "Y", 1) == 0)
							{
								func_ChkLastPlanFile();
								strcpy (chk_fheart, "N");
							} 
							
							break;

						case 1: // 정책 변경
							BXLog (DBG, "POLICY CHANGE [%d]\n", chk_ecode);
							func_nng_client (8, 0);

							break;

						case 2: // 

							break;

						default:
							BXLog (DBG, "HEART BEAT SUCCESS DEFAULT[%d]\n", chk_ecode);

							break;
					}

					break;

				case 2: // 민감정보 수동 검사
					BXLog (DBG, "민감정보 수동 검사 SUCCESS\n");

					break;

				case 3: // 민감정보 정기 검사
					BXLog (DBG, "민감정보 정기 검사 SUCCESS\n");
					// 정책 파일에서 PSCheck = N 으로 바꿔줘야함

					break;

				case 4: // 삭제, 이동, 암호화
					BXLog (DBG, "삭제, 이동, 암호화 SUCCESS\n");

					break;

				case 5: // 복호화
					BXLog (DBG, "복호화 SUCCESS\n");

					break;

				case 6: // 취약점 수동 검사
					BXLog (DBG, "취약점 수동 검사 SUCCESS\n");

					break;

				case 7: // 취약점 정기 검사
					BXLog (DBG, "취약점 정기 검사 SUCCESS\n");
					// 정책 파일에서 PWCheck = N 으로 바꿔줘야함

					break;

				case 8: // 정책 수신
					BXLog (DBG, "정책  SUCCESS\n");
					func_CreatePolicy (rep_obj);
					func_ParsePolicy();
					func_CreateDefaultPolicy (2);

					break;

				case 9: // Send Plan Sensitive Result Offline
					BXLog (DBG, "오프라인 모드 민감정보 정기검사 결과 전송 SUCCESS\n");
					strcpy (pDs.pscheck, "N");
					func_UpdatePolicy();

					break;
				
				case 10: // Send Plan Warning Result Offline
					BXLog (DBG, "오프라인 모드 취약점 정기검사 결과 전송 SUCCESS\n");
					strcpy (pDs.pwcheck, "N");
					func_UpdatePolicy();

					break;

				default:
					BXLog (DBG, "SUCCESS DEFAULT [%d]\n", chk_func);

					break;
			}
		}
		else // ((strncmp (chk_etype, "E", 1) == 0))
		{
			switch (chk_func)
			{
				case 0: // INIT
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "INIT_ERR", chk_ecode);
							func_UsrAdd();		// 프로토콜 정의서 적용되면 다시 바꿔야함 

							break;
					}

					break;

				case 1: // HEARTBEAT		
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "HEARTBEAT_ERR", chk_ecode);
							if (strncmp (chk_fheart, "Y", 1) == 0)
							{
								func_ChkLastPlanFile();
								strcpy (chk_fheart, "N");
							} 

							break;
					}

					break;

				case 2: // 민감정보 수동 검사
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "USERSENSITIVE_ERR", chk_ecode);

							break;
					}

					break;

				case 3: // 민감정보 정기 검사
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "PLANSENSITIVE_ERR", chk_ecode);

							break;
					}

					break;

				case 4: // 삭제, 이동, 암호화
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "DEL_MV_ENC_ERR", chk_ecode);

							break;
					}

					break;

				case 5: // 복호화
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "DECRYPT_ERR", chk_ecode);

							break;
					}

					break;

				case 6: // 취약점 수동 검사
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "USERWARNING_ERR", chk_ecode);

							break;
					}

					break;

				case 7: // 취약점 정기 검사
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "PLANWARNING_ERR", chk_ecode);

							break;
					}

					break;

				case 8: // 정책 수신
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "POLICY_ERR", chk_ecode);

							break;
					}

					break;

				case 9: // Send Plan Sensitive Result Offline
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "OFF_PLANSENSITIVE_ERR", chk_ecode);

							break;
					}

					break;

				case 10: // Send Plan Warning Result Offline
					switch (chk_ecode)
					{
						default:
							BXLog (DBG, "%-30s	eCode: [%03d]\n", "OFF_PLANWARNING_ERR", chk_ecode);

							break;
					}

					break;

				default:
					BXLog (DBG, "DEFAULT_ERROR FUNC: [%d], eCode: [%03d]\n", chk_func, chk_ecode);

					break;

			}
		}
	}
	else
	{
		BXLog (ERR, "%-30s	\n", "BXR_MESSAGE_SIZE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_MESSAGE_SIZE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		nng_close (sock);

		exit(1);
	}
	// This assumes that rbuf is ASCIIZ (zero terminated).
	nng_free(rbuf, sz);

	return (1);
}
/* end of func_nng_client(); function */
/****************** End of NNG *******************/



/*---------------------------------------------------------------------------*/
/*  HeartBeat Thread 							                                      */
/*---------------------------------------------------------------------------*/
void *func_heartbeat_thread ()
{
	struct  timeval t;
	int stime = 0, etime = 0;
	char *msg[MAX_ERROR_MSG] = {0,};

	gettimeofday (&t, NULL);
	localtime (&t.tv_sec);
	stime = t.tv_sec;

	while (1) // 1초  후 넘어감
	{
		gettimeofday (&t, NULL);
		localtime (&t.tv_sec);
		etime = t.tv_sec;
		if ((etime - stime) >= 3)
		{
			break;
		}
	}
	//pthread_mutex_lock(&mutex);
    //======== critical section =============
    while (1)
    {
		//printf (" S @ \n");

		if ((strncmp (chk_online, "N", 1) == 0))
		{
			if ((strncmp (chk_fnonoff, "N", 1) == 0))
			{
				gdk_threads_init();
				gdk_threads_enter();
				strcpy (chk_fnonoff, "Y");
				memset (msg, 0x00, strlen (msg));
				sprintf (msg, "    네트워크가 불안정하여 서버와 연결이 끊어졌습니다.    \n    오프라인 모드로 전환합니다.[Code: %03d]    \n", BXR_REQUEST_ERR);
				func_gtk_dialog_modal (2, window, msg);
				gdk_flush();
				gdk_threads_leave();
			}

			nng_dial (sock, set_nng_server, NULL, 0);
		}
		else
		{
			strcpy (chk_fnonoff, "N");
		}

		//printf (" h1 @ \n");
		/* gdk_threads_init();
		gdk_threads_enter(); */

		//printf (" h2 @ \n");

		func_nng_client (1, 0);
		//printf (" h3 @ \n");

		/* gdk_flush();
		gdk_threads_leave(); */

		//printf (" E @ \n");

		gettimeofday (&t, NULL);
		localtime (&t.tv_sec);
		stime = t.tv_sec;

		while (1) // 3초  후 넘어감
		{
			gettimeofday (&t, NULL);
			localtime (&t.tv_sec);
			etime = t.tv_sec;
			if ((etime - stime) >= 3)
			{
				break;
			}
		}
    }
    //========= critical section ============
    //pthread_mutex_unlock(&mutex);
}
/* end of func_heartbeat_thread(); */


/*---------------------------------------------------------------------------*/
/*  Plan Sensitive Thread 							                                 */
/*---------------------------------------------------------------------------*/
void *func_sensitive_thread ()
{
	struct  timeval t;
	FILE *fd;
	int plcytime = 0, restime = 0, res = 0, stime = 0, etime = 0;
	char  cmdbuf[256] = {0,};
	char  strbuf[1024] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};

	gettimeofday (&t, NULL);
	localtime (&t.tv_sec);
	stime = t.tv_sec;

	while (1) // 3초  후 넘어감
	{
		gettimeofday (&t, NULL);
		localtime (&t.tv_sec);
		etime = t.tv_sec;
		if ((etime - stime) >= 3)
		{
			break;
		}
	}

    //pthread_mutex_lock(&mutex);
    //======== critical section =============
	while (1)
    {
		if (strncmp (pDs.pscheck, "N", 1) == 0)
		{
			continue;
		}
		else
		{
			if ((strncmp (pDs.detectpriority, "S", 1) == 0) || (strncmp (chk_pwstat, "N", 1) == 0))
			{
				printf ("p[%s]  w[%s]\n", pDs.detectpriority, chk_pwstat);

				gdk_threads_init();
				gdk_threads_enter();
				gettimeofday (&t, NULL);
				localtime (&t.tv_sec);
				gdk_flush();
				gdk_threads_leave();

				sprintf (cmdbuf, "date -d \"%s\" +%%s", pDs.psdate);
			
				if ((fd = popen (cmdbuf, "r")) == NULL)
				{
					sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_SENSITIVE_THREAD_ERR);
					func_gtk_dialog_modal (2, window, msg);
					BXLog (DBG, "%-30s	\n", "BXR_SENSITIVE_THREAD_ERR");
					BXLog (DBG, "%-30s	[%s][%d]\n", "JFSCPSCK POPEN ERROR", strerror (errno), errno);
				}
				else
				{
					if (fgets (strbuf, 1024, fd) != NULL)
					{
						plcytime = atoi (strbuf);
						restime = plcytime - t.tv_sec;
						//printf ("[%d] = [%d] - [%d]\n", restime, plcytime, t.tv_sec);
					}
				}

				pclose (fd);

				if ((restime <= 0) && (strncmp (chk_psstat, "R", 1) != 0) && (strncmp (chk_dbtnstat, "Y", 1) != 0) && (strncmp (chk_dbtnstat, "Y", 1) != 0))
				{
					printf (" @[%s]  [%s]\n", pDs.pscheck, pDs.pwcheck);
					printf (" @[%s]  [%s]\n", chk_psstat, chk_dbtnstat);
					sprintf (msg, 
					"\n    3초 뒤 민감정보 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

					gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);

					gdk_threads_init();
					gdk_threads_enter();
					gettimeofday (&t, NULL);
					localtime (&t.tv_sec);
					stime = t.tv_sec;
					gdk_flush();
					gdk_threads_leave();
					
					// Thread, xlib 라이브러리 같이 사용할 때 는 gdk init/enter/flush/leave로 감싸주면 사용가능
					gdk_threads_init();
					gdk_threads_enter();
					d_Refresh_ScrollWindow();
					gdk_flush();
					gdk_threads_leave();

					gdk_threads_init();
					gdk_threads_enter();
					gtk_widget_show (detect_window);
					gtk_widget_show (plan_dialog);
					gtk_widget_hide (main_window);
					gtk_widget_hide (setting_window);
					gtk_widget_hide (decrypt_window);
					gtk_widget_hide (vdetect_window);
					gtk_widget_hide (detectlog_window);
					gtk_widget_hide (vdetectlog_window);
					gdk_flush();
					gdk_threads_leave();
					
					while (1) // 3초 후 넘어감
					{
						gdk_threads_init();
						gdk_threads_enter();
						gettimeofday (&t, NULL);
						localtime (&t.tv_sec);
						etime = t.tv_sec;
						gdk_flush();
						gdk_threads_leave();

						if ((etime - stime) <= 1)
						{
							sprintf (msg, 
							"\n    3초 뒤 민감정보 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if ((etime - stime) <= 2)
						{
							sprintf (msg, 
							"\n    2초 뒤 민감정보 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if (((etime - stime) > 2) && ((etime - stime) <= 3))
						{
							sprintf (msg, 
							"\n    1초 뒤 민감정보 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if((etime - stime) >= 4)
						{
							break;
						}
					}

					if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (pDs.pscheck, "N", 1) == 0))
					{
						continue;
					}
					else
					{
						gdk_threads_init();
						gdk_threads_enter();

						gtk_entry_set_text (GTK_ENTRY (d_detect_entry), pDs.detectpath);

						gtk_widget_hide (plan_dialog);

						chk_detect = 1; // 정기검사 

						d_detect_btn_clicked();

						gdk_flush();
						gdk_threads_leave();

						chk_detect = 0; // 초기화 (수동검사)
						strcpy (pDs.pscheck, "N");
						memset (pDs.detectpriority, 0x00, 2);
						strcpy (pDs.detectpriority, "W");
						func_UpdatePolicy();
					}
				}
			}
			else
			{
				continue;
			}
		}
	}
    //========= critical section ============
    //pthread_mutex_unlock(&mutex);

}
/* end of func_sensitive_thread(); */


/*---------------------------------------------------------------------------*/
/*  Plan Warning Thread 							                                */
/*---------------------------------------------------------------------------*/
void *func_warning_thread()
{
	struct  timeval t;
	FILE *fd;
	int plcytime = 0, restime = 0, res = 0, stime = 0, etime = 0;
	char  cmdbuf[256] = {0,};
	char  strbuf[1024] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};

	gettimeofday (&t, NULL);
	localtime (&t.tv_sec);
	stime = t.tv_sec;

	while (1) // 3초  후 넘어감
	{
		gettimeofday (&t, NULL);
		localtime (&t.tv_sec);
		etime = t.tv_sec;
		if ((etime - stime) >= 3)
		{
			break;
		}
	}
	
    //pthread_mutex_lock(&mutex);
    //======== critical section =============
	while (1)
    {
		if (strncmp (pDs.pwcheck, "N", 1) == 0)
		{
			continue;
		}
		else
		{
			if (strncmp (pDs.detectpriority, "W", 1) == 0)
			{
				gdk_threads_init();
				gdk_threads_enter();
				gettimeofday (&t, NULL);
				localtime (&t.tv_sec);
				gdk_flush();
				gdk_threads_leave();

				sprintf (cmdbuf, "date -d \"%s\" +%%s", pDs.pwdate);
			
				if ((fd = popen (cmdbuf, "r")) == NULL)
				{
					sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_WARNING_THREAD_ERR);
					func_gtk_dialog_modal (2, window, msg);
					BXLog (DBG, "%-30s	\n", "BXR_WARNING_THREAD_ERR");
					BXLog (DBG, "%-30s	[%s][%d]\n", "JFSCPWCK POPEN ERROR", strerror (errno), errno);
				}
				else
				{
					if (fgets (strbuf, 1024, fd) != NULL)
					{
						plcytime = atoi (strbuf);
						restime = plcytime - t.tv_sec;
						//printf ("[%d] = [%d] - [%d]\n", restime, plcytime, t.tv_sec);
					}
				}

				pclose (fd);

				if ((restime <= 0) && (strncmp (chk_pwstat, "R", 1) != 0) && (strncmp (chk_vdbtnstat, "Y", 1) != 0))
				{
					printf (" [%s]  [%s]\n", pDs.pscheck, pDs.pwcheck);
					printf (" [%s]  [%s]\n", chk_pwstat, chk_vdbtnstat);
					sprintf (msg, 
					"\n    3초 뒤 취약점 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

					gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);

					gdk_threads_init();
					gdk_threads_enter();
					gettimeofday (&t, NULL);
					localtime (&t.tv_sec);
					stime = t.tv_sec;
					gdk_flush();
					gdk_threads_leave();

					// Thread, xlib 라이브러리 같이 사용할 때 는 gdk init/enter/flush/leave로 감싸주면 사용가능
					gdk_threads_init();
					gdk_threads_enter();
					v_Refresh_ScrollWindow();
					gdk_flush();
					gdk_threads_leave();

					gdk_threads_init();
					gdk_threads_enter();
					gtk_widget_show (vdetect_window);
					gtk_widget_show (plan_dialog);
					gtk_widget_hide (main_window);
					gtk_widget_hide (setting_window);
					gtk_widget_hide (decrypt_window);
					gtk_widget_hide (detect_window);
					gtk_widget_hide (detectlog_window);
					gtk_widget_hide (vdetectlog_window);
					gdk_flush();
					gdk_threads_leave();

					while (1) // 3초 후 넘어감
					{
						gdk_threads_init();
						gdk_threads_enter();
						gettimeofday (&t, NULL);
						localtime (&t.tv_sec);
						etime = t.tv_sec;
						gdk_flush();
						gdk_threads_leave();

						if ((etime - stime) <= 1)
						{
							sprintf (msg, 
							"\n    3초 뒤 취약점 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if ((etime - stime) <= 2)
						{
							sprintf (msg, 
							"\n    2초 뒤 취약점 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if (((etime - stime) > 2) && ((etime - stime) <= 3))
						{
							sprintf (msg, 
							"\n    1초 뒤 취약점 정기검사를 진행합니다.\n    정기 검사 진행 중에는 개인 검사를 진행할 수 없습니다.         \n");

							gtk_label_set_text (GTK_LABEL (plan_dialog_label), msg);
						}
						else if((etime - stime) >= 4)
						{
							break;
						}
					}

					if ((strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0) || (strncmp (pDs.pwcheck, "N", 1) == 0))
					{
						continue;
					}
					else
					{
						gdk_threads_init();
						gdk_threads_enter();

						gtk_widget_hide (plan_dialog);

						strcpy (chk_pwstat, "S");
						chk_vdetect = 1; // 정기검사 

						gdk_flush();
						gdk_threads_leave();

						gdk_threads_init();
						gdk_threads_enter();

						v_detect_btn_clicked();

						gdk_flush();
						gdk_threads_leave();

						chk_vdetect = 0; // 초기화 (수동검사)
						strcpy (pDs.pwcheck, "N");
						memset (pDs.detectpriority, 0x00, 2);
						strcpy (pDs.detectpriority, "S");
						func_UpdatePolicy();
					}
				}
			}
			else
			{
				continue;
			}
		}	
	}
    //========= critical section ============
    //pthread_mutex_unlock(&mutex);
}
/* end of func_warning_thread(); */


/*---------------------------------------------------------------------------*/
/* Check Current Time 							                                     */
/*---------------------------------------------------------------------------*/
void func_GetTime()
{	
	struct  timeval t;
    struct  tm *tm;

	gettimeofday   (&t, NULL);
    tm = localtime (&t.tv_sec);

	memset (chk_worktime, 0, 24);

	sprintf (chk_worktime,  "%d-%02d-%02d %02d:%02d:%02d.%03ld", tm->tm_year + 1900,
																															tm->tm_mon + 1,
																															tm->tm_mday,
																															tm->tm_hour,
																															tm->tm_min,
																															tm->tm_sec,
																															t.tv_usec/1000
																															);

	return ;
}
/* end of func_GetTime(); */


/*---------------------------------------------------------------------------*/
/* Compare Current Date Time 							                      */
/*---------------------------------------------------------------------------*/
int func_CompareDateTime()
{	
	time_t current;
	struct tm *t;
	guint *cyear = 0, *cmon = 0, *cday = 0;
	int res = -1;

	current = time (NULL);
	t = localtime (&current);
	cyear  = t->tm_year + 1900;
	cmon = t->tm_mon;
	cday   = t->tm_mday;

	if (c_year - cyear < 0)
	{
		func_gtk_dialog_modal (0, window, "\n    날짜를 확인해주세요.    \n");
	}
	else if (c_year - cyear > 0)
	{
		res = 0;
	}
	else
	{
		if (c_month - cmon < 0)
		{
			func_gtk_dialog_modal (0, window, "\n    날짜를 확인해주세요.    \n");
		}
		else if (c_month - cmon > 0)
		{
			res = 0;
		}
		else
		{
			if (c_day - cday < 0)
			{
				func_gtk_dialog_modal (0, window, "\n    날짜를 확인해주세요.    \n");
			}
			else if (c_day - cday > 0)
			{
				res = 0;
			}
		}
	}

	return res;
}
/* end of func_CompareDateTime(); */


/*---------------------------------------------------------------------------*/
/* Get homepath 							                                           */
/*---------------------------------------------------------------------------*/
void func_GetHomePath()
{	
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));
	home_path = getenv ("HOME");

	if (home_path == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_GET_HOMEPATH_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_GET_HOMEPATH_ERR);
		func_gtk_dialog_modal (2, window, msg);

		return -1;
	}
}
/* end of func_GetHomePath(); */


/*---------------------------------------------------------------------------*/
/* Create ConfFile                                            							*/
/*---------------------------------------------------------------------------*/
int func_CreateConf (char *conf_path)
{
	FILE *conf ;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	if ((conf = fopen (conf_path, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_CONF_CREATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_CONF_CREATE_ERR);
		func_gtk_dialog_modal (2, window, msg);

		fclose (conf);
		return -1;
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_CONF_CREATE");
		fprintf (conf,
		"[USER]\n"
		"NAME			=		;\n"
		"RANK			=		;\n\n"
		"[CLIENT]\n"
		"UUID			=	%s	;\n\n"
		"[SERVER]\n"
		"IP				=	%s	;\n"
		"SERVER_PORT	=	%s	;\n"
		"WEB_PORT		=	%s	;\n\n"
		,uDs.uuid, "192.168.200.233", "9500", "8443");
		fclose (conf);
	}
	return BXR_SUCCESS;
}
/* end of func_CreateConf(); */


/*---------------------------------------------------------------------------*/
/* Update ConfFile                                            							*/
/*---------------------------------------------------------------------------*/
int func_UpdateConf (char *conf_path)
{
	FILE *conf ;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	if ((conf = fopen (conf_path, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_CONF_UPDATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_CONF_UPDATE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		
		return -1;
	}
	else
	{
		if ((strncmp (chk_online, "Y", 1) == 0))
		{
			fprintf (conf,
			"[USER]\n"
			"NAME			=	%s	;\n"
			"RANK			=	%s	;\n\n"
			"[CLIENT]\n"
			"UUID			=	%s	;\n\n"
			"[SERVER]\n"
			"IP				=	%s	;\n"
			"SERVER_PORT	=	%s	;\n"
			"WEB_PORT		=	%s	;\n"
			,uDs.name, uDs.rank, uDs.uuid, cDs.sip, cDs.sport, cDs.wport
			);
		}
		else
		{
			fprintf (conf,
			"[USER]\n"
			"NAME			=	%s	;\n"
			"RANK			=	%s	;\n\n"
			"[CLIENT]\n"
			"UUID			=	%s	;\n\n"
			"[SERVER]\n"
			"IP				=	%s	;\n"
			"SERVER_PORT	=	%s	;\n"
			"WEB_PORT		=	%s	;\n"
			,cDs.uname, cDs.urank, cDs.uuid, cDs.sip, cDs.sport, cDs.wport
			);
		}
			fclose (conf);
	}
	return BXR_SUCCESS;
}
/* end of func_UpdateConf(); */


/*---------------------------------------------------------------------------*/
/* Set ConfFile                                             								*/
/*---------------------------------------------------------------------------*/
int  func_SetConf (char * conf_name)
{
	dictionary *conf;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));
	
	/* Some temporary variables to hold query results */
	conf = iniparser_load (conf_name);
	if (conf == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_CONF_SET_ERR", conf_name);
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_CONF_SET_ERR);
		func_gtk_dialog_modal (2, window, msg);
		return -1;
	}
	iniparser_dump (conf, stderr);

	/* Set attributes */
	iniparser_set (conf, "USER:NAME", cDs.uname);
	iniparser_set (conf, "USER:RANK", cDs.urank);
	iniparser_set (conf, "CLIENT:UUID", uDs.uuid);
	iniparser_set (conf, "SERVER:IP", cDs.sip);
	iniparser_set (conf, "SERVER:SERVER_PORT", cDs.sport);
	iniparser_set (conf, "SERVER:WEB_PORT", cDs.wport);

	iniparser_freedict (conf);
	return BXR_SUCCESS ;
}
/* end of func_SetConf(); */



/*---------------------------------------------------------------------------*/
/* Parse ConfFile                                             							 */
/*---------------------------------------------------------------------------*/
int  func_ParseConf (char * conf_name)
{
	dictionary *conf;
	const char *chk_uname = NULL;
	const char *chk_urank = NULL;
	const char *chk_uuid = NULL;
	const char *chk_sip = NULL;
	const char *chk_sport = NULL;
	const char *chk_wport = NULL;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));
	
	/* Some temporary variables to hold query results */

	conf = iniparser_load (conf_name);
	if (conf == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_CONF_PARSE_ERR", conf_name);
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_CONF_PARSE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		return -1 ;
	}
	//iniparser_dump (conf, stderr);
	
	/* Get attributes */
	chk_uname = iniparser_getstring (conf, "USER:NAME", NULL);
	strcpy (cDs.uname, chk_uname);
	BXLog (DBG, "%-30s	[%s]\n", "CONF USERNAME", cDs.uname ? cDs.uname : "UNDEF");

	chk_urank = iniparser_getstring (conf, "USER:RANK", NULL);
	strcpy (cDs.urank, chk_urank);
	BXLog (DBG, "%-30s	[%s]\n", "CONF USERRANK", cDs.urank ? cDs.urank : "UNDEF");
	
	chk_uuid = iniparser_getstring (conf, "CLIENT:UUID", NULL);
	strcpy (cDs.uuid, chk_uuid);
	BXLog (DBG, "%-30s	[%s]\n", "CONF UUID", cDs.uuid ? cDs.uuid : "UNDEF");
	
	chk_sip = iniparser_getstring (conf, "SERVER:IP", NULL);
	strcpy (cDs.sip, chk_sip);
	BXLog (DBG, "%-30s	[%s]\n", "CONF SERVERIP", cDs.sip ? cDs.sip : "UNDEF");

	chk_sport = iniparser_getstring (conf, "SERVER:SERVER_PORT", NULL);
	strcpy (cDs.sport, chk_sport);
	BXLog (DBG, "%-30s	[%s]\n", "CONF SERVERPORT", cDs.sport ? cDs.sport : "UNDEF");

	chk_wport = iniparser_getstring (conf, "SERVER:WEB_PORT", NULL);
	strcpy (cDs.wport, chk_wport);
	BXLog (DBG, "%-30s	[%s]\n", "CONF WEBPORT", cDs.wport ? cDs.wport : "UNDEF");

	//i = iniparser_getint (ini, "a:b", -1);
	//printf ("a:	[%d]\n", i);
	iniparser_freedict (conf);
	return BXR_SUCCESS ;
}
/* end of func_ParseConf(); */


/*---------------------------------------------------------------------------*/
/* Create Default PolicyFile                                            			*/
/*---------------------------------------------------------------------------*/
void func_CreateDefaultPolicy (int chk_cu)
{
	FILE *policy ;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *setpath[MAX_PATH] = {0,};
	char *defaultpolicy[MAX_PATH] = {0,};
	json_object *key_obj;
	char pbuf[MAX_NNG_MSG_SIZE] = {0,};
	char *policyfmt = NULL;

	policyfmt = "{\
							\"RP\" : {\
													\"DateTime\" : \"%s\",\
													\"Uuid\" : \"%s\",\
													\"AppVer\": \"%s\",\
													\"PSCheck\" : \"N\",\
													\"PSRegex\" : [\
																					{\"Name\": 1,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-8][0-9]{6}\"\
																					},\
																					{\"Name\": 2,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}-[0-9]{6}-[0-9]{2}\"\
																					},\
																					{\"Name\": 3,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-8][0-9]{6}\"\
																					},\
																					{\"Name\": 4,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 4\"\
																					},\
																					{\"Name\": 5,\
																					\"Use\": \"%s\",\
																					\"Count\": %d,\
																					\"Regex\": \"정규표현식 5\"\
																					},\
																					{\"Name\": 6,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 6\"\
																					},\
																					{\"Name\": 7,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 7\"\
																					},\
																					{\"Name\": 8,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 8\"\
																					},\
																					{\"Name\": 9,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 9\"\
																					},\
																					{\"Name\": 10,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 10\"\
																					},\
																					{\"Name\": 11,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 11\"\
																					}\
																				],\
													\"PSFile\" : [\
																				{\"Name\": 1,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 2,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 3,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 4,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 5,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 6,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 7,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 8,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 9,\
																					\"Use\": \"%s\"\
																				}\
																			],\
													\"DetectPath\" : \"%s\",\
													\"MovePath\" : \"%s\",\
													\"EncryptPath\" : \"%s\",\
													\"DecryptPath\" : \"%s\",\
													\"MaxFileSize\" : 100,\
													\"MaxFileAble\": \"Y\",\
													\"ForceEncrypt\": \"N\",\
													\"DelEncFile\": \"N\",\
													\"PWCheck\": \"N\",\
													\"PWItem\" : [\
																				{\"Number\": 1,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 2,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 3,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 4,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 5,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 6,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 7,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 8,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 9,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 10,\
																					\"Use\": \"%s\"\
																				}\
																			],\
													\"RealTime\": \"N\",\
													\"PopupMode\": \"A\",\
													\"ShutDown\": \"N\",\
													\"ReTry\": 3,\
													\"UpdateFolder\": \"%s\",\
													\"AutoUpdate\": \"N\",\
													\"GladePath\": \"/usr/share/icons/plover/plover.glade\",\
													}\
                                }";

	func_GetTime();

	memset (setpath, 0x00, strlen (setpath));
	sprintf (setpath, "%s%s", home_path, SETTING);

	if (chk_cu == 0) // CREATE
	{
		snprintf (pbuf, MAX_NNG_MSG_SIZE, policyfmt, chk_worktime, uDs.uuid, VERSION,
					"Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",   
					home_path, setpath, setpath, setpath,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",
					setpath);
	}
	else if (chk_cu == 1) // 환경설정 적용 UPDATE
	{
		snprintf (pbuf, MAX_NNG_MSG_SIZE, policyfmt, chk_worktime, uDs.uuid, VERSION,
					"Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",   
					dpDs.detectpath, dpDs.movepath, dpDs.encpath, dpDs.decpath,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",
					setpath);
	}
	else if (chk_cu == 2) // 정책 받은 후 UPDATE
	{
		snprintf (pbuf, MAX_NNG_MSG_SIZE, policyfmt, chk_worktime, uDs.uuid, VERSION,
					"Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",   
					pDs.detectpath, pDs.movepath, pDs.encpath, pDs.decpath,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",
					setpath);
	}

	
					

	memset (msg, 0x00, strlen (msg));
	memset (defaultpolicy, 0x00, strlen (defaultpolicy));

	sprintf (defaultpolicy, "%s%s", home_path, DEFAULTPOLICY);

	if ((policy = fopen (defaultpolicy, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_DEFAULTPOLICY_CREATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_POLICY_CREATE_ERR);
		func_gtk_dialog_modal (2, window, msg);

		fclose (policy);
		return -1;
	}
	else
	{
		key_obj = json_tokener_parse (pbuf);
		BXLog (DBG, "%-30s	\n", "BXR_DEFAULTPOLICY_CREATE");
		fprintf (policy, "%s", json_object_to_json_string_ext (key_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
		fclose (policy);
	}
}
/* end of func_CreateDefaultPolicy(); */


/*---------------------------------------------------------------------------*/
/* Create PolicyFile                                            							*/
/*---------------------------------------------------------------------------*/
int func_CreatePolicy (json_object *rep_obj)
{
	FILE *policy ;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));
	memset (policy_path, 0x00, strlen (policy_path));

	sprintf (policy_path, "%s%s", home_path, POLICY);
	
	if (access (policy_path, F_OK) == 0)
	{
		if (access (policy_path, W_OK) == 0)
		{
			chk_fmanage = 3;
			func_Move(); // 기존에 정책파일 있으면 이름 plover_last_policy.json 변경하고 새로 생성
		}
	}

	if ((policy = fopen (policy_path, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_POLICY_CREATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_POLICY_CREATE_ERR);
		func_gtk_dialog_modal (2, window, msg);

		fclose (policy);
		return -1;
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_POLICY_CREATE");
		fprintf (policy, "%s", json_object_to_json_string_ext (rep_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
		fclose (policy);
	}

	return BXR_SUCCESS;
}
/* end of func_CreatePolicy(); */


/*---------------------------------------------------------------------------*/
/* Update PolicyFile                                            						*/
/*---------------------------------------------------------------------------*/
void func_UpdatePolicy()
{
	FILE *policy ;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *updatepolicy[MAX_PATH] = {0,};
	json_object *key_obj;
	char pbuf[MAX_NNG_MSG_SIZE] = {0,};
	char *policyfmt = NULL;

	policyfmt = "{\
							\"RP\" : {\
													\"Func\" : \"JFSRPLCY\",\
													\"RqRp\" : \"SC\",\
													\"DateTime\" : \"%s\",\
													\"Uuid\" : \"%s\",\
													\"AppVer\": \"%s\",\
													\"eType\" : \"S\",\
													\"eCode\" : 0,\
													\"eMesg\" : \"Success\",\
													\"PolicyType\" : \"%s\",\
													\"PSCheck\" : \"%s\",\
													\"PSRound\" : %d,\
													\"PSDate\" : \"%s\",\
													\"PSRegex\" : [\
																					{\"Name\": 1,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-8][0-9]{6}\"\
																					},\
																					{\"Name\": 2,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}-[0-9]{6}-[0-9]{2}\"\
																					},\
																					{\"Name\": 3,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-8][0-9]{6}\"\
																					},\
																					{\"Name\": 4,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 4\"\
																					},\
																					{\"Name\": 5,\
																					\"Use\": \"%s\",\
																					\"Count\": %d,\
																					\"Regex\": \"정규표현식 5\"\
																					},\
																					{\"Name\": 6,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 6\"\
																					},\
																					{\"Name\": 7,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 7\"\
																					},\
																					{\"Name\": 8,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 8\"\
																					},\
																					{\"Name\": 9,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 9\"\
																					},\
																					{\"Name\": 10,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 10\"\
																					},\
																					{\"Name\": 11,\
																						\"Use\": \"%s\",\
																						\"Count\": %d,\
																						\"Regex\": \"정규표현식 11\"\
																					}\
																				],\
													\"PSFile\" : [\
																				{\"Name\": 1,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 2,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 3,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 4,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 5,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 6,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 7,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 8,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Name\": 9,\
																					\"Use\": \"%s\"\
																				}\
																			],\
													\"DetectPath\" : \"%s\",\
													\"MovePath\" : \"%s\",\
													\"EncryptPath\" : \"%s\",\
													\"DecryptPath\" : \"%s\",\
													\"MaxFileSize\" : %d,\
													\"MaxFileAble\": \"%s\",\
													\"ForceEncrypt\": \"%s\",\
													\"DelEncFile\": \"%s\",\
													\"PWCheck\": \"%s\",\
													\"PWRound\" : %d,\
													\"PWDate\": \"%s\",\
													\"PWItem\" : [\
																				{\"Number\": 1,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 2,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 3,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 4,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 5,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 6,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 7,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 8,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 9,\
																					\"Use\": \"%s\"\
																				},\
																				{\"Number\": 10,\
																					\"Use\": \"%s\"\
																				}\
																			],\
													\"DetectPriority\": \"%s\",\
													\"RealTime\": \"%s\",\
													\"PopupMode\": \"%s\",\
													\"ShutDown\": \"%s\",\
													\"ReTry\": %d,\
													\"UpdateFolder\": \"%s\",\
													\"AutoUpdate\": \"%s\",\
													\"GladePath\": \"%s\",\
													}\
                                }";

	func_GetTime();

	snprintf (pbuf, MAX_NNG_MSG_SIZE, policyfmt, chk_worktime, uDs.uuid, VERSION,
					pDs.pstype, pDs.pscheck, pDs.psround, pDs.psdate,
					"Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1, "Y", 1,
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",   
					pDs.decpath, pDs.movepath, pDs.encpath, pDs.decpath, pDs.maxfsize, pDs.maxfable,
					pDs.forceenc, pDs.delencfile, pDs.pwcheck, pDs.pwround, pDs.pwdate,	
					"Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y", "Y",
					pDs.detectpriority, pDs.realtime, pDs.popupmode, pDs.shutdown, pDs.retry,
					pDs.updatefolder, pDs.autoupdate, pDs.gladepath);

	memset (msg, 0x00, strlen (msg));
	memset (updatepolicy, 0x00, strlen (updatepolicy));

	sprintf (updatepolicy, "%s%s", home_path, POLICY);

	if ((policy = fopen (updatepolicy, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_UPDATEPOLICY_CREATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_POLICY_CREATE_ERR);
		func_gtk_dialog_modal (2, window, msg);

		fclose (policy);
		return -1;
	}
	else
	{
		key_obj = json_tokener_parse (pbuf);
		BXLog (DBG, "%-30s	\n", "BXR_UPDATEPOLICY_CREATE");
		fprintf (policy, "%s", json_object_to_json_string_ext (key_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
		fclose (policy);
	}
}
/* end of func_UpdatePolicy(); */


/*---------------------------------------------------------------------------*/
/* Parse PolicyFile                                             							 */
/*---------------------------------------------------------------------------*/
int  func_ParsePolicy()
{
	FILE *policy;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *tmp[MAX_PATH] = {0,};
	char *buff;
	size_t result = 0;
	long lsize = 0;

	// key object
	json_object *key_obj, *v1_obj,*v1_psx_obj, *v1_psf_obj, *v1_pwi_obj;

	// array object
	json_object *psx_arr_obj, *psf_arr_obj, *pwi_arr_obj;

	// common object
	json_object *v1_plt_obj, *v1_rtm_obj, *v1_ppm_obj,  *v1_shd_obj, *v1_rtr_obj, *v1_udf_obj, *v1_aud_obj;
	json_object *v1_glp_obj, *v1_dpt_obj;
	
	// 민감정보 object
	json_object *v1_psc_obj, *v1_psr_obj, *v1_psd_obj, *v2_pxn_obj, *v2_pxu_obj, *v2_pxc_obj; 
	json_object *v2_pxr_obj, *v2_psn_obj, *v2_psu_obj, *v1_dtp_obj, *v1_mvp_obj, *v1_ecp_obj, *v1_dcp_obj;
	json_object *v1_mfs_obj, *v1_mfa_obj, *v1_fec_obj, *v1_def_obj;
	
	// 취약점 object
	json_object *v1_pwc_obj, *v1_pwr_obj, *v1_pwd_obj, *v2_pin_obj, *v2_piu_obj;
	

	// common Value
	const char *v1_plt_val, *v1_rtm_val, *v1_ppm_val,  *v1_shd_val, *v1_udf_val, *v1_aud_val, *v1_glp_val, *v1_dpt_val;
	int32_t *v1_rtr_val;	// Data int

	// 민감정보 Value
	const char *v1_psc_val, *v1_psd_val, *v2_pxn_val, *v2_pxu_val, *v2_pxc_val; 
	const char *v2_pxr_val, *v2_psn_val, *v2_psu_val, *v1_dtp_val, *v1_mvp_val, *v1_ecp_val, *v1_dcp_val;
	const char *v1_mfa_val, *v1_fec_val, *v1_def_val;
	int32_t *v1_psr_val, *v1_mfs_val;	// Data int
	
	// 취약점 Value
	const char *v1_pwc_val, *v1_pwd_val, *v2_pin_val, *v2_piu_val;
	int32_t *v1_pwr_val;	// Data int

	memset (msg, 0x00, strlen (msg));
	memset (&pDs, 0, sizeof (struct _Pdata_Storage));

	
	if ((policy = fopen (policy_path, "r")) == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_NONE",  strerror (errno));
		fclose (policy);
	}

	fseek (policy, 0, SEEK_END);
	lsize = ftell (policy);
	rewind (policy);

	buff = (char *) malloc (sizeof (char *) *lsize);
	if (buff == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_MEMORY_ERR", strerror (errno));
	}

	result = fread (buff, 1, lsize, policy);
	if (result != lsize)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_READ_ERR", strerror (errno));
	}

	fclose (policy);

	key_obj = json_tokener_parse (buff);
	json_object_object_get_ex (key_obj,				"RP",					 &v1_obj);
	json_object_object_get_ex (v1_obj,				"PolicyType",		 &v1_plt_obj);
	json_object_object_get_ex (v1_obj,				"PSCheck",			 &v1_psc_obj);
	json_object_object_get_ex (v1_obj,				"PSRound",			&v1_psr_obj);
	json_object_object_get_ex (v1_obj,				"PSDate",			  &v1_psd_obj);

	// 배열
	json_object_object_get_ex (v1_obj,				"PSRegex",			  &v1_psx_obj);

	// 배열
	json_object_object_get_ex (v1_obj,				"PSFile",				 &v1_psf_obj);

	json_object_object_get_ex (v1_obj,				"DetectPath",		 &v1_dtp_obj);
	json_object_object_get_ex (v1_obj,				"MovePath",			&v1_mvp_obj);
	json_object_object_get_ex (v1_obj,				"EncryptPath",		&v1_ecp_obj);
	json_object_object_get_ex (v1_obj,				"DecryptPath",		&v1_dcp_obj);
	json_object_object_get_ex (v1_obj,				"MaxFileSize",		 &v1_mfs_obj);
	json_object_object_get_ex (v1_obj,				"MaxFileAble",		&v1_mfa_obj);
	json_object_object_get_ex (v1_obj,				"ForceEncrypt",		&v1_fec_obj);
	json_object_object_get_ex (v1_obj,				"DelEncFile",		  &v1_def_obj);

	json_object_object_get_ex (v1_obj,				"PWCheck",			 &v1_pwc_obj);
	json_object_object_get_ex (v1_obj,				"PWRound",			&v1_pwr_obj);
	json_object_object_get_ex (v1_obj,				"PWDate",			  &v1_pwd_obj);

	// 배열
	json_object_object_get_ex (v1_obj,				"PWItem",			  &v1_pwi_obj);

	json_object_object_get_ex (v1_obj,				"DetectPriority",	&v1_dpt_obj);
	json_object_object_get_ex (v1_obj,				"RealTime",			  &v1_rtm_obj);
	json_object_object_get_ex (v1_obj,				"PopupMode",	  &v1_ppm_obj);
	json_object_object_get_ex (v1_obj,				"ShutDown",			&v1_shd_obj);
	json_object_object_get_ex (v1_obj,				"ReTry",			  	  &v1_rtr_obj);
	json_object_object_get_ex (v1_obj,				"UpdateFolder",		&v1_udf_obj);
	json_object_object_get_ex (v1_obj,				"AutoUpdate",		&v1_aud_obj);
	json_object_object_get_ex (v1_obj,				"GladePath",			&v1_glp_obj);


	v1_plt_val  = json_object_get_string (v1_plt_obj);
	v1_rtm_val  = json_object_get_string (v1_rtm_obj);
	v1_ppm_val  = json_object_get_string (v1_ppm_obj);
	v1_shd_val  = json_object_get_string (v1_shd_obj);
	v1_rtr_val  = json_object_get_int (v1_rtr_obj);
	v1_udf_val  = json_object_get_string (v1_udf_obj);
	v1_aud_val  = json_object_get_string (v1_aud_obj);
	v1_psc_val  = json_object_get_string (v1_psc_obj);
	v1_psr_val  = json_object_get_int (v1_psr_obj);
	v1_psd_val  = json_object_get_string (v1_psd_obj);
	v1_dtp_val  = json_object_get_string (v1_dtp_obj);
	v1_glp_val  = json_object_get_string (v1_glp_obj);
	v1_dpt_val  = json_object_get_string (v1_dpt_obj);
	v1_mvp_val  = json_object_get_string (v1_mvp_obj);
	v1_ecp_val  = json_object_get_string (v1_ecp_obj);
	v1_dcp_val  = json_object_get_string (v1_dcp_obj);
	v1_mfs_val  = json_object_get_int (v1_mfs_obj);
	v1_mfa_val  = json_object_get_string (v1_mfa_obj);
	v1_fec_val  = json_object_get_string (v1_fec_obj);
	v1_def_val  = json_object_get_string (v1_def_obj);
	v1_pwc_val  = json_object_get_string (v1_pwc_obj);
	v1_pwr_val  = json_object_get_int (v1_pwr_obj);
	v1_pwd_val  = json_object_get_string (v1_pwd_obj);

	for (int i = 0; i < json_object_array_length (v1_psx_obj); i++)
	{
		psx_arr_obj = json_object_array_get_idx (v1_psx_obj, i);

		json_object_object_get_ex (psx_arr_obj,		"Name",					&v2_pxn_obj);
		json_object_object_get_ex (psx_arr_obj,		"Use",					  &v2_pxu_obj);
		json_object_object_get_ex (psx_arr_obj,		"Count",				&v2_pxc_obj);
		json_object_object_get_ex (psx_arr_obj,		"Regex",				&v2_pxr_obj);

		v2_pxn_val  = json_object_get_int (v2_pxn_obj);
		v2_pxu_val  = json_object_get_string (v2_pxu_obj);
		v2_pxc_val  = json_object_get_int (v2_pxc_obj);
		v2_pxr_val  = json_object_get_string (v2_pxr_obj);

		pDs.srPd[i].name = v2_pxn_val;
		strcpy (&pDs.srPd[i].use, v2_pxu_val);
		pDs.srPd[i].count = v2_pxc_val;
		strcpy (&pDs.srPd[i].regex, v2_pxr_val);
	}

	for (int i = 0; i < json_object_array_length (v1_psf_obj); i++)
	{
		psf_arr_obj = json_object_array_get_idx (v1_psf_obj, i);

		json_object_object_get_ex (psf_arr_obj,		 "Name",				&v2_psn_obj);
		json_object_object_get_ex (psf_arr_obj,		 "Use",					   &v2_psu_obj);

		v2_psn_val  = json_object_get_int (v2_psn_obj);
		v2_psu_val  = json_object_get_string (v2_psu_obj);

		pDs.sfPd[i].name = v2_psn_val;
		strcpy (&pDs.sfPd[i].use, v2_psu_val);
	}

	for (int i = 0; i < json_object_array_length (v1_pwi_obj); i++)
	{
		pwi_arr_obj = json_object_array_get_idx (v1_pwi_obj, i);

		json_object_object_get_ex (pwi_arr_obj,		 "Number",				&v2_pin_obj);
		json_object_object_get_ex (pwi_arr_obj,		 "Use",					   &v2_piu_obj);

		v2_pin_val = json_object_get_int (v2_pin_obj);
		v2_piu_val = json_object_get_string (v2_piu_obj);

		pDs.wiPd[i].number = v2_pin_val;
		strcpy (&pDs.wiPd[i].use, v2_piu_val);
	}

	strcpy (&pDs.pstype, v1_plt_val);
	strcpy (&pDs.realtime, v1_rtm_val);
	strcpy (&pDs.popupmode, v1_ppm_val);
	strcpy (&pDs.shutdown, v1_shd_val);
	pDs.retry = v1_rtr_val;
	strcpy (&pDs.autoupdate, v1_aud_val);
	strcpy (&pDs.pscheck, v1_psc_val);
	pDs.psround = v1_psr_val;
	strcpy (&pDs.psdate, v1_psd_val);
	strcpy (&pDs.delencfile, v1_dtp_val);
	pDs.maxfsize = v1_mfs_val;
	strcpy (&pDs.maxfable, v1_mfa_val);
	strcpy (&pDs.forceenc, v1_fec_val);
	strcpy (&pDs.delencfile, v1_def_val);
	strcpy (&pDs.pwcheck, v1_pwc_val);
	pDs.pwround = v1_pwr_val;
	strcpy (&pDs.pwdate, v1_pwd_val);
	strcpy (&pDs.detectpriority, v1_dpt_val);

	strcpy (&pDs.detectpath, v1_dtp_val);
	strcpy (&pDs.movepath, v1_mvp_val);
	strcpy (&pDs.encpath, v1_ecp_val);
	strcpy (&pDs.decpath, v1_dcp_val);
	strcpy (&pDs.updatefolder, v1_udf_val);
	strcpy (&pDs.gladepath, v1_glp_val);

	if (strncmp (&pDs.detectpath, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.detectpath[4]);
		strcpy (&pDs.detectpath, tmp);
		strcpy (&dpDs.detectpath, tmp);
	}

	if (strncmp (&pDs.movepath, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.movepath[4]);
		strcpy (&pDs.movepath, tmp);
		strcpy (&dpDs.movepath, tmp);
	}

	if (strncmp (&pDs.encpath, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.encpath[4]);
		strcpy (&pDs.encpath, tmp);
		strcpy (&dpDs.encpath, tmp);
	}

	if (strncmp (&pDs.decpath, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.decpath[4]);
		strcpy (&pDs.decpath, tmp);
		strcpy (&dpDs.decpath, tmp);
	}

	if (strncmp (&pDs.updatefolder, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.updatefolder[4]);
		strcpy (&pDs.updatefolder, tmp);
	}

	if (strncmp (&pDs.gladepath, "HOME", 4) == 0)
	{
		sprintf (tmp, "%s%s", home_path, &pDs.gladepath[4]);
		strcpy (&pDs.gladepath, tmp);
	}

	BXLog (DBG, "%-30s	\n", "BXR_POLICY_PARSE");

	free (buff);

	return 0 ;
}
/* end of func_ParsePolicy(); */


/*---------------------------------------------------------------------------*/
/* Parse Default PolicyFile                                             			 */
/*---------------------------------------------------------------------------*/
int  func_ParseDeFaultPolicy()
{
	FILE *policy;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *tmp[MAX_PATH] = {0,};
	char *buff;
	size_t result = 0;
	long lsize = 0;

	// key object
	json_object *key_obj, *v1_obj,*v1_psx_obj, *v1_psf_obj, *v1_pwi_obj;

	// array object
	json_object *psx_arr_obj, *psf_arr_obj, *pwi_arr_obj;

	// common object
	json_object *v1_rtm_obj, *v1_ppm_obj,  *v1_shd_obj, *v1_rtr_obj, *v1_udf_obj, *v1_aud_obj;
	json_object *v1_glp_obj;
	
	// 민감정보 object
	json_object *v1_psc_obj, *v2_pxn_obj, *v2_pxu_obj, *v2_pxc_obj; 
	json_object *v2_pxr_obj, *v2_psn_obj, *v2_psu_obj, *v1_dtp_obj, *v1_mvp_obj, *v1_ecp_obj, *v1_dcp_obj;
	json_object *v1_mfs_obj, *v1_mfa_obj, *v1_fec_obj, *v1_def_obj;
	
	// 취약점 object
	json_object *v1_pwc_obj, *v2_pin_obj, *v2_piu_obj;
	

	// common Value
	const char *v1_rtm_val, *v1_ppm_val,  *v1_shd_val, *v1_udf_val, *v1_aud_val, *v1_glp_val;
	int32_t *v1_rtr_val;	// Data int

	// 민감정보 Value
	const char *v1_psc_val, *v2_pxn_val, *v2_pxu_val, *v2_pxc_val; 
	const char *v2_pxr_val, *v2_psn_val, *v2_psu_val, *v1_dtp_val, *v1_mvp_val, *v1_ecp_val, *v1_dcp_val;
	const char *v1_mfa_val, *v1_fec_val, *v1_def_val;
	int32_t *v1_mfs_val;	// Data int
	
	// 취약점 Value
	const char *v1_pwc_val, *v2_pin_val, *v2_piu_val;

	memset (msg, 0x00, strlen (msg));
	memset (&dpDs, 0, sizeof (struct _dPdata_Storage));

	
	if ((policy = fopen (policy_path, "r")) == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_NONE",  strerror (errno));
		fclose (policy);
	}

	fseek (policy, 0, SEEK_END);
	lsize = ftell (policy);
	rewind (policy);

	buff = (char *) malloc (sizeof (char *) *lsize);
	if (buff == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_MEMORY_ERR", strerror (errno));
	}

	result = fread (buff, 1, lsize, policy);
	if (result != lsize)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_POLICY_READ_ERR", strerror (errno));
	}

	fclose (policy);

	key_obj = json_tokener_parse (buff);
	json_object_object_get_ex (key_obj,				"RP",					 &v1_obj);
	json_object_object_get_ex (v1_obj,				"PSCheck",			 &v1_psc_obj);

	// 배열
	json_object_object_get_ex (v1_obj,				"PSRegex",			  &v1_psx_obj);

	// 배열
	json_object_object_get_ex (v1_obj,				"PSFile",				 &v1_psf_obj);

	json_object_object_get_ex (v1_obj,				"DetectPath",		 &v1_dtp_obj);
	json_object_object_get_ex (v1_obj,				"MovePath",			&v1_mvp_obj);
	json_object_object_get_ex (v1_obj,				"EncryptPath",		&v1_ecp_obj);
	json_object_object_get_ex (v1_obj,				"DecryptPath",		&v1_dcp_obj);
	json_object_object_get_ex (v1_obj,				"MaxFileSize",		 &v1_mfs_obj);
	json_object_object_get_ex (v1_obj,				"MaxFileAble",		&v1_mfa_obj);
	json_object_object_get_ex (v1_obj,				"ForceEncrypt",		&v1_fec_obj);
	json_object_object_get_ex (v1_obj,				"DelEncFile",		  &v1_def_obj);

	json_object_object_get_ex (v1_obj,				"PWCheck",			 &v1_pwc_obj);
	
	// 배열
	json_object_object_get_ex (v1_obj,				"PWItem",			  &v1_pwi_obj);

	json_object_object_get_ex (v1_obj,				"RealTime",			  &v1_rtm_obj);
	json_object_object_get_ex (v1_obj,				"PopupMode",	  &v1_ppm_obj);
	json_object_object_get_ex (v1_obj,				"ShutDown",			&v1_shd_obj);
	json_object_object_get_ex (v1_obj,				"ReTry",			  	  &v1_rtr_obj);
	json_object_object_get_ex (v1_obj,				"UpdateFolder",		&v1_udf_obj);
	json_object_object_get_ex (v1_obj,				"AutoUpdate",		&v1_aud_obj);
	json_object_object_get_ex (v1_obj,				"GladePath",			&v1_glp_obj);


	v1_rtm_val  = json_object_get_string (v1_rtm_obj);
	v1_ppm_val  = json_object_get_string (v1_ppm_obj);
	v1_shd_val  = json_object_get_string (v1_shd_obj);
	v1_rtr_val  = json_object_get_int (v1_rtr_obj);
	v1_udf_val  = json_object_get_string (v1_udf_obj);
	v1_aud_val  = json_object_get_string (v1_aud_obj);
	v1_psc_val  = json_object_get_string (v1_psc_obj);

	v1_dtp_val  = json_object_get_string (v1_dtp_obj);
	v1_glp_val  = json_object_get_string (v1_glp_obj);
	v1_mvp_val  = json_object_get_string (v1_mvp_obj);
	v1_ecp_val  = json_object_get_string (v1_ecp_obj);
	v1_dcp_val  = json_object_get_string (v1_dcp_obj);
	v1_mfs_val  = json_object_get_int (v1_mfs_obj);
	v1_mfa_val  = json_object_get_string (v1_mfa_obj);
	v1_fec_val  = json_object_get_string (v1_fec_obj);
	v1_def_val  = json_object_get_string (v1_def_obj);
	v1_pwc_val  = json_object_get_string (v1_pwc_obj);

	for (int i = 0; i < json_object_array_length (v1_psx_obj); i++)
	{
		psx_arr_obj = json_object_array_get_idx (v1_psx_obj, i);

		json_object_object_get_ex (psx_arr_obj,		"Name",					&v2_pxn_obj);
		json_object_object_get_ex (psx_arr_obj,		"Use",					  &v2_pxu_obj);
		json_object_object_get_ex (psx_arr_obj,		"Count",				&v2_pxc_obj);
		json_object_object_get_ex (psx_arr_obj,		"Regex",				&v2_pxr_obj);

		v2_pxn_val  = json_object_get_int (v2_pxn_obj);
		v2_pxu_val  = json_object_get_string (v2_pxu_obj);
		v2_pxc_val  = json_object_get_int (v2_pxc_obj);
		v2_pxr_val  = json_object_get_string (v2_pxr_obj);

		dpDs.srPd[i].name = v2_pxn_val;
		strcpy (dpDs.srPd[i].use, v2_pxu_val);
		dpDs.srPd[i].count = v2_pxc_val;
		strcpy (dpDs.srPd[i].regex, v2_pxr_val);
	}

	for (int i = 0; i < json_object_array_length (v1_psf_obj); i++)
	{
		psf_arr_obj = json_object_array_get_idx (v1_psf_obj, i);

		json_object_object_get_ex (psf_arr_obj,		 "Name",				&v2_psn_obj);
		json_object_object_get_ex (psf_arr_obj,		 "Use",					   &v2_psu_obj);

		v2_psn_val  = json_object_get_int (v2_psn_obj);
		v2_psu_val  = json_object_get_string (v2_psu_obj);

		dpDs.sfPd[i].name = v2_psn_val;
		strcpy (dpDs.sfPd[i].use, v2_psu_val);
	}

	strcpy (dpDs.detectpath, v1_dtp_val);
	strcpy (dpDs.movepath, v1_mvp_val);
	strcpy (dpDs.encpath, v1_ecp_val);
	strcpy (dpDs.decpath, v1_dcp_val);
	dpDs.maxfsize = v1_mfs_val;
	strcpy (dpDs.maxfable, v1_mfa_val);
	strcpy (dpDs.forceenc, v1_fec_val);
	strcpy (dpDs.delencfile, v1_def_val);
	strcpy (dpDs.pwcheck, v1_pwc_val);
	strcpy (pDs.pwcheck, v1_pwc_val);

	for (int i = 0; i < json_object_array_length (v1_pwi_obj); i++)
	{
		pwi_arr_obj = json_object_array_get_idx (v1_pwi_obj, i);

		json_object_object_get_ex (pwi_arr_obj,		 "Number",				&v2_pin_obj);
		json_object_object_get_ex (pwi_arr_obj,		 "Use",					   &v2_piu_obj);

		v2_pin_val = json_object_get_int (v2_pin_obj);
		v2_piu_val = json_object_get_string (v2_piu_obj);

		dpDs.wiPd[i].number = v2_pin_val;
		strcpy (dpDs.wiPd[i].use, v2_piu_val);
	}

	strcpy (dpDs.realtime, v1_rtm_val);
	strcpy (dpDs.popupmode, v1_ppm_val);
	strcpy (dpDs.shutdown, v1_shd_val);
	dpDs.retry = v1_rtr_val;
	strcpy (dpDs.updatefolder, v1_udf_val);
	strcpy (dpDs.autoupdate, v1_aud_val);
	strcpy (dpDs.pscheck, v1_psc_val);
	strcpy (dpDs.delencfile, v1_dtp_val);
	strcpy (dpDs.gladepath, v1_glp_val);
	strcpy (pDs.pscheck, v1_psc_val);

	BXLog (DBG, "%-30s	\n", "BXR_DEFAULTPOLICY_PARSE");

	free (buff);

	return 0 ;
}
/* end of func_ParseDeFaultPolicy(); */


/*---------------------------------------------------------------------------*/
/* Create Last Plan File                                            					*/
/*---------------------------------------------------------------------------*/
int func_CreateLastPlan (json_object *rep_obj, int plan_type)
{
	FILE *lplan;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *lplan_path[MAX_PATH] = {0,};// 마지막 정기검사 파일 경로

	if (plan_type == 3) // 민감정보
	{
		sprintf (lplan_path, "%s%s", home_path, PLAN_SENSITIVE);
	}
	else if (plan_type == 7) // 취약점
	{
		sprintf (lplan_path, "%s%s", home_path, PLAN_WARNING);
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_LASTPLAN_TYPE_ERR");
	}
	
	if ((lplan = fopen (lplan_path, "w+")) == NULL)
	{
		BXLog (DBG, "%-30s	\n", "BXR_LASTPLAN_CREATE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_LASTPLAN_CREATE);
		func_gtk_dialog_modal (2, window, msg);

		fclose (lplan);
		return -1;
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_LASTPLAN_CREATE");
		fprintf (lplan, "%s", json_object_to_json_string_ext (rep_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
		fclose (lplan);
	}

	return BXR_SUCCESS;
}
/* end of func_CreateLastPlan(); */


/*---------------------------------------------------------------------------*/
/* Check Last Plan Detect File                                             		  */
/*---------------------------------------------------------------------------*/
void func_ChkLastPlanFile()
{
	char *planfile[MAX_ERROR_MSG] = {0,};

	memset (planfile, 0x00, strlen (planfile));

	sprintf (planfile, "%s%s", home_path, PLAN_SENSITIVE);
	
	if (access (planfile, F_OK) == 0)
	{
		func_nng_client (9, 0);
		remove (planfile);
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_PLAN_SENSITIVE_NONE");
	}

	memset (planfile, 0x00, strlen (planfile));

	sprintf (planfile, "%s%s", home_path, PLAN_WARNING);
	
	if (access (planfile, F_OK) == 0)
	{
		func_nng_client (10, 0);
		remove (planfile);
	}
	else
	{
		BXLog (DBG, "%-30s	\n", "BXR_PLAN_WARNING_NONE");
	}
}
/* end of func_ChkLastPlanFile(); */


/*---------------------------------------------------------------------------*/
/* Compare Plan Detect Date                                             		*/
/*---------------------------------------------------------------------------*/
void func_ComparePDdate()
{
	struct  timeval t;
    struct  tm *tm;
	FILE *fd;
	int pstime = 0, pwtime = 0, restime = 0;
	char  cmdbuf[256] = {0,};
	char  strbuf[1024] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};

	gettimeofday (&t, NULL);
	localtime (&t.tv_sec);
	sprintf (cmdbuf, "date -d \"%s\" +%%s", pDs.psdate);

	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_COMPARE_DATE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		BXLog (DBG, "%-30s	[%s][%d]\n", "COMPARE DATE POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 1024, fd) != NULL)
		{
			pstime = atoi (strbuf);
		}
	}
	
	memset (cmdbuf, 0x00, strlen (cmdbuf));
	memset (strbuf, 0x00, strlen (strbuf));
	sprintf (cmdbuf, "date -d \"%s\" +%%s", pDs.pwdate);

	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_COMPARE_DATE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		BXLog (DBG, "%-30s	[%s][%d]\n", "COMPARE DATE POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 1024, fd) != NULL)
		{
			pwtime = atoi (strbuf);
		}
	}

	pclose (fd);

	restime = pstime - pwtime;
	if (restime > 0) // 민감정보 정기검사가 더 뒤에 날짜
	{
		memset (pDs.detectpriority, 0x00, 2);
		strcpy (pDs.detectpriority, "W");
	}
	else if (restime < 0) // 취약점 정기검사가 더 뒤에 날짜
	{
		memset (pDs.detectpriority, 0x00, 2);
		strcpy (pDs.detectpriority, "S");
	}
}
/* end of func_ComparePDdate(); */


/*---------------------------------------------------------------------------*/
/* Send Plan Sensitive File                                             			 */
/*---------------------------------------------------------------------------*/
char *func_SendPSFile()
{
	FILE *psfile;
	char *buff = NULL;
	int rsize = 0;
	size_t fsize = 0;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *psfile_path[MAX_PATH] = {0,};

	memset (msg, 0x00, strlen (msg));
	
	sprintf (psfile_path, "%s%s", home_path, PLAN_SENSITIVE);

	if ((psfile = fopen (psfile_path, "r")) == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_PSFILE_NONE",  strerror (errno));
		fclose (psfile);
	}

	fseek (psfile, 0, SEEK_END);
	fsize = ftell (psfile);

	buff = malloc (fsize);
	memset (buff, 0x00, fsize + 1);

	fseek (psfile, 0, SEEK_SET);

	rsize = fread (buff, 1, fsize, psfile);
	
	if (fsize != rsize)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_PSFILE_READ_ERR", strerror (errno));
	}

	fclose (psfile);

	free (buff);

	return buff;
}
/* end of func_SendPSFile(); */

/*---------------------------------------------------------------------------*/
/* Send Plan Warning File                                             				*/
/*---------------------------------------------------------------------------*/
char *func_SendPWFile()
{
	FILE *pwfile;
	char *msg[MAX_ERROR_MSG] = {0,};
	char *buff = NULL;
	int rsize = 0;
	size_t fsize = 0;
	char *pwfile_path[MAX_PATH] = {0,};

	memset (msg, 0x00, strlen (msg));

	sprintf (pwfile_path, "%s%s", home_path, PLAN_WARNING);

	if ((pwfile = fopen (pwfile_path, "r")) == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_PWFILE_NONE",  strerror (errno));
	}

	fseek (pwfile, 0, SEEK_END);
	fsize = ftell (pwfile);

	buff = malloc (fsize);
	memset (buff, 0x00, fsize + 1);

	fseek (pwfile, 0, SEEK_SET);

	rsize = fread (buff, 1, fsize, pwfile);
	
	if (fsize != rsize)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_PWFILE_READ_ERR", strerror (errno));
	}

	fclose (pwfile);

	free (buff);

	return buff;
}
/* end of func_SendPWFile(); */


/*---------------------------------------------------------------------------*/
/* Conf File Check							                                            */
/*---------------------------------------------------------------------------*/
int func_ChkConf()
{	
	int chkconf = 0;
	char *conf_path[MAX_PATH] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	// conf File Check
	sprintf (conf_path, "%s%s", home_path, CONF);

	if (access (conf_path, F_OK) != -1) //conf파일 있음
	{
		func_ParseConf (conf_path);
    }
	else // conf파일 없음
	{
		BXLog (DBG, "%-30s	\n", "BXR_CONF_NONE_ERR");
		func_CreateConf (conf_path);
		func_ParseConf (conf_path);
	}

	return 0;
}
/* end of func_ChkConf(); */


/*---------------------------------------------------------------------------*/
/* Policy File Check							                                         */
/*---------------------------------------------------------------------------*/
int func_ChkPolicy()
{	
	int chkpolicy = 0;
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	memset (policy_path, 0x00, strlen (policy_path));
	sprintf (policy_path, "%s%s", home_path, DEFAULTPOLICY);

	if (access (policy_path, F_OK) != -1) // default policy파일 있음.
	{
		func_ParseDeFaultPolicy();
		BXLog (DBG, "%-30s	\n", "CHECK_DEFAULT_POLICY");
	}
	else
	{
		func_CreateDefaultPolicy (0);
		BXLog (DBG, "%-30s	\n", "CREATE_DEFAULT_POLICY");
		func_ParseDeFaultPolicy();
		BXLog (DBG, "%-30s	\n", "CHECK_DEFAULT_POLICY");
	}

	memset (policy_path, 0x00, strlen (policy_path));
	sprintf (policy_path, "%s%s", home_path, POLICY);

	if (access (policy_path, F_OK) != -1) // policy파일 있음
	{
		func_ParsePolicy();
		BXLog (DBG, "%-30s	\n", "CHECK_POLICY");
    }


	// 민감정보/취약점 정기검사 둘다 예약되어있는  경우
	if ((strncmp (&pDs.pscheck, "Y", 1) == 0) && (strncmp (&pDs.pwcheck, "Y", 1) == 0))
	{
		func_ComparePDdate();
	}

	return 0;
}
/* end of func_ChkPolicy(); */


/*---------------------------------------------------------------------------*/
/* Initiate Client                                            							    */
/*---------------------------------------------------------------------------*/
int func_Initiate()
{	
	func_nng_client (0, 0);
	
	return 0;
}
/* end of func_Initiate(); */


/*---------------------------------------------------------------------------*/
/* User Check 												                               */
/*---------------------------------------------------------------------------*/
void func_UsrChk()
{
	int cmp = 0;
	int chkconf = 0;
	char *usrinfo_buf[40] = {0,};
	char *conf_path[MAX_PATH] = {0,};

	sprintf (conf_path, "%s%s", home_path, CONF);

	chkconf = func_UpdateConf (conf_path);
	BXLog (DBG, "%-30s	[%d]\n", "BXR_CONF_UPDATE", chkconf);
	chkconf = func_ParseConf (conf_path);
	BXLog (DBG, "%-30s	[%d]\n", "BXR_CONF_PARSE", chkconf);

	gtk_label_set_text (GTK_LABEL (m_version_label), VERSION);

	if ((strncmp (chk_online, "Y", 1) == 0))
	{
		sprintf (usrinfo_buf, "%s %s", uDs.name, uDs.rank);
		gtk_label_set_text (GTK_LABEL (m_userinfo_label), usrinfo_buf);
	}
	else
	{
		sprintf (usrinfo_buf, "%s %s", cDs.uname, cDs.urank);
		gtk_label_set_text (GTK_LABEL (m_userinfo_label), usrinfo_buf);
	}
	
}
/* end of func_UsrChk(); */


/*---------------------------------------------------------------------------*/
/* User Add 												                               */
/*---------------------------------------------------------------------------*/
void func_UsrAdd()
{
	char *web_uri_login[1024] = {0,};

	BXLog (DBG, "%-30s	\n", "USER ADD");
	e_context = webkit_web_view_get_context (e_webview);
	webkit_web_view_reload_bypass_cache (e_webview);
	webkit_web_context_clear_cache (e_context);
	webkit_web_context_set_tls_errors_policy (e_context, WEBKIT_TLS_ERRORS_POLICY_IGNORE); //SSL에러 무시 정책 설정
	sprintf (web_uri_login, "https://%s:%s/client/created?uuid=%s", cDs.sip, cDs.wport, uDs.uuid); //URI에 UUID추가
	webkit_web_view_load_uri (e_webview, web_uri_login); //웹뷰 불러오기
	e_webview_load_failed (e_webview, load_event);
	gtk_widget_show (enrollment_window);				 //사용자 등록 창
	gtk_main();

}
/* end of func_UsrAdd(); */


/*---------------------------------------------------------------------------*/
/* User UUID Parsing                                         						  */
/*---------------------------------------------------------------------------*/
int func_Uuid()
{
	FILE *uidfp	= NULL;
	char *pstr	= NULL;
	char  strbuf[300] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	uidfp = fopen ("/etc/fstab", "r");

	if (NULL == uidfp)
	{
        BXLog (DBG, "%-30s	\n", "BXR_UUID_NONE_ERR");
		sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_UUID_NONE_ERR);
		func_gtk_dialog_modal (2, window, msg);
		fclose (uidfp);
		return -1;
	}

	while (feof (uidfp) == 0)
	{
		pstr = fgets (strbuf, sizeof (strbuf), uidfp);

		if (pstr != 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
		{
			if (pstr[0] == 'U' && pstr[42] == '/' && pstr[43] != 'h')
			{
				for (int i = 0; i < 36; i++)
				{ 
					set_uuid[i] = pstr[i+5];
				}
				strcpy (uDs.uuid, set_uuid);
				BXLog (DBG, "%-30s	[%s]\n", "BXR_UUID_OK", uDs.uuid);
			}
		}
    }
    
    memset (strbuf, 0, sizeof(strbuf));
	fclose (uidfp);

	return BXR_SUCCESS;
}
/* end of func_Uuid(); */


/****************** Start of DB ********************/
/*---------------------------------------------------------------------------*/
/*	BXDB CallBack Pop Data														 */
/*---------------------------------------------------------------------------*/
int callback (void *NotUsed, int argc, char **argv, char **azColName)
{
    NotUsed = 0;

	switch (chk_dbtable)
	{
		case 0:
			sprintf (fDs[chk_dldbcol].fname,    "%s", argv[0]);
			fDs[chk_dldbcol].jcnt   = atoi (argv[1]);
			fDs[chk_dldbcol].dcnt  = atoi (argv[2]);
			fDs[chk_dldbcol].fgcnt = atoi (argv[3]);
			fDs[chk_dldbcol].pcnt  = atoi (argv[4]);
			sprintf (fDs[chk_dldbcol].fstat,     	 "%s", argv[5]);
			fDs[chk_dldbcol].fsize  = atoi (argv[6]);
			sprintf (fDs[chk_dldbcol].ftype,     "%s", argv[7]);
			sprintf (fDs[chk_dldbcol].fpath,	   "%s", argv[8]);
			fDs[chk_dldbcol].round  = atoi (argv[9]);
			sprintf (fDs[chk_dldbcol].start,		 "%s", argv[10]);
			sprintf (fDs[chk_dldbcol].end,		"%s", argv[11]);
			chk_dldbcol++;

			break;

		case 1:
			sprintf (fDs[chk_dldbcol].fname,    "%s", argv[0]);
			fDs[chk_dldbcol].jcnt   = atoi (argv[1]);
			fDs[chk_dldbcol].dcnt  = atoi (argv[2]);
			fDs[chk_dldbcol].fgcnt = atoi (argv[3]);
			fDs[chk_dldbcol].pcnt  = atoi (argv[4]);
			sprintf (fDs[chk_dldbcol].fstat,     	 "%s", argv[5]);
			fDs[chk_dldbcol].fsize  = atoi (argv[6]);
			sprintf (fDs[chk_dldbcol].ftype,     "%s", argv[7]);
			sprintf (fDs[chk_dldbcol].fpath,	   "%s", argv[8]);
			fDs[chk_dldbcol].round  = atoi (argv[9]);
			sprintf (fDs[chk_dldbcol].start,		 "%s", argv[10]);
			sprintf (fDs[chk_dldbcol].end,		"%s", argv[11]);
			chk_dldbcol++;

			break;

		case 2:
			sprintf (viDs[chk_vldbcol].item,    "%s", argv[0]);
			sprintf (viDs[chk_vldbcol].stat,    "%s", argv[1]);
			viDs[chk_vldbcol].round   = atoi (argv[2]);
			sprintf (viDs[chk_vldbcol].start, "%s", argv[3]);
			sprintf (viDs[chk_vldbcol].end, "%s", argv[4]);
			chk_vldbcol++;

			break;

		case 3:
			sprintf (viDs[chk_vldbcol].item,    "%s", argv[0]);
			sprintf (viDs[chk_vldbcol].stat,    "%s", argv[1]);
			viDs[chk_vldbcol].round   = atoi (argv[2]);
			sprintf (viDs[chk_vldbcol].start,    "%s", argv[3]);
			sprintf (viDs[chk_vldbcol].end,    "%s", argv[4]);
			chk_vldbcol++;

			break;
	}
	
    return 0;
}
/* BXDB CallBack Pop Data() */


/*---------------------------------------------------------------------------*/
/*	BXDB Push Data														           	  */
/*---------------------------------------------------------------------------*/
int  BXDB_Push (int insert_cnt, int type)
{
	sqlite3	*db;
	int			 rc 		    		= 0;
	char 	 *err_msg  		    = 0;
	char	 *sql			 		 = 0;
	char 	   sql_buf[MAX_PATH] = {0,};
	char	 *db_path[MAX_PATH] = {0,};

	sprintf (db_path, "%s%s", home_path, DBFILE);

	rc = sqlite3_open (db_path, &db);

    if (rc != SQLITE_OK)
    {
		BXLog (DBG, "BXR_DB_OPEN_ERR: %s\n", sqlite3_errmsg (db));
		sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    }

	switch (type)
	{
		case 0: 
			sprintf (sql_buf, "INSERT INTO user_sensitive VALUES ('%s', %d, %d, %d, %d, '%s', %d, '%s', '%s', %d, '%s', '%s');", 
																																							fDs[insert_cnt].fname,
																																							fDs[insert_cnt].jcnt,
																																							fDs[insert_cnt].dcnt,
																																							fDs[insert_cnt].fgcnt,
																																							fDs[insert_cnt].pcnt,
																																							fDs[insert_cnt].fstat,
																																							fDs[insert_cnt].fsize,
																																							fDs[insert_cnt].ftype,
																																							fDs[insert_cnt].fpath,
																																							0,
																																							fDs[insert_cnt].start,
																																							fDs[insert_cnt].end);

			break;

		case 1: 
			sprintf (sql_buf, "INSERT INTO plan_sensitive VALUES ('%s', %d, %d, %d, %d, '%s', %d, '%s', '%s', %d, '%s', '%s');", 
																																							fDs[insert_cnt].fname,
																																							fDs[insert_cnt].jcnt,
																																							fDs[insert_cnt].dcnt,
																																							fDs[insert_cnt].fgcnt,
																																							fDs[insert_cnt].pcnt,
																																							fDs[insert_cnt].fstat,
																																							fDs[insert_cnt].fsize,
																																							fDs[insert_cnt].ftype,
																																							fDs[insert_cnt].fpath,
																																							fDs[insert_cnt].round,
																																							fDs[insert_cnt].start,
																																							fDs[insert_cnt].end);

			break;

		case 2:
			sprintf (sql_buf, "INSERT INTO user_warning VALUES ('%s', '%s', %d, '%s', '%s');", 
																									viDs[insert_cnt].item,
																									viDs[insert_cnt].stat,
																									viDs[insert_cnt].round,
																									viDs[insert_cnt].start,
																									viDs[insert_cnt].end);


			break;

		case 3: 
			sprintf (sql_buf, "INSERT INTO plan_warning VALUES (%d, '%s', %d, '%s', '%s');", 
																									viDs[insert_cnt].item,
																									viDs[insert_cnt].stat,
																									viDs[insert_cnt].round,
																									viDs[insert_cnt].start,
																									viDs[insert_cnt].end);
																									

			break;

		default: 

			break;
	}																																						

	sql = sql_buf;

	rc = sqlite3_exec (db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
		BXLog (DBG, "BXR_DB_EXECUTION_ERR: %s, rc = [%d]\n", err_msg, rc);
        sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    } 

	sqlite3_close (db);

	return 0;
}
/* end of BXDB_Push () */


/*---------------------------------------------------------------------------*/
/*	BXDB Pop Data														           	   */
/*---------------------------------------------------------------------------*/
int BXDB_Pop (int type)
{
    sqlite3 *db;
	int 		rc = 0;
	char 	 *sql [1024] = {0,};
    char     *err_msg = 0;
	char	 *db_path[MAX_PATH] = {0,};
	gchar 	*date1 = 0, *date2 = 0;

	sprintf (db_path, "%s%s", home_path, DBFILE);

	rc = sqlite3_open (db_path, &db);
    
    if (rc != SQLITE_OK)
    {
		BXLog (DBG, "BXR_DB_OPEN_ERR: %s\n", sqlite3_errmsg (db));
		sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    }

	switch (type)
	{
		case 0: 
			date1 = (gchar *) gtk_label_get_text (GTK_LABEL (dl_date1_label));
			date2 = (gchar *) gtk_label_get_text (GTK_LABEL (dl_date2_label));
			sprintf (sql, "SELECT * FROM user_sensitive WHERE DATE(작업시작) BETWEEN '%s' AND '%s'", date1, date2);
			chk_dldbcol = 0;

			break;

		case 1: 
			date1 = (gchar *) gtk_label_get_text (GTK_LABEL (dl_date1_label));
			date2 = (gchar *) gtk_label_get_text (GTK_LABEL (dl_date2_label));
			sprintf (sql, "SELECT * FROM plan_sensitive WHERE DATE(작업시작) BETWEEN '%s' AND '%s'", date1, date2);
			chk_dldbcol = 0;

			break;
		
		case 2: 
			date1 = (gchar *) gtk_label_get_text (GTK_LABEL (vl_date1_label));
			date2 = (gchar *) gtk_label_get_text (GTK_LABEL (vl_date2_label));
			sprintf (sql, "SELECT * FROM user_warning WHERE DATE(작업시작) BETWEEN '%s' AND '%s'", date1, date2);
			chk_vldbcol = 0;

			break;

		case 3: 
			date1 = (gchar *) gtk_label_get_text (GTK_LABEL (vl_date1_label));
			date2 = (gchar *) gtk_label_get_text (GTK_LABEL (vl_date2_label));
			sprintf (sql, "SELECT * FROM plan_warning WHERE DATE(작업시작) BETWEEN '%s' AND '%s'", date1, date2);
			chk_vldbcol = 0;

			break;

		default:

			break;
	}
	
	rc = sqlite3_exec (db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        BXLog (DBG, "BXR_DB_EXECUTION_ERR: %s, rc = [%d]\n", err_msg, rc);
        sqlite3_free   (err_msg);
        sqlite3_close (db);
        
        return 1;
    } 

    sqlite3_close (db);

    return 0;
}
/* end of BXDB_Pop() */
/****************** End of DB ********************/


/***************** Start of REGEX ******************/
/*---------------------------------------------------------------------------*/
/* Regex Compile                                             							*/
/*---------------------------------------------------------------------------*/
int compile_regex (regex_t *r, const char *regex_text)
{
	int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);

	if (status != 0)
	{
		char error_message[MAX_ERROR_MSG];

		regerror (status, r, error_message, MAX_ERROR_MSG);

		BXLog (DBG, "%-30s	[%s]: [%s]\n", "BXR_REGEX_COMPILE_ERR", regex_text, error_message);

		return 1;
	}

	return 0;
}
/* end of compile_regex(); */


/*---------------------------------------------------------------------------*/
/* Jumin and Foreign Check                                  					 */
/*---------------------------------------------------------------------------*/
char match_regex_jnfg (regex_t *r, const char *to_match, size_t fsize, char *filepath, char *filename)
{
	char *tmppath[MAX_PATH] = {0,};
	/* "P" is a pointer into the string which points to the end of the
	previous match. */
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed. //
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교 //
	while (1)
	{
		int nomatch = regexec (r, p, n_matches, m, 0);
		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				int start;

				if (m[i].rm_so == -1)
				{
					break;
				}

				start = m[i].rm_so + (p - to_match);

				// 주민번호, 외국인등록번호 정규식 검사 통과
				if (i == 0)
				{
					int chk = 0, jtmp = 0, fgtmp = 0, sum = 0, chkbuf6 = 0;
					char buf_tmp[15];

					// 주민번호, 외국인등록번호 유효성 검사
					for (int j = 0; j < 14; j++)
					{
						buf_tmp[j] = *(to_match + start + j);
						buf_tmp[j] -= 48;
					}

					sum = buf_tmp[0]*2 + buf_tmp[1]*3 + buf_tmp[2]*4 + buf_tmp[3]*5 + buf_tmp[4]*6 + buf_tmp[5]*7
					+ buf_tmp[7]*8 + buf_tmp[8]*9 + buf_tmp[9]*2 + buf_tmp[10]*3 + buf_tmp[11]*4 + buf_tmp[12]*5;

					chk = buf_tmp[13];
					chkbuf6  = buf_tmp[7];
					jtmp = 11 - (sum % 11);	// 주민번호
					fgtmp = 13 - (sum % 11);	// 외국인번호

					if (jtmp >= 10)
					{
						jtmp -= 10;
					}

					if (fgtmp >= 10)
					{
						fgtmp -= 10;
					}

					// 주민번호 유효성 통과
					if (jtmp == chk)
					{
						if((chkbuf6 == 1) || (chkbuf6 == 2) || (chkbuf6 == 3) || (chkbuf6 == 4))
						{
							int res = strcmp (chk_fname, filename); // 같은파일 = 0

							if (res != 0)
							{
								chk_fcnt++;
							}

							// 읽고있는중인 파일 이름 저장
							strcpy (chk_fname, filename);

							// 검출된 주민등록번호의 수
							fDs[chk_fcnt].jcnt++;

							// data 구조체에 저장
							strcpy (tmppath, filepath);
							strcpy (fDs[chk_fcnt].fpath, dirname (tmppath));
							strcpy (fDs[chk_fcnt].fname, filename);
							fDs [chk_fcnt].fsize = fsize;
							strcpy (fDs[chk_fcnt].fstat, "일반");
							strcpy (fDs[chk_fcnt].ftype, chk_ftype);
							}
					}

					// 외국인등록번호 유효성 통과
					if (fgtmp == chk)
					{
						if((chkbuf6 == 5) || (chkbuf6 == 6) || (chkbuf6 == 7) || (chkbuf6 == 8))
						{
							int res = strcmp (chk_fname, filename); // 같은파일 = 0

							if (res != 0)
							{
								chk_fcnt++;
							}

							// 읽고있는중인 파일 이름 저장
							strcpy (chk_fname, filename);

							// 검출된 외국인등록번호의 수
							fDs[chk_fcnt].fgcnt++;

							// data 구조체에 저장
							strcpy (tmppath, filepath);
							strcpy (fDs[chk_fcnt].fpath, dirname (tmppath));
							strcpy (fDs[chk_fcnt].fname, filename);
							fDs [chk_fcnt].fsize = fsize;
							strcpy (fDs[chk_fcnt].fstat, "일반");
							strcpy (fDs[chk_fcnt].ftype, chk_ftype);
						}
					}
				}
			}
		}
		else
		{
			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_jnfg(); */


/*---------------------------------------------------------------------------*/
/* Driver Check                                              							  */
/*---------------------------------------------------------------------------*/
char match_regex_d (regex_t *r, const char *to_match, size_t fsize, char *filepath, char *filename)
{
	char *tmppath[MAX_PATH] = {0,};
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed.
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교
	while (1)
	{
		int nomatch = regexec (r, p, n_matches, m, 0);

		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
				    break;
				}

				// 운전면허 정규식 검사 통과
				if (i == 0)
				{
					int res = strcmp (chk_fname, filename); // 같은파일 = 0

					if (res != 0)
					{
						chk_fcnt++;
					}

					// 읽고있는중인 파일 이름 저장
					strcpy (chk_fname, filename);

					// 검출된 운전면허의 수
					fDs[chk_fcnt].dcnt++;

					// data 구조체에 저장
					strcpy (tmppath, filepath);
					strcpy (fDs[chk_fcnt].fpath, dirname (tmppath));
					strcpy (fDs[chk_fcnt].fname, filename);
					fDs [chk_fcnt].fsize = fsize;
					strcpy (fDs[chk_fcnt].fstat, "일반");
					strcpy (fDs[chk_fcnt].ftype, chk_ftype);
				}
			}
		}
		else
		{
			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_d(); */


/*---------------------------------------------------------------------------*/
/* Passport Check                                            							*/
/*---------------------------------------------------------------------------*/
char match_regex_p (regex_t *r, const char *to_match, size_t fsize, char *filepath, char *filename)
{
	char *tmppath[MAX_PATH] = {0,};
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed.
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교
	while (1)
	{
		int nomatch = regexec (r, p, n_matches, m, 0);

		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
					break;
				}

				// 운전면허 정규식 검사 통과
				if (i == 0)
				{
					int res = strcmp (chk_fname, filename); // 같은파일 = 0

					if (res != 0)
					{
						chk_fcnt++;
					}

					// 읽고있는중인 파일 이름 저장
					strcpy (chk_fname, filename);

					// 검출된 운전면허의 수
					fDs[chk_fcnt].pcnt++;

					// data 구조체에 저장
					strcpy (tmppath, filepath);
					strcpy (fDs[chk_fcnt].fpath, dirname (tmppath));
					strcpy (fDs[chk_fcnt].fname, filename);
					fDs [chk_fcnt].fsize = fsize;
					strcpy (fDs[chk_fcnt].fstat, "일반");
					strcpy (fDs[chk_fcnt].ftype, chk_ftype);
				}
			}
		}
		else
		{
			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_p(); */


/*---------------------------------------------------------------------------*/
/* Regex Check kind of data                                  					 */
/*---------------------------------------------------------------------------*/
void check_kind_of_data (const char *to_match, size_t fsize, char *filepath, char *filename)
{
	regex_t r;
	const char *regex_text;

	/*
	switch(data_flag) //나중에 민감정보 종류 선택 검출 할 때 사용
	{
		case 1:
			regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})"; //주민번호 정규식
			compile_regex(&r, regex_text); //정규식 컴파일
			match_regex_j(&r, to_match, filepath, file, buf);
			break;
		case 2:
			regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}"; //운전면허 정규식
			compile_regex(&r, regex_text); //정규식 컴파일
			//match_regex_d(&r, to_match, filepath, file, buf);
	}
	*/
	// 주민번호, 외국인등록번호 정규식 //
	regex_text = "[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-8][0-9]{6}";
	compile_regex(&r, regex_text); // 정규식 컴파일 //
	match_regex_jnfg(&r, to_match, fsize, filepath, filename);

	// 운전면허 정규식 //
	regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}";
	compile_regex (&r, regex_text); // 정규식 컴파일 //
	match_regex_d (&r, to_match, fsize, filepath, filename);

	// 여권번호 정규식 //
	regex_text = "[a-zA-Z]{1}[0-9]{8}";
	compile_regex (&r, regex_text); // 정규식 컴파일 //
	match_regex_p (&r, to_match, fsize, filepath, filename);
	return;
}
/* end of check_kind_of_data(); */
/***************** End of REGEX ******************/



/*---------------------------------------------------------------------------*/
/* Read Document																	  */
/*---------------------------------------------------------------------------*/
int func_docuparser (char *path[])
{
	/* 문서를 읽어 텍스트로 바꿔주는 상용 라이브러리 제거 */
	BXLog (DBG, "%-30s	[%s]\n", "INFO CHECK START", path);
	check_kind_of_data (tmpBuf, size,  path, basename (path));
	memset (chk_ftype, 0, strlen (chk_ftype));
	BXLog (DBG, "%-30s	[%s]\n", "INFO CHECK END", path);

    return 0;
}
/* end of func_docuparser(); */


/*---------------------------------------------------------------------------*/
/* Count File                                      				   							*/
/*---------------------------------------------------------------------------*/
int func_FileCount (gchar *path)
{
	FILE *fd;
	char  cmdbuf[MAX_PATH] = {0,};
	char  strbuf[1024] = {0,};
	char *pstr	= NULL;

	sprintf (cmdbuf, "find %s -type f | wc -l", path);

	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		BXLog (DBG, "%-30s	[%s][%d]\n", "FILE COUNT POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if ((pstr = fgets (strbuf, 1024, fd)) != NULL)
		{
			chk_afcnt = atoi (pstr);
		}
	}
	pclose (fd);

	return  0;
}
/* end of func_FileCount(); */


/*---------------------------------------------------------------------------*/
/* Scanning Folder and File                                      				   */
/*---------------------------------------------------------------------------*/
int func_Detect (gchar *path)
{
	DIR *dp = NULL;
	struct dirent *file = NULL;
	struct stat buf;
	char filepath[MAX_PATH];
	long lSize = 0;
	
	memset (message, 0x00, strlen (message));
	if (lstat (path, &buf) == -1)
	{
		//perror ("stat");
		BXLog (DBG, "%-30s	\n", "STAT ERROR");

		return -1;
	}

	// 폴더 //
	if (S_ISDIR (buf.st_mode))
	{
		if ((dp = opendir(path)) == NULL)
		{
			BXLog (DBG, "%-30s	[%s]\n", "BXR_FOLDER_OPEN_ERROR", path);
			return -1;
		}
		while ((file = readdir(dp)) != NULL)
		{
			// filepath에 현재 path넣기
			sprintf (filepath, "%s/%s", path, file->d_name);

			// .이거하고 ..이거 제외
			if ((!strcmp (file->d_name, ".")) || (!strcmp (file->d_name, "..")))
			{
				continue;
			}
			// 안에 폴더로 재귀함수
			func_Detect(filepath);
		}
		closedir (dp);
		BXLog (DBG, "%-30s	\n", "CLOSE DIR");
	}
	else if (S_ISREG (buf.st_mode))
	{
		int tmp_fcnt = 0;
		++chk_dcnt;
		++chk_ingfcnt;

		tmp_fcnt = chk_fcnt;
		func_GetTime();
		strncpy (fDs[chk_fcnt + 1].start, chk_worktime, 23);

		BXLog (DBG, "%-30s	\n", "READ FILE START", path);
		func_docuparser (path);
		BXLog (DBG, "%-30s	\n", "READ FILE END", path);

		percent = chk_ingfcnt / chk_afcnt;
		
		if ((tmp_fcnt + 1) == chk_fcnt)
		{
			func_GetTime();
			strncpy (fDs[chk_fcnt].end, chk_worktime, 23);

			if (chk_detect == 0)
			{
				fDs[chk_fcnt].round = 0;
			}
			else
			{
				fDs[chk_fcnt].round = pDs.psround;
			}
		}

		memset (message, 0x00, strlen (message));
		sprintf (message, "%.2f%% Complete", percent * 100.00);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), percent);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);

		while (gtk_events_pending()) 
				gtk_main_iteration (); 

		d_Refresh_ScrollWindow();

		chk_fname[0] = 0; // 초기화
	}

	return  0;
}
/* end of func_Detect(); */


/*---------------------------------------------------------------------------*/
/* Find Encrypt File                                      						 	     */
/*---------------------------------------------------------------------------*/
int func_Find_EncFile (gchar *path)
{
	DIR *dp = NULL;
	struct dirent *file = NULL;
	struct stat buf;
	char filepath[MAX_PATH];
	long lSize;
	char *tmp = NULL;
	
	memset (message, 0x00, strlen (message));
	if (lstat (path, &buf) == -1)
	{
		//perror ("stat");
		BXLog (DBG, "%-30s	\n", "STAT ERROR");

		return -1;
	}
	// 폴더 //
	if (S_ISDIR (buf.st_mode))
	{
		if ((dp = opendir(path)) == NULL)
		{
			BXLog (DBG, "%-30s	[%s]\n", "BXR_FOLDER_OPEN_ERROR", path);
			return -1;
		}
		while ((file = readdir(dp)) != NULL)
		{
			// filepath에 현재 path넣기
			sprintf (filepath, "%s/%s", path, file->d_name);

			// .이거하고 ..이거 제외
			if ((!strcmp (file->d_name, ".")) || (!strcmp (file->d_name, "..")))
			{
				continue;
			}
			// 안에 폴더로 재귀함수
			func_Find_EncFile (filepath);
		}
		closedir (dp);
		BXLog (DBG, "%-30s	\n", "CLOSE DIR");
	}
	else if (S_ISREG (buf.st_mode))
	{
		tmp = strstr (path, ".enc");
		if ((tmp != NULL) && (tmp[4] == NULL))
		{
			chk_dfcnt++;
			// data 구조체에 저장  
			func_GetTime();
			strcpy (dfDs[chk_dfcnt].start, chk_worktime);
			strcpy (dfDs[chk_dfcnt].ffpath, path);
			strcpy (dfDs[chk_dfcnt].fname, basename (path));
			strcpy (dfDs[chk_dfcnt].fpath, dirname (path));
			dfDs [chk_dfcnt].fsize = buf.st_size;
			strcpy (dfDs[chk_dfcnt].fstat, "암호화");
			
			func_GetTime();
			strcpy (dfDs[chk_dfcnt].end, chk_worktime);

			dec_Refresh_ScrollWindow();
		
			memset (message, 0x00, strlen (message));
			sprintf (message, "%.2f%% Complete", percent * 100.00);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dec_progressbar), percent);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dec_progressbar), message);

			while (gtk_events_pending ()) 
					gtk_main_iteration (); 
		}
	}

	return  0;
}
/* end of func_Find_EncFile(); */


/*---------------------------------------------------------------------------*/
/* gtk_dialog_modal                                          						*/
/*---------------------------------------------------------------------------*/
int func_gtk_dialog_modal (int type, GtkWidget *widget, char *message)
{
	GtkWidget *dialog, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;
	int	rtn = GTK_RESPONSE_REJECT;

	switch (type)
	{
		case 0 : // 로그 안찍히는 다이얼로그
			dialog = gtk_dialog_new_with_buttons ("Blue X-ray Plover", GTK_WINDOW (widget), flags, 
						("확인"), GTK_RESPONSE_ACCEPT, NULL);
			break;

		case 1 : // 확인, 취소 있는 다이얼로그
			dialog = gtk_dialog_new_with_buttons ("Blue X-ray Plover", GTK_WINDOW (widget), flags,
						("확인"), GTK_RESPONSE_ACCEPT, 
						("닫기"), GTK_RESPONSE_REJECT, NULL);
			break;
			
		case 2 : // 로그 찍히는 다이얼로그
			dialog = gtk_dialog_new_with_buttons ("Blue X-ray Plover", GTK_WINDOW (widget), flags, 
						("확인"), GTK_RESPONSE_ACCEPT, NULL);
		
		default :
			break;
	}

	label = gtk_label_new (message);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all (dialog);
	
	rtn = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return (rtn);
}
/* end of fgtk_dialog_modal(); */


/*---------------------------------------------------------------------------*/
/* Move File                                               									 */
/*---------------------------------------------------------------------------*/
void func_Move()
{
	char newfpath[MAX_PATH] = {0,};
	int fl = 0;

	switch (chk_fmanage)
	{
		case 0: // 파일 이동 폴더로 이동
			sprintf (newfpath, "%s/%s", dpDs.movepath, sfDs.fname);
			fl = rename (sfDs.ffpath, newfpath);
			
			break;

		case 1: // 파일 암호화 폴더로 이동
			sprintf (newfpath, "%s/%s.enc", dpDs.encpath, sdfDs.fname);
			fl = rename (sdfDs.ffpath, newfpath);

			break;

		case 2: // 파일 복호화 폴더로 이동
			sprintf (newfpath, "%s/%s.dec", dpDs.decpath, sdfDs.fname);
			fl = rename (sdfDs.ffpath, newfpath);

			break;

		case 3: // 새로운 정책 받을 시 기존 정책 이름 변경
			sprintf (newfpath, "%s%s", home_path, LASTPOLICY);
			fl = rename (policy_path, newfpath);

			break;

	}

	if (fl == -1)
	{
		BXLog (DBG, "%-30s	[%d][%s]\n", "BXR_FILE_MOVE_ERR", chk_fmanage, strerror (errno));

		return -1;
	}
	else
	{
		BXLog (DBG, "%-30s	[%d]\n", "BXR_FILE_MOVE_OK", chk_fmanage);
	}

	return;
}
/* end of func_Move(); */


/*---------------------------------------------------------------------------*/
/* Delete File                                               								 */
/*---------------------------------------------------------------------------*/
int func_file_eraser (int type)
{
	FILE *fp;
	int mode = R_OK | W_OK;
	char MsgTmp[5] = {0,};
	gdouble size = 0.0;
	char *msize;
	char newfpath[MAX_PATH] = {0,};
	char tempfpath[MAX_PATH] = {0,};
	char randstr[24] = {0,};
	int fl = 0;

	percent = 0.0;
	if (access (sfDs.ffpath, mode) != 0 )
	{
		func_gtk_dialog_modal (0, window, "\n    파일이 삭제 가능한 상태가 아닙니다.    \n");
		return -1;
	}
	else
		{
			msize = malloc (ERASER_SIZE);
			fp = fopen (sfDs.ffpath, "w");

			while (gtk_events_pending ()) 
					gtk_main_iteration (); 

			memset (message, 0x00, strlen (message));
			sprintf (message, "%.2f%% Complete", percent);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), percent / 100.00);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);

			for( int i = 0 ; i < type ; i++ )
			{
				size = 0.0;

				while (size < sfDs.fsize)
				{   
					switch(i)
					{
						case 0 :
							MsgTmp[0] = 'A';
							memset( msize, MsgTmp[0], ERASER_SIZE );
							break;
						case 1 :
							MsgTmp[0] = '^';
							memset( msize, MsgTmp[0], ERASER_SIZE );
							break;
						case 2 :
							srand(time(NULL));
							if( size < ERASER_SIZE )
								for( int j=0 ; j < ERASER_SIZE ; j++ )
									msize[j] = 'A' + (random() % 26);
							break;
						case 3 :
							MsgTmp[0] = 'Z';
							memset( msize, MsgTmp[0], ERASER_SIZE );
							break;
						case 4 :
							MsgTmp[0] = 'A';
							memset( msize, MsgTmp[0], ERASER_SIZE );
							break;
						case 5 :
							MsgTmp[0] = '^';
							memset( msize, MsgTmp[0], ERASER_SIZE );
							break;
						case 6 :
							srand(time(NULL));
							if( size < ERASER_SIZE )
								for( int j=0 ; j < ERASER_SIZE ; j++ )
									msize[j] = 'A' + (random() % 26);
							break;
						default :
							break;

					}

					while (gtk_events_pending ()) 
							gtk_main_iteration (); 

					fwrite( msize, 1, ((sfDs.fsize/size))>0?ERASER_SIZE:(sfDs.fsize%ERASER_SIZE), fp);
					size += ERASER_SIZE;
					percent = (size+(sfDs.fsize*i))/(sfDs.fsize*type)*100.00;
					if( (int)size % PROGRESS_SIZE == 0 )
					{
						memset( message, 0x00, strlen (message));
						sprintf( message, "%.2f%% Complete", percent);
					//				gchar *message = g_strdup_printf ("%.0f%% Complete", percent);
						gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), percent / 100.00);
						gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);
					}
				}

				fseek( fp, 0L, SEEK_SET );
			}

			sprintf( message, "%.2f%% Complete", 100.00);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), 100.00);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);
	
			fclose(fp);
			free(msize);
		}

	for (int i = 0; i < 24; i++ )
	{
		randstr[i] = 'a' + rand() % 26;
	}

	sprintf (tempfpath, "%s", sfDs.ffpath);
	sprintf (newfpath, "%s/%s", home_path, randstr);
	fl = rename (sfDs.ffpath, newfpath);

	if (fl == -1)
	{
		if (errno == 2)
		{
			BXLog (DBG, "%-30s	[%s]\n", "BXR_FILE_OPEN_ERR", sfDs.ffpath);
		}
		BXLog (DBG, "%-30s	[%d]\n", "BXR_FILE_ERROR", errno);
		return -1;
	}
	else
	{
		BXLog (DBG, "%-30s	[%d]\n", "RANDOMIZE FILE NAME");
	}

	remove (newfpath);

	func_gtk_dialog_modal (0, window, "\n    파일 완전 삭제가 완료되었습니다.    \n");

	return (TRUE);
}
/* end of func_file_eraser(); */


/*---------------------------------------------------------------------------*/
/* Encryption                       									                    */
/*---------------------------------------------------------------------------*/
void func_encrypt()
{
	unsigned char *buffer = NULL;
	int encresult = 0;

	buffer = (unsigned char *) malloc (sizeof (char) *sfDs.fsize);
	func_get_data (buffer, sfDs.fsize, sfDs.ffpath);

	BXLog (DBG, "%-30s	[%s]\n", "START FILE ENCRYPT", sfDs.ffpath);

	encresult = enc_aria256 (buffer, sfDs.fsize , ARIA_KEY, sfDs.fname, dpDs.encpath);

	if (encresult != 0)
	{
		BXLog (DBG, "%-30s	[%d]\n", "ENCRYPT RESULT", encresult);
	}
	
	BXLog (DBG, "%-30s	[%s]\n", "END FILE ENCRYPT", sfDs.fname);
	//remove(sfDs.fpath);

	return;
}
/* end of func_encrypt(); */


/*---------------------------------------------------------------------------*/
/* Decryption                       									                    */
/*---------------------------------------------------------------------------*/
void func_decrypt()
{
	unsigned char *buffer = NULL;
	int decresult = 0;

	buffer = (unsigned char *) malloc (sizeof (char) *sdfDs.fsize);
	func_get_data (buffer, sdfDs.fsize, sdfDs.ffpath);

	BXLog (DBG, "%-30s	[%s]\n", "START FILE DECRYPT", sdfDs.ffpath);

	decresult = dec_aria256 (buffer, sdfDs.fsize , ARIA_KEY, sdfDs.fname, dpDs.decpath);


	if (decresult != 0)
	{
		BXLog (DBG, "%-30s	[%d][%s]\n", "DECRYPT RESULT", decresult, MC_GetErrorString (decresult));
	}

	BXLog (DBG, "%-30s	[%s]\n", "END FILE DECRYPT", sdfDs.fname);
	//remove(sfDs.fpath);

	return;
}
/* end of func_decrypt(); */


/*---------------------------------------------------------------------------*/
/* Vulnerability Status Check                       							   */
/*---------------------------------------------------------------------------*/
void func_Vulnerability_stat (int chk_vstat, int chk_vnum)
{
	switch (chk_vstat)
	{
		case 0:
			BXLog (DBG, "<SAFE> V_ITEM%d\n", chk_vnum);
			strcpy( viDs[chk_vnum].result, "성공");
			strcpy (viDs[chk_vnum].stat, "안전");

			break;
		
		case 1:
			BXLog (DBG, "<IMPOSSIBLE> V_ITEM%d\n", chk_vnum);
			strcpy( viDs[chk_vnum].result, "미수행");
			strcpy (viDs[chk_vnum].stat, "미수행");
			
			break;

		case 2:
			BXLog (DBG, "<WARN> V_ITEM%d\n", chk_vnum);
			strcpy( viDs[chk_vnum].result, "성공");
			strcpy (viDs[chk_vnum].stat, "위험");
			
			break;
	}

	return;
}
/* end of func_decrypt(); */


/********** Start of Vulnerability Item Function **********/
/*---------------------------------------------------------------------------*/
/* 취약점 검사 항목 그룹			                 		   							*/
/*---------------------------------------------------------------------------*/
void func_VulnerabilityGroup (int item)
{
	struct  timeval t;
	int stime = 0, etime = 0;

	switch (item)
	{
		case 0:
			func_Vulnerability_item0();

			break;
		
		case 1:
			func_Vulnerability_item1();

			break;

		case 2:
			func_Vulnerability_item2();

			break;
		
		case 3:
			func_Vulnerability_item3();

			break;

		case 4:
			func_Vulnerability_item4();

			break;
		
		case 5:
			func_Vulnerability_item5();

			break;

		case 6:
			func_Vulnerability_item6();

			break;

		case 7:
			func_Vulnerability_item7();

			break;
		
		case 8:
			func_Vulnerability_item8();

			break;

		case 9:
			func_Vulnerability_item9();

			break;
		
		default:
			BXLog (DBG, "%-30s	\n", "WRONG V_ITEM NUMBER");
			
			break;
	}
}/* end of func_VulnerabilityGroup(); */


/*---------------------------------------------------------------------------*/
/* 계정 원격접속 제한				                 		   							*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item0()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	//  /etc/pam.d/login
	if (access ("/etc/pam.d/login", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/pam.d/login]\n", "NOT EXIST FILE/AUTHORITY");
		
		return 0;
	}
	fd = fopen ("/etc/pam.d/login", "r");
	
	while (feof (fd) == 0)
	{
		pstr = fgets (strbuf, sizeof (strbuf), fd);

		if (pstr == 0)  // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
		{
			break;
		}

		if (((pstrstr = strchr (pstr, '#')) == NULL)  && ((pstrstr = strstr (pstr, "auth")) != NULL) && ((pstrstr = strstr (pstr, "pam_securetty.so")) != NULL)) 
		{
			chk_vsafe++;
			BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/pam.d/login", pstr);
			//printf ("%s\n", pstr); // 여기에 찍히는 설정 값들이 고급설정으로 되어있으면 자세하게  하나하나 따져야함
		}
	}
	fclose (fd);

	//  /etc/ssh/sshd_config
	if (access ("/etc/ssh/sshd_config", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/ssh/sshd_config\n", "NOT EXIST FILE/AUTHORITY");

		return 0;
	}

	fd = fopen ("/etc/ssh/sshd_config", "r");

	while (feof (fd) == 0)
	{
		pstr = fgets (strbuf, sizeof (strbuf), fd);

		if (pstr == 0)
		{
			break;
		}

		if (((pstrstr = strchr (pstr, '#') == NULL)) && ((pstrstr = strstr (pstr, "PermitRootLogin")) != NULL) && ((pstrstr = strstr (pstr, "no")) != NULL))
		{
			chk_vsafe++;
			BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/ssh/sshd_config", pstr);
		}
	}
	fclose (fd); 

	//  /etc/securetty
	if (access ("/etc/securetty", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/securetty\n", "NOT EXIST FILE/AUTHORITY");

		return 0;
	}
	fd = fopen ("/etc/securetty", "r");

	while (feof (fd) == 0)
	{
		pstr = fgets (strbuf, sizeof (strbuf), fd);

		if (pstr == 0)
		{
			break;
		}

		if (((pstrstr = strchr (pstr, '#')) == NULL) && ((pstrstr = strstr (pstr, "pts")) != NULL))
		{
			chk_vvur++;
			BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/securetty", pstr);
		}
	}
	fclose (fd);

	if ((chk_vsafe == 2) && (chk_vvur != 1))
	{
		chk_vstat = 0;
	}
	else if ((chk_vimpossible  > 0) && (chk_vimpossible < 4))
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 2;
	}

	func_Vulnerability_stat (chk_vstat, 0);
}
/* end of func_Vulnerability_item0(); */


/*---------------------------------------------------------------------------*/
/* 패스워드 복잡성 설정				                 		   						   */
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item1()
{
	FILE *fd;
	char * pstrstr = NULL;
	char *pstr	= NULL;
	char  strbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	//  /etc/security/pwquality.conf
	if (access ("/etc/security/pwquality.conf", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/security/pwquality.conf]\n", "NOT EXIST FILE/AUTHORITY");
	}
	else
	{
		fd = fopen ("/etc/security/pwquality.conf", "r");
	
		while (feof (fd) == 0)
		{
			pstr = fgets (strbuf, sizeof (strbuf), fd);

			if (pstr == 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
			{
				break;
			}
			
			if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "difok") != NULL) && ((pstrstr = strstr (pstr, "N")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "minlen") != NULL) && ((pstrstr = strstr (pstr, "8")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL)  && (pstrstr = strstr (pstr, "dcredit") != NULL) && ((pstrstr = strstr (pstr, "-1")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL)  && (pstrstr = strstr (pstr, "ucredit") != NULL) && ((pstrstr = strstr (pstr, "-1")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL) && ((pstrstr = strstr (pstr, "lcredit") != NULL)) && ((pstrstr = strstr (pstr, "-1")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL) && ((pstrstr = strstr (pstr, "ocredit")) != NULL) && ((pstrstr = strstr (pstr, "-1")) != NULL))
			{
				chk_vsafe++;
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/security/pwquality.conf", pstr);
			}
		}

		fclose (fd);
	}
	
	if (chk_vsafe == 6)
	{
		chk_vstat = 0;
	}
	else if (chk_vimpossible == 1)
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 2;
	}

	func_Vulnerability_stat (chk_vstat, 1);
}
/* end of func_Vulnerability_item1(); */


/*---------------------------------------------------------------------------*/
/* 계정 잠금 임계값 설정				                 		   					   */
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item2()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	//  /etc/pam.d/common-auth
	if (access ("/etc/pam.d/common-auth", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/pam.d/login]\n", "NOT EXIST FILE/AUTHORITY");
		
		return 0;
	}
	fd = fopen ("/etc/pam.d/common-auth", "r");
	
	while (feof (fd) == 0)
	{
		pstr = fgets (strbuf, sizeof (strbuf), fd);

		if (pstr == 0)
		{
			break;
		}

		if (((pstrstr = strchr (pstr, '#')) == NULL) && ((pstrstr = strstr (pstr, "auth")) != NULL) && ((pstrstr = strstr (pstr, "required")) != NULL) && ((pstrstr = strstr (pstr, "pam_tally2.so")) != NULL) && ((pstrstr = strstr (pstr, "no_magic_root")) != NULL))
		{
			if ((pstrstr = strstr (pstr, "deny=")) != NULL)
			{
				if ((pstrstr[5] >= 48) && (pstrstr[5] <= 57) && pstrstr[6] == 32)
				{
					chk_vsafe++;
					BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/pam.d/common-auth", pstrstr);
				}
			}
		}
		else if (((pstrstr = strchr (pstr, '#')) == NULL) && ((pstrstr = strstr (pstr, "account")) != NULL) && ((pstrstr = strstr (pstr, "required")) != NULL) && ((pstrstr = strstr (pstr, "pam_tally2.so")) != NULL) && ((pstrstr = strstr (pstr, "no_magic_root")) != NULL) && ((pstrstr = strstr (pstr, "reset")) != NULL))
		{
			chk_vsafe++;
			BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/pam.d/common-auth", pstr);
		}
	}
	fclose (fd);

	if (chk_vsafe == 2)
	{
		chk_vstat = 0;
	}
	else if (chk_vimpossible == 1)
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 2;
	}

	func_Vulnerability_stat (chk_vstat, 2);
}
/* end of func_Vulnerability_item2(); */


/*---------------------------------------------------------------------------*/
/* 패스워드 파일 보호				                 		   							*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item3()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	//  /etc/shadow, /etc/passwd
	if (access ("/etc/shadow", F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/shadow]\n", "DOES NOT EXIST FILE");
	}
	else if (access ("/etc/passwd", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/passwd]\n", "DOES NOT EXIST FILE/AUTHORITY");
	}
	else
	{
		fd = fopen ("/etc/passwd", "r");
	
		while (feof (fd) == 0)
		{
			pstr = fgets (strbuf, sizeof (strbuf), fd);

			if (pstr == 0)  // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
			{
				break;
			}

			if (((pstrstr = strchr (pstr, '#')) == NULL)  && ((pstrstr = strstr (pstr, ":x:")) == NULL)) // 두번째 필드 x값 의미: /etc/shadow파일에 비밀번호가 암호화되어 저장돼있다는 의미.
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/passwd", pstr);
			}
		}

		fclose (fd);
	}

	if (chk_vvur > 0)
	{
		chk_vstat = 2;
	}
	else if (chk_vimpossible == 2)
	{
		chk_vstat = 1;
	}
	else if ((chk_vimpossible == 1) && (chk_vvur == 0))
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 0;
	}

	func_Vulnerability_stat (chk_vstat, 3);
}
/* end of func_Vulnerability_item3(); */


/*---------------------------------------------------------------------------*/
/* Root 홈, 패스 디렉터리 권한 및 패스 설정			 						*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item4()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[256] = {0,};
	char  cmdbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	sprintf (cmdbuf, "echo $PATH");
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM4 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if ((pstr = fgets (strbuf, 1024, fd)) != NULL)
		{
			if (((pstrstr = strstr (pstr, ".:")) != NULL)) 
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%s]\n", "<WARN> V_ITEM4", pstr);
			}
			else if ((pstr[0] == ':') && (pstr[1] == ':') )
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%s]\n", "<WARN> V_ITEM4", pstr);
			}
		}
	}
	pclose (fd);

	if (chk_vvur > 0)
	{
		chk_vstat = 2;
	}
	else if (chk_vimpossible == 1)
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 0;
	}

	func_Vulnerability_stat (chk_vstat, 4);
}
/* end of func_Vulnerability_item4(); */


/*---------------------------------------------------------------------------*/
/* 파일 및 디렉터리 소유자 설정		               		   						   */
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item5()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[1024] = {0,};
	char  cmdbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	sprintf (cmdbuf, "find / -nogroup -print 2> /dev/null");
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM5 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 1024, fd) != NULL)
		{
			chk_vvur++;
			BXLog (DBG, "%-30s	[%s]\n", "<WARN> V_ITEM5 NO GROUP", strbuf);
		}
		else
		{
			BXLog (DBG, "%-30s	\n", "<SAFE> V_ITEM5 NO GROUP NOT EXIST");
		}
	}
	pclose (fd);
	

	sprintf (cmdbuf, "find / -nouser -print 2> /dev/null");
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM5 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 1024, fd) != NULL)
		{
			chk_vvur++;
			BXLog (DBG, "%-30s	[%s]\n", "<WARN> V_ITEM5 NO USER", strbuf);
		}
		else
		{
			BXLog (DBG, "%-30s	\n", "<SAFE> V_ITEM5 NO USER NOT EXIST");
		}
	}
	pclose (fd);

	if (chk_vvur > 0)
	{
		chk_vstat = 2;
	}
	else if (chk_vimpossible == 2)
	{
		chk_vstat = 1;
	}
	else if ((chk_vimpossible == 1) && (chk_vvur == 0))
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 0;
	}

	func_Vulnerability_stat (chk_vstat, 5);
}
/* end of func_Vulnerability_item5(); */


/*---------------------------------------------------------------------------*/
/* 로그인 패스워드 안전성 여부 점검	          		   							*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item6()
{
	FILE *fd;
	char *pstrstr = NULL;
	char *pstr	= NULL;
	char  strbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	func_Vulnerability_item1();
	if (strcmp (viDs[1].stat, "안전") == 0)
	{
		//chk_vscore++;
		chk_vstat = 0;
	}
	else if (strcmp (viDs[1].stat, "위험") == 0)
	{
		chk_vstat = 2;
	}
	else //(strcmp (viDs[1].stat, "미수행") == 0)
	{
		chk_vstat = 1;
	}

	// 추후에 들어갈 기능, chk_vsafe/chk_vvur/chk_vimpossible계산 로직 다시넣어줘야함
	/* //  /etc/login.defs
	if (access ("/etc/login.defs", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/login.defs]\n", "NOT EXIST FILE/AUTHORITY");
	}
	else
	{
		fd = fopen ("/etc/login.defs", "r");
		while (feof (fd) == 0)
		{
			pstr = fgets (strbuf, sizeof (strbuf), fd);

			if (pstr == 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
			{
				break;
			}
			
			if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "PASS_MAX_DAYS") != NULL))
			{
				pstrstr = NULL;
				sprintf (&pstrstr, "%s", &pstr[14]);
				if (atoi (&pstrstr) >= 90)
				{
					BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/login.defs", pstr);
				}
				else
				{
					BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/login.defs", pstr);
					chk_vscore++;
				}
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "PASS_MIN_DAYS") != NULL))
			{
				pstrstr = NULL;
				sprintf (&pstrstr, "%s", &pstr[14]);
				if (atoi (&pstrstr) < 1)
				{
					BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/login.defs", pstr);
				}
				else
				{
					BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/login.defs", pstr);
					chk_vscore++;
				}
			}
			else if (((pstrstr = strchr (pstr, '#')) == NULL)  && (pstrstr = strstr (pstr, "PASS_MIN_LEN") != NULL))
			{
				pstrstr = NULL;
				sprintf (&pstrstr, "%s", &pstr[13]);
				if (atoi (&pstrstr) < 9)
				{
					BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/login.defs", pstr);
				}
				else
				{
					BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/login.defs", pstr);
					chk_vscore++;
				}
			}
		}

		fclose (fd);
	}
	
	//  /etc/pam.d/common-auth
	if (access ("/etc/pam.d/common-auth", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/pam.d/common-auth]\n", "NOT EXIST FILE/AUTHORITY");
	}
	else
	{
		fd = fopen ("/etc/pam.d/common-auth", "r");
	
		while (feof (fd) == 0)
		{
			pstr = fgets (strbuf, sizeof (strbuf), fd);

			if (pstr == 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
			{
				break;
			}

			if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "password") != NULL) && ((pstrstr = strstr (pstr, "required")) != NULL) && ((pstrstr = strstr (pstr, "pam_pwquality.so")) != NULL) && ((pstrstr = strstr (pstr, "enforce_for_root")) != NULL))
			{
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/pam.d/common-auth", pstr);
				chk_vscore++;
			}
		}

		fclose (fd);
	}

	//  /etc/pam.d/common-password
	if (access ("/etc/pam.d/common-password", R_OK | F_OK) != 0)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[/etc/pam.d/common-password]\n", "NOT EXIST FILE/AUTHORITY");
	}
	else
	{
		fd = fopen ("/etc/pam.d/common-password", "r");
	
		while (feof (fd) == 0)
		{
			pstr = fgets (strbuf, sizeof (strbuf), fd);

			if (pstr == 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
			{
				break;
			}

			if (((pstrstr = strchr (pstr, '#')) == NULL) && (pstrstr = strstr (pstr, "password") != NULL) && ((pstrstr = strstr (pstr, "requisite")) != NULL) && ((pstrstr = strstr (pstr, "pam_pwquality.so")) != NULL) && ((pstrstr = strstr (pstr, "retry=")) != NULL))
			{
				pstrstr = NULL;
				sprintf (&pstrstr, "%s", &pstr[44]);

				if (atoi (&pstrstr) >= 3) // "retry="의 값
				{
					BXLog (DBG, "%-30s	[%s]\n", "<SAFE> /etc/pam.d/common-password", pstr);
					chk_vscore++;
				}
				else
				{
					BXLog (DBG, "%-30s	[%s]\n", "<WARN> /etc/pam.d/common-password", pstr);
				}
			}
		}

		fclose (fd);
	}

	if (chk_vscore == 0)
	{
		chk_vstat = 0;
	}
	else if ((chk_vscore > 0) && (chk_vscore < 10))
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 2;
	} */

	func_Vulnerability_stat (chk_vstat, 6);
}
/* end of func_Vulnerability_item6(); */


/*---------------------------------------------------------------------------*/
/* 로그인 패스워드의 분기 1회 이상 변경 여부 점검						*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item7() // 다시짜자.., 단말시간 서버에서 받아와서 동기화 해야함(정책받아올때 받아오자)
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[1024] = {0,};
	char  strbuf2[1024] = {0,};
	char  cmdbuf[256] = {0,};
	int lctime = 0, pwdftime = 0, restime = 0;
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	//int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	pstr = getlogin();
	sprintf (cmdbuf, "passwd -S %s | awk {'print $3'}", pstr);
	

	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vstat = 1;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM7 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 1024, fd) != NULL)
		{
			memset (cmdbuf, 0x00, sizeof (cmdbuf));
			strncpy (strbuf2, strbuf, 10);
			sprintf (cmdbuf, "date -d \"%s\" +%%s", strbuf2);
			memset (strbuf, 0x00, sizeof (strbuf));
			if ((fd = popen (cmdbuf, "r")) == NULL)
			{
				chk_vstat = 1;
				BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM7 POPEN ERROR", strerror (errno), errno);
			}
			else
			{
				if (fgets (strbuf, 1024, fd) != NULL)
				{
					pwdftime = atoi (strbuf);
					memset (cmdbuf, 0x00, sizeof (cmdbuf));
					memset (strbuf, 0x00, sizeof (strbuf));
					sprintf (cmdbuf, "date -d \"\" +%%s");
	
					if ((fd = popen (cmdbuf, "r")) == NULL)
					{
						chk_vstat = 1;
						BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM7 POPEN ERROR", strerror (errno), errno);
					}
					else
					{
						if (fgets (strbuf, 1024, fd) != NULL)
						{
							lctime = atoi (strbuf);
							restime = ((lctime - pwdftime) / 86400);
							if (restime > 90)
							{
								chk_vstat = 2;
								BXLog (DBG, "%-30s	[%d]\n", "<WARN> V_ITEM7 HAVE TO CHANGE PASSWORD", restime);
							}
							else
							{
								BXLog (DBG, "%-30s	[%d]\n", "<SAFE> V_ITEM7", restime);
							}
						}
					}	
				}
			}
		}
	}
	
	pclose (fd);
	func_Vulnerability_stat (chk_vstat, 7);
}
/* end of func_Vulnerability_item7(); */


/*---------------------------------------------------------------------------*/
/* 화면보호기 설정 여부 점검	                 		   							*/
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item8()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[256] = {0,};
	char  cmdbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;


// 화면보호기 설정 확인
	sprintf (cmdbuf, "gsettings get org.gnome.desktop.screensaver lock-enabled");
	
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM8 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 256, fd) != NULL)
		{
			if  (strncmp (strbuf, "true", 4) == 0)
			{
				BXLog (DBG, "%-30s	[%s]\n", "<SAFE> V_ITEM8 SCREEN SAFE", strbuf);
			}
			else
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%s]\n", "<WARN> V_ITEM8 SCREEN SAFE", strbuf);
			}
		}
	}
	
	pclose (fd);


	// 화면보호기 시간 확인
	sprintf (cmdbuf, "gsettings get org.gnome.desktop.session idle-delay  | awk {'print $2'}");
	
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM8 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if (fgets (strbuf, 256, fd) != NULL)
		{
			if  ((atoi (strbuf) > 0) && (atoi (strbuf) < 600))
			{
				BXLog (DBG, "%-30s	[%d]\n", "<SAFE> V_ITEM8 SCREEN SAFE TIME", atoi (strbuf));
			}
			else
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%d]\n", "<WARN> V_ITEM8 SCREEN SAFE TIME", atoi (strbuf));
			}
		}
	}
	
	pclose (fd);

	if (chk_vvur > 0)
	{
		chk_vstat = 2;
	}
	else if (chk_vimpossible == 2)
	{
		chk_vstat = 1;
	}
	else if ((chk_vimpossible == 1) && (chk_vvur == 0))
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 0;
	}

	func_Vulnerability_stat (chk_vstat, 8);
}
/* end of func_Vulnerability_item8(); */


/*---------------------------------------------------------------------------*/
/* 사용자 공유 폴더 설정 여부 점검             		   							  */
/*---------------------------------------------------------------------------*/
void func_Vulnerability_item9()
{
	FILE *fd;
	char *pstr	= NULL, *pstrstr = NULL;
	char  strbuf[1024] = {0,};
	char  cmdbuf[256] = {0,};
	int chk_vstat = 0; // 0: 안전, 1: 미수행, 2: 위험
	int chk_vsafe = 0,  chk_vimpossible= 0, chk_vvur = 0;

	sprintf (cmdbuf, "ps -ef | grep nfsd");
	if ((fd = popen (cmdbuf, "r")) == NULL)
	{
		chk_vimpossible++;
		BXLog (DBG, "%-30s	[%s][%d]\n", "V_ITEM9 POPEN ERROR", strerror (errno), errno);
	}
	else
	{
		if ((pstr = fgets (strbuf, 1024, fd)) != NULL)
		{
			if ((pstrstr = strstr (pstr, "grep")) == NULL)
			{
				chk_vvur++;
				BXLog (DBG, "%-30s	[%s]\n", "<WARN>  V_ITEM9 NFS", pstr);
			}
		}
	}
	pclose (fd);

	if (chk_vvur > 0)
	{
		chk_vstat = 2;
	}
	else if (chk_vimpossible == 1)
	{
		chk_vstat = 1;
	}
	else
	{
		chk_vstat = 0;
	}

	func_Vulnerability_stat (chk_vstat, 9);
}
/* end of func_Vulnerability_item9(); */
/********** End of Vulnerability Item Function **********/

/*---------------------------------------------------------------------------*/
/* Vulnerability Item Code					                                       */
/*---------------------------------------------------------------------------*/
void func_VulnerabilityCode (int vnum)
{
	switch (vnum) // C는 SECU의 C
	{
		case 0:
			strcpy (viDs[vnum].item, "C001");

			break;

		case 1:
			strcpy (viDs[vnum].item, "C002");

			break;

		case 2:
			strcpy (viDs[vnum].item, "C003");

			break;

		case 3:
			strcpy (viDs[vnum].item, "C004");

			break;

		case 4:
			strcpy (viDs[vnum].item, "C005");

			break;

		case 5:
			strcpy (viDs[vnum].item, "C006");

			break;

		case 6:
			strcpy (viDs[vnum].item, "C007");

			break;

		case 7:
			strcpy (viDs[vnum].item, "C008");

			break;

		case 8:
			strcpy (viDs[vnum].item, "C009");

			break;

		case 9:
			strcpy (viDs[vnum].item, "C010");

			break;
	}
} ;
/* end of func_VulnerabilityCode(); function */

/*---------------------------------------------------------------------------*/
/* Vulnerability Detect				                 		   							*/
/*---------------------------------------------------------------------------*/
void func_VulnerabilityDetect()
{
	struct  timeval t;
	int stime = 0, etime = 0;

	while (gtk_events_pending())
		gtk_main_iteration ();

	memset (&vDs, 0x00, sizeof (struct _Vdata_Storage));
	memset (message, 0x00, strlen (message));
	percent = 0;
	
	sprintf (message, "%.2f%% Complete", percent);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (v_progressbar), percent/100.00);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (v_progressbar), message);

	func_GetTime();
	strcpy (vDs.start, chk_worktime);

	if (chk_vdetect == 0)
	{
		for (int i = 0; i <= 9; i++)
		{
			if (strncmp (dpDs.wiPd[i].use, "Y", 1) == 0) // 수동검사 정책
			{
				while (gtk_events_pending())
					gtk_main_iteration ();

				gettimeofday (&t, NULL);
				localtime (&t.tv_sec);
				stime = t.tv_sec;

				vDs.item++;
				func_VulnerabilityCode (i);
				func_GetTime();
				strcpy (viDs[i].start, chk_worktime);
				func_VulnerabilityGroup (i);
				func_GetTime();
				strcpy (viDs[i].end, chk_worktime);
				viDs[i].round = 0;
				BXDB_Push (i, chk_vdetect + 2);
				percent += 10;
				memset (message, 0x00, strlen (message));
				sprintf (message, "%.2f%% Complete", percent);
				gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (v_progressbar), percent/100.00);
				gtk_progress_bar_set_text (GTK_PROGRESS_BAR (v_progressbar), message);

				if (strcmp (viDs[i].stat, "위험") == 0)
				{
					vDs.vurnelability++;
				}
				else if (strcmp (viDs[i].stat, "안전") == 0)
				{
					vDs.safe++;
				}
				else
				{
					vDs.imposiible++;
				}
				
				while (1) // 1 후 넘어감
				{
					gettimeofday (&t, NULL);
					localtime (&t.tv_sec);
					etime = t.tv_sec;
					if ((etime - stime) >= 1)
					{
						break;
					}
				}

				v_Refresh_ScrollWindow();

				while (gtk_events_pending())
					gtk_main_iteration ();
			}
		}
	}
	else if (chk_vdetect == 1)
	{
		for (int i = 0; i <= 9; i++)
		{
			while (gtk_events_pending())
					gtk_main_iteration ();

			if (strncmp (pDs.wiPd[i].use, "Y", 1) == 0) // 정기검사 정책
			{
				while (gtk_events_pending())
					gtk_main_iteration ();

				gettimeofday (&t, NULL);
				localtime (&t.tv_sec);
				stime = t.tv_sec;

				vDs.item++;
				func_VulnerabilityCode (i);
				func_GetTime();
				strcpy (viDs[i].start, chk_worktime);
				func_VulnerabilityGroup (i);
				func_GetTime();
				strcpy (viDs[i].end, chk_worktime);
				viDs[i].round = pDs.pwround;
				BXDB_Push (i, chk_vdetect + 2);
				percent += 10;
				memset (message, 0x00, strlen (message));
				sprintf (message, "%.2f%% Complete", percent);
				gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (v_progressbar), percent/100.00);
				gtk_progress_bar_set_text (GTK_PROGRESS_BAR (v_progressbar), message);

				if (strcmp (viDs[i].stat, "위험") == 0)
				{
					vDs.vurnelability++;
				}
				else if (strcmp (viDs[i].stat, "안전") == 0)
				{
					vDs.safe++;
				}
				else
				{
					vDs.imposiible++;
				}
				
				while (1) // 1 후 넘어감
				{
					gettimeofday (&t, NULL);
					localtime (&t.tv_sec);
					etime = t.tv_sec;
					if ((etime - stime) >= 1)
					{
						break;
					}
				}

				v_Refresh_ScrollWindow();

				while (gtk_events_pending())
					gtk_main_iteration ();
			}
		}
	}

	func_GetTime();
	strcpy (vDs.end, chk_worktime);
}
/* end of func_VulnerabilityDetect(); function */


/*---------------------------------------------------------------------------*/
/* BXLog start                                               							   */
/*---------------------------------------------------------------------------*/
int BXLog (const char *logfile, int logflag, int logline, const char *fmt, ...)
{
	int fd, len;
	struct	timeval	t;
	struct tm *tm;
	static char fname[MAX_NAME];
	static char sTmp[1024 * 20], sFlg[5];

	va_list ap;

	//if( LOGMODE < logflag ) return 0;

	switch (logflag)
	{
		case	0 :
		case	1 :
			sprintf (sFlg, "E");
			break;

		case	2 :
			sprintf (sFlg, "I");
			break;

		case	3 :
			sprintf (sFlg, "W");
			break;

		case	4 :
		default   :
#ifndef _BXDBG
			return 0;
#endif
			sprintf( sFlg, "D" );

			break;
	}

	memset (sTmp, 0x00, sizeof (sTmp));
	gettimeofday (&t, NULL);
	tm = localtime (&t.tv_sec);

	/* [HHMMSS ssssss flag __LINE__] */
	len = sprintf (sTmp, "[%5d:%08x/%02d%02d%02d %06ld/%s:%4d:%4d] ",
			getpid(), (unsigned int) pthread_self(),
			tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec, 
			sFlg, errno, logline);

	va_start (ap, fmt);
	vsprintf ((char *) &sTmp[len], fmt, ap);
	va_end (ap);

	sprintf (fname, "%s/.bxrG/%s.%02d%02d", home_path, logfile, tm->tm_mon+1, tm->tm_mday);

	if (access (fname, 0) != 0)
	{
		fd = open (fname, O_WRONLY|O_CREAT, 0660);
	}
	else
	{
		fd = open (fname, O_WRONLY|O_APPEND, 0660);
	}

	if (fd >= 0)
	{
		write (fd, sTmp, strlen (sTmp));
		close (fd);
	}

	return 0;

}
/* end of BXLog();*/


/*---------------------------------------------------------------------------*/
/* Enrollment Window OK Button Click                                    */
/*---------------------------------------------------------------------------*/
void e_ok_btn_clicked (GtkButton *e_ok_btn, gpointer *data)
{
	pthread_t hb, ps, pw;
	pthread_attr_t hbattr, psattr, pwattr;
	int threaderr;	

	func_Initiate();

	// 확인가입 정상적으로 안하고 눌렀을때 경우 수정 필요
	if (uDs.name[0] != NULL)
	{	
		BXLog (DBG, "%-30s	[%s]\n", "회원가입이 완료되었습니다.", cDs.uname);
		gtk_widget_destroy (data);

		//pthread_mutex_init (&mutex, NULL);

		//pthread_mutex_destroy (&mutex);

		//gtk_widget_hide (data); // destroy말고 hide로 테스트 해보기
		gtk_widget_show (main_window);			// 메인 창
		//gtk_main();
	}
	else
	{
		func_gtk_dialog_modal (0, window, "\n    회원가입을 해주세요.    \n");
	}
	
	return;
}
/* end of e_ok_btn_clicked(); */


/*---------------------------------------------------------------------------*/
/* Enrollment_window Webview ReLoad		                            */
/*---------------------------------------------------------------------------*/
void e_webview_load_failed (WebKitWebView *e_webview, WebKitLoadEvent load_event)
{
	gchar *uri = NULL;
	char msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));

	switch (load_event)
	{
		case WEBKIT_LOAD_STARTED:
			/* New load, we have now a provisional URI */
			uri = webkit_web_view_get_uri (e_webview);
			BXLog (DBG, "%-30s	[%s]\n", "WEBKIT_LOAD_STARTED", uri);

			if (uri[0] != 'h')
			{
				BXLog (ERR, "%-30s	\n", "BXR_WEBVIEW_ERR");
				sprintf (msg, "\n    관리자에게 문의하세요.[Code: %03d]    \n", BXR_WEBVIEW_ERR);
				func_gtk_dialog_modal (2, window, msg);
				nng_close (sock);
				gtk_main_quit();
			}
			/* Here we could start a spinner or update the
				* location bar with the provisional URI */
			break;

		case WEBKIT_LOAD_REDIRECTED:
			uri = webkit_web_view_get_uri (e_webview);
			BXLog (DBG, "%-30s	[%s]\n", "WEBKIT_LOAD_REDIRECTED", uri);
			break;

		case WEBKIT_LOAD_COMMITTED:
			/* The load is being performed. Current URI is
				* the final one and it won't change unless a new
				* load is requested or a navigation within the
				* same page is performed */
			uri = webkit_web_view_get_uri (e_webview);
			BXLog (DBG, "%-30s	[%s]\n", "WEBKIT_LOAD_COMMITTED", uri);
			break;

		case WEBKIT_LOAD_FINISHED:
			BXLog (DBG, "%-30s	[%s]\n", "WEBKIT_LOAD_FINISHED", uri);
			/* Load finished, we can now stop the spinner */
		break;

		default:
			BXLog (DBG, "%-30s	[%s]\n", "WEBKIT_LOAD_DEFAULT", uri);
			break;
    }

	return;
}
/* end of e_webview_load_failed function */


/*---------------------------------------------------------------------------*/
/* Enrollment Window Destroy                                    	  		 */
/*---------------------------------------------------------------------------*/
void e_window_destroy()
{
	BXLog (DBG, "%-30s	[%s] [%s]\n", "@BYE", cDs.uname, cDs.urank);
	nng_close (sock);
	gtk_widget_destroy (enrollment_window);
	gtk_main_quit();

	return;
}
/* end of m_window_destroy function */
/******** End of Enrollment Window Function **********/


/*---------------------------------------------------------------------------*/
/* Detect Refresh ScrollWindow                                      		  */
/*---------------------------------------------------------------------------*/
void func_set_display()
{
	d_view = d_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (d_scrolledwindow), d_view);
	gtk_widget_show_all ((GtkWidget *) d_scrolledwindow);

	v_view = v_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (v_scrolledwindow), v_view);
	gtk_widget_show_all ((GtkWidget *) v_scrolledwindow);

	dec_view = dec_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dec_scrolledwindow), dec_view);
	gtk_widget_show_all ((GtkWidget *) dec_scrolledwindow);

	dl_view = dl_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dl_scrolledwindow), dl_view);
	gtk_widget_show_all ((GtkWidget *) dl_scrolledwindow);

	vl_view = vl_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (vl_scrolledwindow), vl_view);
	gtk_widget_show_all ((GtkWidget *) vl_scrolledwindow);

	return;
}
/* end of func_set_display() */


/*---------------------------------------------------------------------------*/
/* Detect Refresh ScrollWindow                                      		  */
/*---------------------------------------------------------------------------*/
void d_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (d_scrolledwindow), d_view);	// 다 지우기	
	d_view = d_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (d_scrolledwindow), d_view);
	gtk_widget_show_all ((GtkWidget *) d_scrolledwindow);

	return;
}
/* end of d_Refresh_ScrollWindow() */

/*---------------------------------------------------------------------------*/
/* Vulnerability Detect Refresh ScrollWindow                           */
/*---------------------------------------------------------------------------*/
void v_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (v_scrolledwindow), v_view);	// 다 지우기	
	v_view = v_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (v_scrolledwindow), v_view);
	gtk_widget_show_all ((GtkWidget *) v_scrolledwindow);

	return;
}
/* end of v_Refresh_ScrollWindow() */

/*---------------------------------------------------------------------------*/
/* Decrypt Refresh ScrollWindow                                      		*/
/*---------------------------------------------------------------------------*/
void dec_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (dec_scrolledwindow), dec_view);	// 다 지우기	
	dec_view = dec_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dec_scrolledwindow), dec_view);
	gtk_widget_show_all ((GtkWidget *) dec_scrolledwindow);

	return;
}
/* end of dec_Refresh_ScrollWindow() */

/*---------------------------------------------------------------------------*/
/* Detect Log Refresh ScrollWindow                                      					   */
/*---------------------------------------------------------------------------*/
void dl_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (dl_scrolledwindow), dl_view);	// 다 지우기	
	dl_view = dl_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dl_scrolledwindow), dl_view);
	gtk_widget_show_all ((GtkWidget *) dl_scrolledwindow);

	return;
}
/* end of dl_Refresh_ScrollWindow() */

/*---------------------------------------------------------------------------*/
/* Vulneralbility Log Refresh ScrollWindow                               */
/*---------------------------------------------------------------------------*/
void vl_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (vl_scrolledwindow), vl_view);	// 다 지우기	
	vl_view = vl_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (vl_scrolledwindow), vl_view);
	gtk_widget_show_all ((GtkWidget *) vl_scrolledwindow);

	return;
}
/* end of vl_Refresh_ScrollWindow() */


/*********** Start of Main Window Function ***********/
/*---------------------------------------------------------------------------*/
/* Main Window Detect Button Click                                    	  */
/*---------------------------------------------------------------------------*/
void m_detect_btn_clicked (GtkButton *m_detect_btn, gpointer *data)
{
	gchar *check_kind_of_detect = 0;

	check_kind_of_detect = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (data));

	if (check_kind_of_detect == NULL)
	{
		func_gtk_dialog_modal (0, window, "\n    검사 종류 선택해주세요.    \n");
		return 0;
	}
	else
	{
		if (strcmp (check_kind_of_detect, "민감정보 검사") == 0)
		{
			//민감정보 검사 화면
			//d_Refresh_ScrollWindow();
			gtk_widget_show (detect_window);
			gtk_widget_hide (main_window);
		}
		else if (strcmp (check_kind_of_detect, "취약점 검사") == 0)
		{
			//취약점 검사 화면
			//v_Refresh_ScrollWindow();
			gtk_widget_show (vdetect_window);
			gtk_widget_hide (main_window);
		}
		else if (strcmp (check_kind_of_detect, "암호화 파일 검사") == 0)
		{
			//복호화  화면
			//dec_Refresh_ScrollWindow();
			gtk_entry_set_text (GTK_ENTRY (dec_detect_entry), dpDs.detectpath);
			gtk_widget_show (decrypt_window);
			gtk_widget_hide (main_window);
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    검사 종류를  선택해주세요.    \n");
		}
	}

	return;
}
/* end of m_detect_btn_clicked function */


/*---------------------------------------------------------------------------*/
/* Main Window Check Detect Entry                                    	  */
/*---------------------------------------------------------------------------*/
void m_combobox_changed (GtkComboBox *m_combobox, gpointer *data)
{
	gchar *check_kind_of_detect = 0;

	check_kind_of_detect = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (m_combobox));

	if (check_kind_of_detect == NULL)
	{
		func_gtk_dialog_modal (0, window, "\n    검사 종류 선택해주세요.    \n");
		return 0;
	}
	else
	{
		if (strcmp (check_kind_of_detect, "민감정보 검사") == 0)
		{
			//민감정보 검사 화면
			//d_Refresh_ScrollWindow();
			gtk_widget_show (detect_window);
			gtk_widget_hide (main_window);
		}
		else if (strcmp (check_kind_of_detect, "취약점 검사") == 0)
		{
			//취약점 검사 화면
			//v_Refresh_ScrollWindow();
			gtk_widget_show (vdetect_window);
			gtk_widget_hide (main_window);
		}
		else if (strcmp (check_kind_of_detect, "암호화 파일 검사") == 0)
		{
			//복호화  화면
			//dec_Refresh_ScrollWindow();
			gtk_entry_set_text (GTK_ENTRY (dec_detect_entry), dpDs.decpath);
			gtk_widget_show (decrypt_window);
			gtk_widget_hide (main_window);
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    검사 종류를  선택해주세요.    \n");
		}
	}

	return;
}
/* end of m_combobox_changed function */



/*---------------------------------------------------------------------------*/
/* Main Window Setting Button Click                                    	  */
/*---------------------------------------------------------------------------*/
void m_setting_btn_clicked (GtkButton *m_setting_btn, gpointer *data)
{
	chk_window = 0;
	gtk_widget_show (setting_window);
	gtk_widget_hide (main_window);

	return;
}
/* end of m_setting_btn_clicked function */


/*---------------------------------------------------------------------------*/
/* Main Window Destroy                                    	  					 */
/*---------------------------------------------------------------------------*/
void m_window_destroy()
{
	BXLog (DBG, "%-30s	[%s] [%s]\n", "BYE", cDs.uname, cDs.urank);
	nng_close (sock);
	gtk_main_quit();

	return;
}
/* end of m_window_destroy function */
/************ End of Main Window Function ***********/



/********** Start of Decrypt Window Function **********/
/*---------------------------------------------------------------------------*/
/* Decrypt TreeView Column variable                                        */
/*---------------------------------------------------------------------------*/
enum
{
	dec_treeview_num = 0,
	dec_treeview_filename,
	dec_treeview_fsize,
	dec_treeview_fstat,
	dec_treeview_fileloca,
	dec_treeview_workstart,
	dec_treeview_workend,
	DEC_NUM_COLS
} ;
/* end of Decrypt TreeView Column variable */


/*---------------------------------------------------------------------------*/
/* Decrypt TreeView Selection 			                                        */
/*---------------------------------------------------------------------------*/
gboolean dec_view_selection_func 	(GtkTreeSelection *selection,
														 GtkTreeModel	   *model,
														 GtkTreePath		  *path,
														 gboolean				 path_currently_selected,
														 gpointer				  userdata)
{
	GtkTreeIter iter;
	gchar *fstat, *fpath, *wstart, *wend;
	unsigned int fsize;
	
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, dec_treeview_filename, 	&fname,	-1);
			gtk_tree_model_get (model, &iter, dec_treeview_fsize,		    &fsize,	   -1);
			gtk_tree_model_get (model, &iter, dec_treeview_fstat,		    &fstat,		-1);
			gtk_tree_model_get (model, &iter, dec_treeview_fileloca,	   &fpath,	 -1);
			gtk_tree_model_get (model, &iter, dec_treeview_workstart,	&wstart, -1);
			gtk_tree_model_get (model, &iter, dec_treeview_workend,	   &wend,  -1);

			memset (sdfDs.fname, 0x00, sizeof (sdfDs.fname));
			memset (sdfDs.fstat, 0x00, sizeof (sdfDs.fstat));
			memset (sdfDs.fpath, 0x00, sizeof (sdfDs.fpath));
			memset (sdfDs.start, 0x00, sizeof (sdfDs.start));
			memset (sdfDs.end, 0x00, sizeof (sdfDs.end));
			memset (sdfDs.ffpath, 0x00, sizeof (sdfDs.ffpath));
			sdfDs.fsize = 0;

			// input data in structure
			strcpy (sdfDs.fname, fname);
			sdfDs.fsize = fsize;
			strcpy (sdfDs.fstat, fstat);
			strcpy (sdfDs.fpath, fpath);
			strcpy (sdfDs.start, wstart);
			strcpy (sdfDs.end, wend);
			sprintf (sdfDs.ffpath, "%s/%s", fpath, fname);
			g_print ("파일위치: [%s], 파일크기: [%d] 선택.\n", sdfDs.ffpath, sdfDs.fsize);
		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	}

	return TRUE; /* allow selection state to change */
}
/* end of dec_view_selection_func(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt TreeModel Create and Fill 			                            */
/*---------------------------------------------------------------------------*/
static GtkTreeModel *dec_create_and_fill_model (void)
{
	GtkTreeIter iter;
	dectreestore = gtk_tree_store_new (DEC_NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	for (int i = 0; i <= chk_dfcnt; i++)
	{
		gtk_tree_store_append (dectreestore, &iter, NULL);
		gtk_tree_store_set (dectreestore, &iter,
					  dec_treeview_num, i + 1,
					  dec_treeview_filename,	dfDs[i].fname,
					  dec_treeview_fsize,			dfDs[i].fsize,
					  dec_treeview_fstat,			dfDs[i].fstat,
					  dec_treeview_fileloca,	   dfDs[i].fpath,
					  dec_treeview_workstart,	dfDs[i].start,
					  dec_treeview_workend,	   dfDs[i].end,
					  -1);
	}
	
	return GTK_TREE_MODEL (dectreestore);
}
/* end of dec_create_and_fill_model(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Create View and Model 				                            */
/*---------------------------------------------------------------------------*/
static GtkWidget *dec_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		     *renderer;
	GtkTreeModel		      *model;
	GtkTreeSelection	      *selection;
	
	dec_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_num);

	// --- Column #파일 이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일이름");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_filename);

	// --- Column #파일 크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일크기");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_fsize);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_fstat);
	
	// --- Column #파일 위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일위치");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_fileloca);

	// --- Column #작업 시작 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업시작");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_workstart);

	// --- Column #작업 종료 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업종료");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dec_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dec_treeview_workend);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dec_view));
	
	gtk_tree_selection_set_select_function (selection, dec_view_selection_func, NULL, NULL);

	model = dec_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (dec_view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection (GTK_TREE_VIEW (dec_view)),
							  GTK_SELECTION_SINGLE);

	return dec_view;
}
/* end of dec_create_view_and_model(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Folder Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void dec_folder_btn_clicked (GtkButton *dec_folder_btn, gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("폴더 선택", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_ACCEPT)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (dec_detect_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}

	return;
}
/* end of dec_folder_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Detect Entry ACtivate				                               */
/*---------------------------------------------------------------------------*/
void dec_detect_entry_activate (GtkEntry *dec_detect_entry, gpointer *data)
{
	path = (gchar *) gtk_entry_get_text (dec_detect_entry);

	return;
}
/* end of dec_detect_entry_activate(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Detect Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void dec_detect_btn_clicked (GtkButton *dec_detect_btn, gpointer *data)
{
	chk_dfcnt = -1; 	// 암호파일개수 count 초기화
	percent = 0.0;
	memset (&fDs, 0, sizeof(fDs)); // 구조체 초기화
	char *msg[MAX_ERROR_MSG] = {0,};

	memset (msg, 0x00, strlen (msg));
	memset (message, 0x00, strlen (message));
	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dec_progressbar), 0.00);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dec_progressbar), message);

	while (gtk_events_pending()) 
				gtk_main_iteration(); 

	path = (gchar *) gtk_entry_get_text (dec_detect_entry);

	if (path[0] == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    [파일/폴더]이 선택되지 않았습니다.   \n");

		return -1;
	}
	
	BXLog (DBG, "%-30s	\n", "START func_Find_EncFile()");
	func_Find_EncFile (path);
	BXLog (DBG, "%-30s	\n", "END func_Find_EncFile()");

	while (gtk_events_pending()) 
				gtk_main_iteration(); 

	memset (message, 0x00, strlen (message));
	sprintf (message, "%.2f%% Complete", 100.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dec_progressbar), 100.00);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dec_progressbar), message);

	while (gtk_events_pending()) 
						gtk_main_iteration(); 

	dec_Refresh_ScrollWindow();
	//BXLog (DBG, "%-30s	[%.3f] [%.3f]\n", "TOTAL func_Find_EncFile() TIME: A SECOND, AVERAGE: B SECOND", chktime, chktime / (chk_dfcnt + 1));

	sprintf (msg, "\n	발견된 암호파일 총 개수:   %d		  \n", chk_dfcnt + 1);
	func_gtk_dialog_modal (0, window, msg);
	chk_loop = 0;

	return;
}
/* end of dec_detect_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Setting Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void dec_setting_btn_clicked (GtkButton *dec_setting_btn, gpointer *data)
{
	chk_window = 5;
	gtk_widget_show (setting_window);
	gtk_widget_hide (decrypt_window);

	return;
}
/* end of dec_setting_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Button Click				                            		 			  */
/*---------------------------------------------------------------------------*/
void dec_decrypt_btn_clicked (GtkButton *dec_decrypt_btn, gpointer *data)
{
	int res = 0;
	char message[1134] = {0,};

	percent = 0.0;

	memset (message, 0x00, strlen (message));

	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
	gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);

	// 검사 종류에 따른 복호화 파일 저장위치 확인
	if ((chk_detect == 0) && (dpDs.decpath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 복호화가 불가능한 상태입니다.   \n    복호화 파일 저장 위치를 확인 해주세요.	\n");
	}
	else if ((chk_detect == 1) && (pDs.decpath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 복호화가 불가능한 상태입니다.   \n    복호화 파일 저장 위치를 확인 해주세요.	\n");
	}
	
	// 선택한 파일 과 파일의 상태 확인
	if (sdfDs.ffpath[0] == 0x00 || strcmp (sdfDs.fstat, "암호화") != 0)
	{
		func_gtk_dialog_modal (0, window, "\n    파일 복호화가 불가능한 상태입니다.   \n    파일 [선택/상태]을 확인 해주세요.	\n");
	}
	else
	{
		sprintf (message, 
			"\n    아래 파일을 복호화 하시겠습니까?\n    [ %s ]    \n", sdfDs.ffpath);
    
		if (func_gtk_dialog_modal(1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_GetTime();
			func_decrypt();
			sprintf (message, "%.2f%% Complete", 100.00);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 100.00);
			gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);
			func_gtk_dialog_modal (0, window, "\n    파일 복호화가 완료되었습니다.    \n");

			for (int i = 0; i <= chk_dfcnt; i++)
			{
				res = strcmp (fname, dfDs[i].fname);

				if (res == 0)
				{
					strcpy (dfDs[i].start, chk_worktime);
					strcpy (sdfDs.start, chk_worktime);
					func_GetTime();
					strcpy (dfDs[i].end, chk_worktime);
					strcpy (sdfDs.end, chk_worktime);
					strcpy (dfDs[i].fstat, "복호화");
					strcpy (sdfDs.fstat, "복호화");
					BXLog (DBG, "%-30s	[%d] [%s] [%s]\n", "NUMBER: A, FILE: B, DID: C", i, dfDs[i].fname, dfDs[i].fstat);
					BXDB_Push (i, chk_dbtable);
					//func_nng_client (5, i);
				}
			}

			dec_Refresh_ScrollWindow();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
		}
	}

	chk_fname[0] = 0; // 초기화 //

	return;
}
/* end of dec_decrypt_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Log ButtonClick				                            		 	  */
/*---------------------------------------------------------------------------*/
void dec_log_btn_clicked (GtkButton *dec_log_btn, gpointer *data)
{
	chk_dlwindow = 1;
	gtk_widget_hide (decrypt_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of dec_log_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Close Button Click				                            		  */
/*---------------------------------------------------------------------------*/
void dec_close_btn_clicked (GtkButton *dec_close_btn, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));
	gtk_widget_show (main_window);

	return;
}
/* end of dec_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Decrypt Window Delete				                            		     */
/*---------------------------------------------------------------------------*/
void dec_window_delete (GtkWidget *decrypt_window)
{
	gtk_widget_hide (decrypt_window);
	gtk_widget_show (main_window);

	return;
}
/* end of dec_window_delete(); function */
/********** End of Decrypt Window Function ***********/



/*********** Start of Detect Window Function **********/
/*---------------------------------------------------------------------------*/
/* Detect Entry Activate	                                    					 */
/*---------------------------------------------------------------------------*/
void d_detect_entry_activate (GtkEntry *d_detect_entry, gpointer *data)
{
	path = (gchar *) gtk_entry_get_text (d_detect_entry);
	//g_print ("선택한 폴더 위치: %s\n", path);

	return;
}
/* end of d_detect_entry_activate(); function */


/*---------------------------------------------------------------------------*/
/* Detect Folder Button Click				                            		  */
/*---------------------------------------------------------------------------*/
void d_folder_btn_clicked (GtkButton *d_folder_btn, gpointer *data)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

    filechooserdialog = gtk_file_chooser_dialog_new ("Select File or Folder", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_OPEN, ("_선택"), GTK_RESPONSE_NONE, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_NONE)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (d_detect_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}
	//g_print ("선택한 폴더 위치: %s\n", path);

	return;
}
/* end of d_folder_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect TreeView Column variable                                         */
/*---------------------------------------------------------------------------*/
enum
{
	d_treeview_num = 0,
	d_treeview_filename,
	d_treeview_jcnt,
	d_treeview_dcnt,
	d_treeview_fgcnt,
	d_treeview_pcnt,
	d_treeview_fstat,
	d_treeview_fsize,
	d_treeview_ftype,
	d_treeview_fileloca,
	d_treeview_round,
	d_treeview_workstart,
	d_treeview_workend,
	D_NUM_COLS
} ;
/* end of Detect TreeView Column variable */


/*---------------------------------------------------------------------------*/
/* Detect TreeView Selection 			                                         */
/*---------------------------------------------------------------------------*/
gboolean d_view_selection_func 	(GtkTreeSelection *selection,
														 GtkTreeModel	   *model,
														 GtkTreePath		  *path,
														 gboolean				 path_currently_selected,
														 gpointer				  userdata)
{
	GtkTreeIter iter;
	gchar *fstat, *fpath, *ftype, *wstart, *wend;
	unsigned int fsize = 0, jcnt = 0, dcnt = 0, fgcnt = 0, pcnt = 0;
	
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, d_treeview_filename, 	&fname,	 -1);
			gtk_tree_model_get (model, &iter, d_treeview_jcnt,		    &jcnt,		 -1);
			gtk_tree_model_get (model, &iter, d_treeview_dcnt,		   &dcnt,	   -1);
			gtk_tree_model_get (model, &iter, d_treeview_fgcnt,		   &fgcnt,	  -1);
			gtk_tree_model_get (model, &iter, d_treeview_pcnt,		   &pcnt,	   -1);
			gtk_tree_model_get (model, &iter, d_treeview_fstat,		    &fstat,		 -1);
			gtk_tree_model_get (model, &iter, d_treeview_fsize,		    &fsize,	     -1);
			gtk_tree_model_get (model, &iter, d_treeview_ftype,		  &ftype, 	 -1);
			gtk_tree_model_get (model, &iter, d_treeview_fileloca,	   &fpath,	  -1);
			gtk_tree_model_get (model, &iter, d_treeview_workstart,	&wstart,   -1);
			gtk_tree_model_get (model, &iter, d_treeview_workend,	&wend,	 -1);

			memset (sfDs.fname, 0x00, sizeof (sfDs.fname));
			memset (sfDs.fstat, 0x00, sizeof (sfDs.fstat));
			memset (sfDs.ftype, 0x00, sizeof (sfDs.ftype));
			memset (sfDs.fpath, 0x00, sizeof (sfDs.fpath));
			memset (sfDs.start, 0x00, sizeof (sfDs.start));
			memset (sfDs.end, 0x00, sizeof (sfDs.end));
			memset (sfDs.ffpath, 0x00, sizeof (sfDs.ffpath));
			sfDs.jcnt = 0;
			sfDs.dcnt = 0;
			sfDs.fgcnt = 0;
			sfDs.pcnt = 0;
			sfDs.fsize = 0;

			// input data in structure
			strcpy (sfDs.fname, fname);
			sfDs.jcnt = jcnt;
			sfDs.dcnt = dcnt;
			sfDs.fgcnt = fgcnt;
			sfDs.pcnt = pcnt;
			sfDs.fsize = fsize;
			strcpy (sfDs.fstat, fstat);
			strcpy (sfDs.ftype, ftype);
			strcpy (sfDs.fpath, fpath);
			strcpy (sfDs.start, wstart);
			strcpy (sfDs.end, wend);
			sprintf (sfDs.ffpath, "%s/%s", fpath, fname);
		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	}

	return TRUE; /* allow selection state to change */
}
/* end of d_view_selection_func(); function */


/*---------------------------------------------------------------------------*/
/* Detect TreeModel Create and Fill 			                             */
/*---------------------------------------------------------------------------*/
static GtkTreeModel *d_create_and_fill_model (void)
{
	int jcnt = 0, dcnt = 0, fgcnt = 0, pcnt = 0;
	GtkTreeIter iter;


	dtreestore = gtk_tree_store_new (D_NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
																				G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

	for (int i = 0; i <= chk_fcnt; i++)
	{
		gtk_tree_store_append (dtreestore, &iter, NULL);

		gtk_tree_store_set (dtreestore, &iter,
										d_treeview_num, i + 1,
										d_treeview_filename,	fDs[i].fname,
										d_treeview_jcnt,	fDs[i].jcnt,
										d_treeview_dcnt,	fDs[i].dcnt,
										d_treeview_fgcnt,	fDs[i].fgcnt,
										d_treeview_pcnt,	fDs[i]. pcnt,
										d_treeview_fstat,	fDs[i].fstat,
										d_treeview_fsize,	fDs[i].fsize,
										d_treeview_ftype,	fDs[i].ftype,
										d_treeview_fileloca,	fDs[i].fpath,
										d_treeview_round,	fDs[i].round,
										d_treeview_workstart,	fDs[i].start,
										d_treeview_workend,	fDs[i].end,
										-1);

		jcnt  += fDs[i].jcnt;
		dcnt += fDs[i].dcnt;
		fgcnt += fDs[i].fgcnt;
		pcnt += fDs[i]. pcnt;

	}

	chk_jumin = jcnt;
	chk_driver = dcnt;
	chk_foreigner = fgcnt;
	chk_passport = pcnt;
	
	return GTK_TREE_MODEL (dtreestore);
}
/* end of d_create_and_fill_model(); function */


/*---------------------------------------------------------------------------*/
/* Detect Create View and Model 				                            */
/*---------------------------------------------------------------------------*/
static GtkWidget *d_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		     *renderer;
	GtkTreeModel		      *model;
	GtkTreeSelection	      *selection;
	
	d_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_num);

	// --- Column #파일 이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일이름");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_filename);

	// --- Column #주민번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "주민번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_jcnt);

	// --- Column #운전면허 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "운전면허");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_dcnt);
	
	// --- Column #외국인등록번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "외국인등록");
	gtk_tree_view_append_column (GTK_TREE_VIEW(d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fgcnt);
	
	// --- Column #여권번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "여권번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_pcnt);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fstat);
	
	// --- Column #파일 크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일크기");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fsize);

	// --- Column #파일 포맷 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일포맷");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_ftype);
	
	// --- Column #파일 위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일위치");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fileloca);

	// --- Column #회차 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "회차");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_round);
	
	// --- Column #작업 시작 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업시작");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_workstart);

	// --- Column #작업 종료 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업종료");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_workend);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (d_view));
	
	gtk_tree_selection_set_select_function (selection, d_view_selection_func, NULL, NULL);

	model = d_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (d_view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection (GTK_TREE_VIEW (d_view)),
							  GTK_SELECTION_SINGLE);

	return d_view;
}
/* end of d_create_view_and_model(); function */


/*---------------------------------------------------------------------------*/
/* Detect Button Click				                 				           		  */
/*---------------------------------------------------------------------------*/
void d_detect_btn_clicked()
{	
	char *msg[MAX_ERROR_MSG] = {0,};

	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		printf ("d [%s]  [%s]  [%s]  [%s]\n", chk_psstat, chk_dbtnstat, chk_pwstat, chk_vdbtnstat);
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}
	strcpy (chk_psstat, "R");
	strcpy (chk_dbtnstat, "Y");
	
	while (gtk_events_pending()) 
				gtk_main_iteration(); 

	chk_fcnt = -1; 	// 검출된 파일개수 count 초기화
	chk_dcnt = 0;	// 검사한 파일개수 count 초기화
	chk_afcnt = 0; // 검사할 파일 총개수 count 초기화
	chk_ingfcnt = 0; // 검사할 파일 총개수 count 초기화
	percent = 0.0;

	memset (&fDs, 0, sizeof(fDs)); // 구조체 초기화
	memset (msg, 0x00, strlen (msg));
	memset (message, 0x00, strlen (message));
	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);

	while (gtk_events_pending()) 
					gtk_main_iteration(); 

	path = (gchar *) gtk_entry_get_text (d_detect_entry);

	if (path[0] == 0x00)
	{
		strcpy (chk_psstat, "0");
		strcpy (chk_dbtnstat, "0");
		func_gtk_dialog_modal (0, window, "\n    [파일/폴더]이 선택되지 않았습니다.   \n");

		return -1;
	}

	func_FileCount (path);

	BXLog (DBG, "%-30s	\n", "START func_Detect()");
	func_Detect (path);
	BXLog (DBG, "%-30s	\n", "END func_Detect()");

	memset (message, 0x00, strlen (message));
	sprintf (message, "%.2f%% Complete", 100.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 100.00);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);

	while (gtk_events_pending()) 
			gtk_main_iteration();

	d_Refresh_ScrollWindow();

	printf ("%d\n", chk_detect);
	if (chk_detect == 0)
	{
		for (int i = 0; i <= chk_fcnt; i++)
		{
			BXDB_Push (i, chk_detect);
		}

		func_nng_client (2, 0);

		sprintf (msg, "\n	검사 종류:   민감정보 수동검사		  \n	검출된 파일/검사한 파일:   %d/%d		  \n	주민등록번호 개수:   %d		  \n	운전면허번호 개수:   %d		  \n	외국인등록번호 개수:   %d		  \n	여권번호 개수:   %d		  \n", 
								chk_fcnt + 1, chk_dcnt, chk_jumin, chk_driver, chk_foreigner, chk_passport);
	}
	else if (chk_detect == 1)
	{
		for (int i = 0; i <= chk_fcnt; i++)
		{
			BXDB_Push (i, chk_detect);
		}

		func_nng_client (3, 0);

		sprintf (msg, "\n	검사 종류:   민감정보 정기검사		  \n	검출된 파일/검사한 파일:   %d/%d		  \n	주민등록번호 개수:   %d		  \n	운전면허번호 개수:   %d		  \n	외국인등록번호 개수:   %d		  \n	여권번호 개수:   %d		  \n", 
								chk_fcnt + 1, chk_dcnt, chk_jumin, chk_driver, chk_foreigner, chk_passport);
	}

	func_gtk_dialog_modal (2, window, msg);

	strcpy (chk_psstat, "N");
	strcpy (chk_dbtnstat, "N");

	chk_loop = 0;
}
/* end of d_detect_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Setting Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void d_setting_btn_clicked (GtkButton *d_setting_btn, gpointer *data)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	chk_window = 1;
	gtk_widget_show (setting_window);
	gtk_widget_hide (detect_window);
	
	return;
}
/* end of d_setting_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Move Button Click				                            			 */
/*---------------------------------------------------------------------------*/
void d_move_btn_clicked (GtkButton *d_move_btn, gpointer *data)
{
	int res = 0;
	gchar *move_path = 0;

	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	memset (message, 0x00, strlen (message));

	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
	gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);

	// 검사 종류에 따른 파일 이동 저장위치 확인
	if ((chk_detect == 0) && (dpDs.movepath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 이동이 불가능한 상태입니다.   \n    민감정보가 검출된 파일을 이동시킬 위치를 확인 해주세요.	\n");
	}
	else if ((chk_detect == 1) && (pDs.movepath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 이동이 불가능한 상태입니다.   \n    민감정보가 검출된 파일을 이동시킬 위치를 확인 해주세요.	\n");
	}

	// 선택한 파일 과 파일의 상태 확인
	if (sfDs.ffpath[0] == 0x00 || strcmp (sfDs.fstat, "일반") != 0)
	{
		func_gtk_dialog_modal (0, window, "\n    파일 이동이 불가능한 상태입니다.   \n    파일 [선택/상태]을 확인 해주세요.	\n");
	}
	else
	{
		if (chk_detect == 0)
		{
			move_path = (gchar *) gtk_entry_get_text (s_move_entry);
		}
		else
		{
			strcpy (move_path, pDs.movepath);
		}

		sprintf (message, 
			"\n    %s 위치로 \n    아래 파일을 이동하시겠습니까?\n    [ %s ]    \n", move_path, sfDs.ffpath);

		if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			chk_fmanage = 0;
			func_GetTime();
			func_Move();
			sprintf (message, "%.2f%% Complete", 100.00);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 100.00);
			gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);
			func_gtk_dialog_modal (0, window, "\n    파일 이동이 완료되었습니다.    \n");

			for (int i = 0; i <= chk_fcnt; i++)
			{	
				res = strcmp (fname, fDs[i].fname);

				if (res == 0)
				{
					strcpy (fDs[i].start, chk_worktime);
					strcpy (sfDs.start, chk_worktime);
					func_GetTime();
					strcpy (fDs[i].end, chk_worktime);
					strcpy (sfDs.end, chk_worktime);
					strcpy (fDs[i].fstat, "이동");
					strcpy (sfDs.fstat, "이동");
					BXLog (DBG, "%-30s	[%d] [%s] [%s]\n", "NUMBER: A, FILE: B, DID: C", i, fDs[i].fname, fDs[i].fstat);
					BXDB_Push (i, chk_dbtable);
					if (chk_detect == 1)
					{
						//func_nng_client (4, i);
					}
				}
			}

			d_Refresh_ScrollWindow();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
		}
	}

	chk_fname[0] = 0; // 초기화 //

	return;
}
/* end of d_move_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Encrypt Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void d_encrypt_btn_clicked (GtkButton *d_encrypt_btn, gpointer *data)
{
	int res = 0;
	char message[1134] = {0,};
	
	
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	percent = 0.0;
	memset (message, 0x00, strlen (message));

	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
	gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);

	// 검사 종류에 따른 암호화 파일 저장위치 확인
	if ((chk_detect == 0) && (dpDs.encpath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 암호화가 불가능한 상태입니다.   \n    암호화 파일 저장 위치를 확인 해주세요.	\n");
	}
	else if ((chk_detect == 1) && (pDs.encpath[0] == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    파일 암호화가 불가능한 상태입니다.   \n    암호화 파일 저장 위치를 확인 해주세요.	\n");
	}

	// 선택한 파일 과 파일의 상태 확인
	if (sfDs.ffpath[0] == 0x00 || strcmp (sfDs.fstat, "일반") != 0)
	{
		func_gtk_dialog_modal (0, window, "\n    파일 암호화가 불가능한 상태입니다.   \n    파일 [선택/상태]을 확인 해주세요.	\n");
	}
	else
	{
		sprintf (message, 
			"\n 아래 파일을 암호화 하시겠습니까?\n    [ %s ]    \n", sfDs.ffpath);
	
		if (func_gtk_dialog_modal(1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_GetTime();
			func_encrypt();
			sprintf (message, "%.2f%% Complete", 100.00);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 100.00);
			gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);
			func_gtk_dialog_modal (0, window, "\n    파일 암호화가 완료되었습니다.    \n");

			for (int i = 0; i <= chk_fcnt; i++)
			{
				res = strcmp (fname, fDs[i].fname);

				if (res == 0)
				{
					strcpy (fDs[i].start, chk_worktime);
					strcpy (sfDs.start, chk_worktime);
					func_GetTime();
					strcpy (fDs[i].end, chk_worktime);
					strcpy (sfDs.end, chk_worktime);
					strcpy (fDs[i].fstat, "암호화");
					strcpy (sfDs.fstat, "암호화");	
					BXLog (DBG, "%-30s	[%d] [%s] [%s]\n", "NUMBER: A, FILE: B, DID: C", i, fDs[i].fname, fDs[i].fstat);
					BXDB_Push (i, chk_dbtable);
					if (chk_detect == 1)
					{
						//func_nng_client (4, i);
					}
				}
			}

			d_Refresh_ScrollWindow();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
		}
	}

	chk_fname[0] = 0; // 초기화 //

	return;
}
/* end of d_encrypt_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Delete Button Click				                            		 */
/*---------------------------------------------------------------------------*/
void d_delete_btn_clicked (GtkButton *d_delete_btn, gpointer *data)
{
	int res = 0;

	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	memset (message, 0x00, strlen (message));

	sprintf (message, "%.2f%% Complete", 0.00);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
	gtk_progress_bar_set_text  (GTK_PROGRESS_BAR (d_progressbar), message);

	if (sfDs.ffpath[0] == 0x00 || strcmp (sfDs.fstat, "일반") != 0)
	{
		func_gtk_dialog_modal (0, window, "\n    파일 삭제가 불가능한 상태입니다.   \n    파일 [선택/상태]를 확인 해주세요.	\n");
	}
	else
	{
		sprintf (message, 
			"\n    삭제 후에는 복구가 불가능 합니다.\n    아래 파일을 삭제하시겠습니까?\n    [ %s ]    \n", sfDs.ffpath);
	
		if (func_gtk_dialog_modal (1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_GetTime();
			func_file_eraser (3);
			for (int i = 0; i <= chk_fcnt; i++)
			{	
				res = strcmp (fname, fDs[i].fname);

				if (res == 0)
				{
					strcpy (fDs[i].start, chk_worktime);
					strcpy (sfDs.start, chk_worktime);
					func_GetTime();
					strcpy (fDs[i].end, chk_worktime);
					strcpy (sfDs.end, chk_worktime);
					strcpy (fDs[i].fstat, "삭제");
					strcpy (sfDs.fstat, "삭제");
					BXLog (DBG, "%-30s	[%d] [%s] [%s]\n", "NUMBER: A, FILE: B, DID: C", i, fDs[i].fname, fDs[i].fstat);
					BXDB_Push (i, chk_dbtable);
					if (chk_detect == 1)
					{
						func_nng_client (4, i);
					}
				}
			}

			d_Refresh_ScrollWindow();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0.00);
		}
	}

	chk_fname[0] = 0; // 초기화 //

	return;
}
/* end of d_delete_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Log Button Click				                            		 	   */
/*---------------------------------------------------------------------------*/
void d_log_btn_clicked (GtkButton *d_log_btn, gpointer *data)
{
	chk_dlwindow = 0;

	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	gtk_widget_hide (detect_window);
	//dl_Refresh_ScrollWindow();
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of d_log_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Close Button Click				                            		  */
/*---------------------------------------------------------------------------*/
void d_close_btn_clicked (GtkButton *d_close_btn, gpointer *data)
{	
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	gtk_widget_hide (GTK_WIDGET (data));
	gtk_widget_show (main_window);

	return;
}
/* end of d_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Detect Window Delete				                            		 		  */
/*---------------------------------------------------------------------------*/
void d_window_delete (GtkWidget *detect_window)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	gtk_widget_hide (detect_window);
	gtk_widget_show (main_window);

	return;
}
/* end of d_window_delete(); function */
/*********** End of Detect Window Function ***********/



/****** Start of Vulnerability Detect Window Function *****/
/*---------------------------------------------------------------------------*/
/* Vulnerability TreeView Column variable                    			 */
/*---------------------------------------------------------------------------*/
enum
{
	v_treeview_num = 0,
	v_treeview_list,
	v_treeview_result,
	v_treeview_stat,
	V_NUM_COLS
} ;
/* end of Vulnerability Detect TreeView Column variable */


/*---------------------------------------------------------------------------*/
/* Vulnerability TreeView Create and Fill 			            			*/
/*---------------------------------------------------------------------------*/
static GtkTreeModel *v_create_and_fill_model (void)
{
	GtkTreeIter iter;

	vtreestore = gtk_tree_store_new (V_NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   1, v_treeview_list, "root계정 원격 접속 제한						", v_treeview_result, viDs[0].result, v_treeview_stat, viDs[0].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   2, v_treeview_list, "패스워드 복잡성 설정						", v_treeview_result, viDs[1].result, v_treeview_stat, viDs[1].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   3, v_treeview_list, "계정 잠금 임계값 설정						", v_treeview_result, viDs[2].result, v_treeview_stat, viDs[2].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   4, v_treeview_list, "패스워드 파일 보호						", v_treeview_result, viDs[3].result, v_treeview_stat, viDs[3].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   5, v_treeview_list, "root 홈, 패스 디렉터리 권한 및 패스 설정						", v_treeview_result, viDs[4].result, v_treeview_stat, viDs[4].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   6, v_treeview_list, "파일 및 디렉터리 소유자 설정						", v_treeview_result, viDs[5].result, v_treeview_stat, viDs[5].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   7, v_treeview_list, "로그인 패스워드 안전성 여부 점검						", v_treeview_result, viDs[6].result, v_treeview_stat, viDs[6].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   8, v_treeview_list, "로그인 패스워드의 분기 1회 이상 변경 여부 점검						", v_treeview_result, viDs[7].result, v_treeview_stat, viDs[7].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num,   9, v_treeview_list, "화면보호기 설정 여부 점검						", v_treeview_result, viDs[8].result, v_treeview_stat, viDs[8].stat, -1);

	gtk_tree_store_append (vtreestore, &iter, NULL);
	gtk_tree_store_set (vtreestore, &iter, v_treeview_num, 10, v_treeview_list, "사용자 공유 폴더 설정 여부 점검						", v_treeview_result, viDs[9].result, v_treeview_stat, viDs[9].stat, -1);
	
	return GTK_TREE_MODEL (vtreestore);
}
/* end of v_create_and_fill_model(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Create View and Model 				  		 			  */
/*---------------------------------------------------------------------------*/
static GtkWidget *v_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		     *renderer;
	GtkTreeModel		      *model;
	GtkTreeSelection	      *selection;
	
	v_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (v_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", v_treeview_num);

	// --- Column #검사항목 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "검사항목");
	gtk_tree_view_append_column (GTK_TREE_VIEW (v_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", v_treeview_list);
	
	// --- Column #결과 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "결과    ");
	gtk_tree_view_append_column (GTK_TREE_VIEW (v_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", v_treeview_result);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (v_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", v_treeview_stat);
	
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (v_view));

	model = v_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (v_view), model);

	g_object_unref (model); // destroy model automatically with view //

	return v_view;
}
/* end of v_create_view_and_model(); function */
/* end of treeview function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Button Click				                 		   				*/
/*---------------------------------------------------------------------------*/
void v_detect_btn_clicked()
{
	char *vitemstr[2] = {0,}, *vsafestr[2] = {0,}, *vvurstr[2] = {0,}, *vimposiible[2] = {0,};
	char *msg[MAX_ERROR_MSG] = {0,};
	//char *vdatebuf[40] = {0, };

	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		printf ("v [%s]  [%s]  [%s]  [%s]\n", chk_psstat, chk_dbtnstat, chk_pwstat, chk_vdbtnstat);
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}
	strcpy (chk_pwstat, "R");
	strcpy (chk_vdbtnstat, "Y");

	// 앞에 정책 받아오는거 있어야함, ex)수동/정기 어떤항목  //
	func_VulnerabilityDetect ();

	// main화면에서 취약점검사화면 올때 default화면은 마지막 정기검사 결과, 클릭하면 수동검사 화면으로 전환
	sprintf (vitemstr, "%d", vDs.item);
	sprintf (vsafestr, "%d", vDs.safe);
	sprintf (vimposiible, 	"%d", vDs.imposiible);
	sprintf (vvurstr, "%d",   vDs.vurnelability);
	//sprintf (vdatebuf, "%s (%d 회차)",  vDs.end, pDs.pwround);
	gtk_label_set_text (GTK_LABEL (v_item_label), vitemstr);
	gtk_label_set_text (GTK_LABEL (v_safe_label), vsafestr);
	gtk_label_set_text (GTK_LABEL (v_vurnelability_label), vvurstr);
	gtk_label_set_text (GTK_LABEL (v_imposiible_label), vimposiible);
	//gtk_label_set_text (GTK_LABEL (v_date_label), vdatebuf); // 정책 받아온값 넣어야함
	
	v_Refresh_ScrollWindow ();
	
	if (chk_vdetect == 0)
	{
		func_nng_client (6, 0);

		sprintf (msg, "\n	검사 종류:   취약점 수동검사		  \n	완료된 점검 항목 수/선택된 점검 항목 수:   %d/%d		  \n	안전:   %d		  \n	위험:   %d		  \n	미수행:   %d		  \n", 
								vDs.safe + vDs.vurnelability, vDs.item, vDs.safe, vDs.vurnelability, vDs.imposiible);
	}
	else if (chk_vdetect == 1)
	{
		func_nng_client (7, 0);	

		sprintf (msg, "\n	검사 종류:   취약점 정기검사		  \n	완료된 점검 항목 수/선택된 점검 항목 수:   %d/%d		  \n	안전:   %d		  \n	위험:   %d		  \n	미수행:   %d		  \n", 
								vDs.safe + vDs.vurnelability, vDs.item, vDs.safe, vDs.vurnelability, vDs.imposiible);
	}

	func_gtk_dialog_modal (2, window, msg);	

	strcpy (chk_pwstat, "N");
	strcpy (chk_vdbtnstat, "N");
}
/* end of v_detect_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Log Button Click				                    			 */
/*---------------------------------------------------------------------------*/
void v_log_btn_clicked (GtkButton *v_log_btn, gpointer *data)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	gtk_widget_hide (vdetect_window);
	//vl_Refresh_ScrollWindow();
	gtk_widget_show (GTK_WIDGET (data));
}
/* end of v_log_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Button Click				                 				  		*/
/*---------------------------------------------------------------------------*/
void v_setting_btn_clicked (GtkButton *v_setting_btn, gpointer *data)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	chk_window = 2;

	gtk_widget_hide (vdetect_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of v_setting_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Close Button Click				                            	*/
/*---------------------------------------------------------------------------*/
void v_close_btn_clicked (GtkButton *v_close_btn, gpointer *data)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}

	gtk_widget_hide (GTK_WIDGET (data));
	gtk_widget_show (main_window);

	return;
}
/* end of v_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Window Delete				                            		*/
/*---------------------------------------------------------------------------*/
void v_window_delete (GtkWidget *vdetect_window)
{
	if ((strncmp (chk_psstat, "R", 1) == 0) || (strncmp (chk_dbtnstat, "Y", 1) == 0) || (strncmp (chk_pwstat, "R", 1) == 0) || (strncmp (chk_vdbtnstat, "Y", 1) == 0))
	{
		func_gtk_dialog_modal (0, window, "\n    헌재 검사 진행 중이므로 이동할 수 없습니다.    \n");

		return -1;
	}
	
	gtk_widget_hide (vdetect_window);
	gtk_widget_show (main_window);

	return;
}
/* end of v_window_delete(); function */
/********* End of Vulnerability Window Function ********/



/********* Start of DetectLog Window Function *********/
/*---------------------------------------------------------------------------*/
/* DetectLog TreeView Column variable                    				*/
/*---------------------------------------------------------------------------*/
enum
{
	dl_treeview_num = 0,
	dl_treeview_filename,
	dl_treeview_jcnt,
	dl_treeview_dcnt,
	dl_treeview_fgcnt,
	dl_treeview_pcnt,
	dl_treeview_fstat,
	dl_treeview_fsize,
	dl_treeview_ftype,
	dl_treeview_fileloca,
	dl_treeview_round,
	dl_treeview_workstart,
	dl_treeview_workend,
	DL_NUM_COLS
} ;
/* end of DetectLog TreeView Column variable */


/*---------------------------------------------------------------------------*/
/* DetectLog TreeView Selection 			                                  */
/*---------------------------------------------------------------------------*/
gboolean dl_view_selection_func 	(GtkTreeSelection *selection,
														 GtkTreeModel	   *model,
														 GtkTreePath		  *path,
														 gboolean				 path_currently_selected,
														 gpointer				  userdata)
{
	GtkTreeIter iter;
	gchar *fstat, *fpath, *ftype, *wstart, *wend;
	unsigned int fsize = 0, jcnt = 0, dcnt = 0, fgcnt = 0, pcnt = 0;
	
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, dl_treeview_filename, 	&fname,	 -1);
			gtk_tree_model_get (model, &iter, dl_treeview_jcnt,		    	&jcnt,		 -1);
			gtk_tree_model_get (model, &iter, dl_treeview_dcnt,		   	   &dcnt,	   -1);
			gtk_tree_model_get (model, &iter, dl_treeview_fgcnt,		   &fgcnt,	  -1);
			gtk_tree_model_get (model, &iter, dl_treeview_pcnt,		   	   &pcnt,	   -1);
			gtk_tree_model_get (model, &iter, dl_treeview_fstat,		    &fstat,		 -1);
			gtk_tree_model_get (model, &iter, dl_treeview_fsize,		    &fsize,	     -1);
			gtk_tree_model_get (model, &iter, dl_treeview_ftype,		  &ftype, 	 -1);
			gtk_tree_model_get (model, &iter, dl_treeview_fileloca,	   	   &fpath,	  -1);
			gtk_tree_model_get (model, &iter, dl_treeview_workstart,	&wstart,   -1);
			gtk_tree_model_get (model, &iter, dl_treeview_workend,	   &wend,	 -1);

			// input data in structure
			/* strcpy (sfDs.fname, fname);
			sfDs.jcnt = jcnt;
			sfDs.dcnt = dcnt;
			sfDs.fgcnt = fgcnt;
			sfDs.pcnt = pcnt;
			sfDs.fsize = fsize;
			strcpy (sfDs.fstat, fstat);
			strcpy (sfDs.ftype, ftype);
			strcpy (sfDs.fpath, fpath);
			strcpy (sfDs.start, wstart);
			strcpy (sfDs.end, wend); */
		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	}

	return TRUE; /* allow selection state to change */
}
/* end of dl_view_selection_func(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog TreeModel Create and Fill 			                          */
/*---------------------------------------------------------------------------*/
static GtkTreeModel *dl_create_and_fill_model (void)
{
	GtkTreeIter iter;

	dltreestore = gtk_tree_store_new (DL_NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
																				  G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

	for (int i = 0; i < chk_dldbcol; i++)
	{
		gtk_tree_store_append (dltreestore, &iter, NULL);
		gtk_tree_store_set (dltreestore, &iter,
					  dl_treeview_num, i + 1,
					  dl_treeview_filename,	fDs[i].fname,
					  dl_treeview_jcnt,			fDs[i].jcnt,
					  dl_treeview_dcnt,		   fDs[i].dcnt,
					  dl_treeview_fgcnt,	   fDs[i].fgcnt,
					  dl_treeview_pcnt,		   fDs[i]. pcnt,
					  dl_treeview_fstat,		fDs[i].fstat,
					  dl_treeview_fsize,		fDs[i].fsize,
					  dl_treeview_ftype,	  fDs[i].ftype,
					  dl_treeview_fileloca,	   fDs[i].fpath,
					  dl_treeview_round,	fDs[i].round,
					  dl_treeview_workstart, fDs[i].start,
					  dl_treeview_workend,	fDs[i].end,
					  -1);
	}
	
	return GTK_TREE_MODEL (dltreestore);
}
/* end of dl_create_and_fill_model(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Create View and Model 				                         */
/*---------------------------------------------------------------------------*/
static GtkWidget *dl_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		     *renderer;
	GtkTreeModel		      *model;
	GtkTreeSelection	      *selection;
	
	dl_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_num);

	// --- Column #파일 이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일이름");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_filename);

	// --- Column #주민번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "주민번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_jcnt);

	// --- Column #운전면허 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "운전면허");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_dcnt);
	
	// --- Column #외국인등록번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "외국인등록");
	gtk_tree_view_append_column (GTK_TREE_VIEW(dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_fgcnt);
	
	// --- Column #여권번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "여권번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_pcnt);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_fstat);
	
	// --- Column #파일 크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일크기");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_fsize);

	// --- Column #파일 포맷 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일포맷");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_ftype);
	
	// --- Column #파일 위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일위치");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_fileloca);
	
	// --- Column #회차 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "회차");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_round);

	// --- Column #작업 시작 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업시작");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_workstart);

	// --- Column #작업 종료 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업종료");
	gtk_tree_view_append_column (GTK_TREE_VIEW (dl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", dl_treeview_workend);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dl_view));
	
	gtk_tree_selection_set_select_function (selection, d_view_selection_func, NULL, NULL);

	model = dl_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (dl_view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection (GTK_TREE_VIEW (dl_view)),
							  GTK_SELECTION_SINGLE);

	return dl_view;
}
/* end of dl_create_view_and_model(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Date1 Button Click				                 				  */
/*---------------------------------------------------------------------------*/
void dl_date1_btn_clicked (GtkButton *dl_date1_btn, gpointer *data)
{
	chk_calwindow = 0;
	c_chk_label = 2;
	//gtk_widget_hide (detectlog_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of dl_date1_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Date2 Button Click				                 				  */
/*---------------------------------------------------------------------------*/
void dl_date2_btn_clicked (GtkButton *dl_date2_btn, gpointer *data)
{
	chk_calwindow = 0;
	c_chk_label = 3;
	//gtk_widget_hide (detectlog_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of dl_date2_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Log Button Click				                 				    */
/*---------------------------------------------------------------------------*/
void dl_log_btn_clicked (GtkButton *dl_log_btn, gpointer *data)
{
	gchar *chk_kind_log = 0;
	int chk_date1 = 0, chk_date2 = 0;

	chk_kind_log 		 = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (data));
	chk_date1			  = strcmp ("기간선택", gtk_label_get_text (GTK_LABEL (dl_date1_label)));
	chk_date2			  = strcmp ("기간선택", gtk_label_get_text (GTK_LABEL (dl_date2_label)));

	if (chk_kind_log == NULL ||  chk_date1 == 0 || chk_date2 == 0)
	{
		func_gtk_dialog_modal (0, window, "\n    로그조회 하실 검사항목과 기간을 선택해주세요.    \n");
		return 0;
	}
	else
	{
		if (strcmp (chk_kind_log, "정기검사") == 0)
		{
			//정기검사 로그
			chk_dbtable = 1;
			BXDB_Pop (chk_dbtable);
			dl_Refresh_ScrollWindow();
		}
		else if (strcmp (chk_kind_log, "수동검사") == 0)
		{
			//수동검사 로그
			chk_dbtable = 0;
			BXDB_Pop (chk_dbtable);
			dl_Refresh_ScrollWindow();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    로그조회 하실 검사항목을 선택해주세요.    \n");
		}
	}
}
/* end of dl_log_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Setting Button Click				                 			   */
/*---------------------------------------------------------------------------*/
void dl_setting_btn_clicked (GtkButton *dl_setting_btn, gpointer *data)
{
	chk_window = 3;
	gtk_widget_show (setting_window);
	gtk_widget_hide (detectlog_window);

	return;
}
/* end of dl_setting_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Close Button Click				                 			 	    */
/*---------------------------------------------------------------------------*/
void dl_close_btn_clicked (GtkButton *d_close_btn, gpointer *data)
{
	if (chk_dlwindow == 0)
	{
		gtk_widget_hide (GTK_WIDGET (data));
		gtk_widget_show (detect_window);
	}
	else
	{
		gtk_widget_hide (GTK_WIDGET (data));
		gtk_widget_show (decrypt_window);
	}

	return;
}
/* end of dl_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* DetectLog Window Delete				                 				  	   */
/*---------------------------------------------------------------------------*/
void dl_window_delete (GtkWidget *detectlog_window)
{
	if (chk_dlwindow == 0)
	{
		gtk_widget_hide (detectlog_window);
		gtk_widget_show (detect_window);
	}
	else
	{
		gtk_widget_hide (detectlog_window);
		gtk_widget_show (decrypt_window);
	}

	return;
}
/* end of dl_window_delete(); function */
/********* End of DetectLog Window Function **********/



/******* Start of Vulnerability Log Window Function *******/
/*---------------------------------------------------------------------------*/
/* Vulnerability Log TreeView Column variable                    	  */
/*---------------------------------------------------------------------------*/
enum
{
	vl_treeview_num = 0,
	vl_treeview_item,
	vl_treeview_stat,
	vl_treeview_round,
	vl_treeview_workstart,
	vl_treeview_workend,
	VL_NUM_COLS
} ;
/* end of Vulnerability TreeView Column variable */

/*---------------------------------------------------------------------------*/
/* Vulnerability TreeView Selection 			                                  */
/*---------------------------------------------------------------------------*/
gboolean vl_view_selection_func 	(GtkTreeSelection *selection,
														 GtkTreeModel	   *model,
														 GtkTreePath		  *path,
														 gboolean				 path_currently_selected,
														 gpointer				  userdata)
{
	GtkTreeIter iter;
	gchar *item, *stat, *wstart, *wend;
	unsigned int round = 0;
	
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, vl_treeview_item, 			&item,	 -1);
			gtk_tree_model_get (model, &iter, vl_treeview_stat,		    	&stat,		 -1);
			gtk_tree_model_get (model, &iter, vl_treeview_round,	   	   &round,	  -1);
			gtk_tree_model_get (model, &iter, vl_treeview_workstart,	&wstart,   -1);
			gtk_tree_model_get (model, &iter, vl_treeview_workend,	   &wend,	 -1);
		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	}

	return TRUE; /* allow selection state to change */
}
/* end of vl_view_selection_func(); function */

/*---------------------------------------------------------------------------*/
/* Vulnerability Log TreeModel Create and Fill 			                */
/*---------------------------------------------------------------------------*/
static GtkTreeModel *vl_create_and_fill_model (void)
{
	GtkTreeIter iter;

	vltreestore = gtk_tree_store_new (VL_NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

	for (int i = 0; i < chk_vldbcol; i++)
	{
		gtk_tree_store_append (vltreestore, &iter, NULL);
		gtk_tree_store_set (vltreestore, &iter,
					  vl_treeview_num, i + 1,
					  vl_treeview_item,	viDs[i].item,
					  vl_treeview_stat, viDs[i].stat,
					  vl_treeview_round,	viDs[i].round,
					  vl_treeview_workstart, viDs[i].start,
					  vl_treeview_workend,	viDs[i].end,
					  -1);
	}
	
	return GTK_TREE_MODEL (vltreestore);
}
/* end of vl_create_and_fill_model(); function */


/*---------------------------------------------------------------------------*/
/* Vulnerability Log Create View and Model 				   				*/
/*---------------------------------------------------------------------------*/
static GtkWidget *vl_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		     *renderer;
	GtkTreeModel		      *model;
	GtkTreeSelection	      *selection;
	
	vl_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_num);

	// --- Column #항목 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "검사항목");
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_item);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_stat);
	
	// --- Column #회차 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, " 회차 ");
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_round);

	// --- Column #작업 시작 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업시작");
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_workstart);

	// --- Column #작업 종료 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "작업종료");
	gtk_tree_view_append_column (GTK_TREE_VIEW (vl_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", vl_treeview_workend);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (vl_view));
	
	gtk_tree_selection_set_select_function (selection, vl_view_selection_func, NULL, NULL);

	model = vl_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (vl_view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection (GTK_TREE_VIEW (vl_view)),
							  GTK_SELECTION_SINGLE);

	return vl_view;
}
/* end of vl_create_view_and_model(); function */

/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Date1 Button Click				                 		*/
/*---------------------------------------------------------------------------*/
void vl_date1_btn_clicked (GtkButton *vl_date1_btn, gpointer *data)
{
	chk_calwindow = 1;
	c_chk_label = 4;
	//gtk_widget_hide (vdetectlog_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of vl_date1_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Date2 Button Click				                 		*/
/*---------------------------------------------------------------------------*/
void vl_date2_btn_clicked (GtkButton *vl_date2_btn, gpointer *data)
{
	chk_calwindow = 1;
	c_chk_label = 5;
	//gtk_widget_hide (vdetectlog_window);
	gtk_widget_show (GTK_WIDGET (data));

	return;
}
/* end of vl_date2_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Log Button Click				                 		  */
/*---------------------------------------------------------------------------*/
void vl_log_btn_clicked (GtkButton *vl_detect_btn, gpointer *data)
{
	gchar *chk_kind_log = 0;
	int chk_date1 = 0, chk_date2 = 0;

	chk_kind_log 		 = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (data));
	chk_date1			  = strcmp ("기간선택", gtk_label_get_text (GTK_LABEL (vl_date1_label)));
	chk_date2			  = strcmp ("기간선택", gtk_label_get_text (GTK_LABEL (vl_date2_label)));

	if (chk_kind_log == NULL ||  chk_date1 == 0 || chk_date2 == 0)
	{
		func_gtk_dialog_modal (0, window, "\n    로그조회 하실 검사항목과 기간을 선택해주세요.    \n");
		return 0;
	}
	else
	{
		if (strcmp (chk_kind_log, "정기검사") == 0)
		{
			//정기검사 로그
			chk_dbtable = 3;
			BXDB_Pop (chk_dbtable);
			vl_Refresh_ScrollWindow();
		}
		else if (strcmp (chk_kind_log, "수동검사") == 0)
		{
			//수동검사 로그
			chk_dbtable = 2;
			BXDB_Pop (chk_dbtable);
			vl_Refresh_ScrollWindow();

		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    로그조회 하실 검사항목을 선택해주세요.    \n");
		}
	}
}
/* end of vl_log_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Setting Button Click				                 	*/
/*---------------------------------------------------------------------------*/
void vl_setting_btn_clicked (GtkButton *vl_setting_btn, gpointer *data)
{
	chk_window = 4;
	gtk_widget_show (setting_window);
	gtk_widget_hide (vdetectlog_window);

	return;
}
/* end of vl_setting_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Close Button Click				                 		 */
/*---------------------------------------------------------------------------*/
void vl_close_btn_clicked (GtkButton *vl_close_btn, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));
	gtk_widget_show (vdetect_window);

	return;
}
/* end of vl_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* VulnerabilityLog Window Delete				                 			*/
/*---------------------------------------------------------------------------*/
void vl_window_delete (GtkWidget *vdetectlog_window)
{
	gtk_widget_hide (vdetectlog_window);
	gtk_widget_show (vdetect_window);

	return;
}
/* end of vl_window_delete(); function */
/******* End of VulnerabilityLog Window Function *******/



/********** Start of Calendar Dialog Function ***********/
/*---------------------------------------------------------------------------*/
/* Calendar Day Select		                                    			  		  */
/*---------------------------------------------------------------------------*/
void c_calendar_day_selected (GtkCalendar *c_calendar)
{
	char *real_month[2] = {0,};
	int tmp = 0;

	memset (c_date, 0, sizeof (c_date));
	gtk_calendar_get_date (GTK_CALENDAR (c_calendar), &c_year, &c_month, &c_day);
	tmp = c_month;
	tmp += 1;

	if (c_month < 9)
	{
		sprintf (real_month, "0%d", tmp);
	}
	else
	{
		sprintf (real_month, "%d", tmp);
	}

	sprintf (c_date, "%d-%s-%02d", c_year, real_month, c_day);

	return;
}
/* end of c_calendar_day_selected(); function */


/*---------------------------------------------------------------------------*/
/* Calendar OK Button Click                                    					 */
/*---------------------------------------------------------------------------*/
void c_ok_btn_clicked (GtkButton *c_ok_btn, gpointer *data)
{
	char *datebuf[1024] = {0,};
	int res = 0;
	
	switch (c_chk_label)
	{
		case 2:
			gtk_label_set_text (GTK_LABEL (dl_date1_label), c_date);
			res = 0;

			break;

		case 3:
			gtk_label_set_text (GTK_LABEL (dl_date2_label), c_date);
			res = 0;

			break;

		case 4:
			gtk_label_set_text (GTK_LABEL (vl_date1_label), c_date);
			res = 0;

			break;

		case 5:
			gtk_label_set_text (GTK_LABEL (vl_date2_label), c_date);
			res = 0;

			break;

		case 6:
		
			break;
	}

	if (res == 0)
	{
		gtk_widget_hide (GTK_WIDGET (data));

		switch (chk_calwindow)
		{
			case 0:
				gtk_widget_show (detectlog_window);

				break;
			
			case 1:
				gtk_widget_show (vdetectlog_window);
				
				break;
		}
	}
	

	return;
}
/* end of c_ok_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Calendar Close Button Click                                    				*/
/*---------------------------------------------------------------------------*/
void c_close_btn_clicked (GtkButton *c_close_btn, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	switch (chk_calwindow)
	{
		case 0:
			gtk_widget_show (detectlog_window);
			break;
		
		case 1:
			gtk_widget_show (vdetectlog_window);
			break;
	}

	return;
}
/* end of c_close_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Calendar Dialog Delete                                    					   */
/*---------------------------------------------------------------------------*/
void calendar_dialog_delete()
{
	gtk_widget_hide (calendar_dialog);

	switch (chk_calwindow)
	{
		case 0:
			gtk_widget_show (detectlog_window);
			break;
		
		case 1:
			gtk_widget_show (vdetectlog_window);
			break;
	}

	return;
}
/* end of calendar_dialog_delete function */
/********** End of Calendar Dialog Function ***********/



/*********** Start of Setting Window Function **********/
/*---------------------------------------------------------------------------*/
/* Setting Detect Folder Button Click				                         */
/*---------------------------------------------------------------------------*/
void s_detect_folder_btn_clicked (GtkButton *s_detect_folder_btn,	gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("폴더 선택", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_ACCEPT)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (s_detect_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}

	return;
}
/* end of s_detect_folder_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Setting Move Folder Button Click				                         	 */
/*---------------------------------------------------------------------------*/
void s_move_folder_btn_clicked (GtkButton *s_move_folder_btn,	gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("폴더 선택", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_ACCEPT)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (s_move_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}

	return;
}
/* end of s_move_folder_btn_clicked(); function */

/*---------------------------------------------------------------------------*/
/* Setting Encrypt Folder Button Click				                         */
/*---------------------------------------------------------------------------*/
void s_encrypt_folder_btn_clicked (GtkButton *s_encrypt_folder_btn,	gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("폴더 선택", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_ACCEPT)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (s_encrypt_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}

	return;
}
/* end of s_encrypt_folder_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Setting Decrypt Folder Button Click				                         */
/*---------------------------------------------------------------------------*/
void s_decrypt_folder_btn_clicked (GtkButton *s_decrypt_folder_btn,	gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("폴더 선택", GTK_WINDOW (data),
	GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));
	
	if (resp == GTK_RESPONSE_ACCEPT)
	{
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
		gtk_entry_set_text (s_decrypt_entry, path);
		gtk_widget_destroy (filechooserdialog);
	}

	return;
}
/* end of s_decrypt_folder_btn_clicked(); function */


/*---------------------------------------------------------------------------*/
/* Setting OK Button Click				                         					*/
/*---------------------------------------------------------------------------*/
void s_ok_btn_clicked (GtkButton *s_ok_btn,		gpointer *data)
{
	int chkconf = 0;
	gchar *detect_date = 0;
	gchar *vdetect_date = 0;
	gchar *default_path = 0;
	gchar *move_path = 0;
	gchar *encrypt_path = 0;
	gchar *decrypt_path = 0;
	gchar *ip_etry = 0;
	gchar *port_entry = 0;
	char *conf_path[MAX_PATH] = {0,};
	char *web_uri[2048] = {0,};
	
	sprintf (conf_path, "%s%s", home_path, CONF);

	default_path	= (gchar *) gtk_entry_get_text (s_detect_entry);
	move_path		= (gchar *) gtk_entry_get_text (s_move_entry);
	encrypt_path	= (gchar *) gtk_entry_get_text (s_encrypt_entry);
	decrypt_path	= (gchar *) gtk_entry_get_text (s_decrypt_entry);
	ip_etry				 = (gchar *) gtk_entry_get_text (s_ip_entry);
	port_entry		  = (gchar *) gtk_entry_get_text (s_port_entry);
	
	strcpy (dpDs.detectpath, default_path);
	strcpy (dpDs.movepath, move_path);
	strcpy (dpDs.encpath,  encrypt_path);
	strcpy (dpDs.decpath,  decrypt_path);
	strcpy (cDs.sip, ip_etry);
	strcpy (cDs.sport, port_entry);

	func_CreateDefaultPolicy (1);

	sprintf (web_uri, "https://%s:%s", cDs.sip, cDs.wport); 
	gtk_label_set_text (GTK_LABEL (s_pagelink_label), web_uri);
	gtk_link_button_set_uri (GTK_LINK_BUTTON (s_pagelink_btn), web_uri);

	
	chkconf = func_UpdateConf (conf_path);
	BXLog (DBG, "%-30s	[%d]\n", "BXRC_CONF_UPDATE", chkconf);
	chkconf = func_ParseConf (conf_path);
	BXLog (DBG, "%-30s	[%d]\n", "BXRC_CONF_PARSE", chkconf);

	func_gtk_dialog_modal (0, window, "\n    환경설정이 적용되었습니다.    \n");

	return;
}
/* end of s_ok_btn_clicked function */


/*---------------------------------------------------------------------------*/
/* Setting Close Button Click				                         			   */
/*---------------------------------------------------------------------------*/
void s_close_btn_clicked (GtkButton *setting_window, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	switch (chk_window)
	{
		case 0:
			gtk_widget_show (main_window);
			break;
		
		case 1:
			gtk_widget_show (detect_window);
			break;

		case 2:
			gtk_widget_show (vdetect_window);
			break;

		case 3:
			gtk_widget_show (detectlog_window);
			break;

		case 4:
			gtk_widget_show (vdetectlog_window);
			break;

		case 5:
			gtk_widget_show (decrypt_window);
			break;
	}

	return;
}
/* end of s_close_btn_clicked function */


/*---------------------------------------------------------------------------*/
/* Setting Window Delete										                  */
/*---------------------------------------------------------------------------*/
void s_window_delete (GtkWidget *setting_window)
{
	gtk_widget_hide (setting_window);

	switch (chk_window)
	{
		case 0:
			gtk_widget_show (main_window);
			break;
		
		case 1:
			gtk_widget_show (detect_window);
			break;

		case 2:
			gtk_widget_show (vdetect_window);
			break;

		case 3:
			gtk_widget_show (detectlog_window);
			break;

		case 4:
			gtk_widget_show (vdetectlog_window);
			break;

		case 5:
			gtk_widget_show (decrypt_window);
			break;
	}

	return;
}
/* end of s_window_delete function */
/********** End of Setting Window Function ***********/


/*---------------------------------------------------------------------------*/
/* Plan Dialog Button												                  */
/*---------------------------------------------------------------------------*/
void plan_dialog_btn_clicked (GtkButton *plan_dialog_btn, gpointer *data)
{
	if ((strncmp (pDs.pscheck, "Y", 1) == 0) && (strncmp (pDs.detectpriority, "S", 1) == 0))
	{
		gtk_entry_set_text (GTK_ENTRY (d_detect_entry), pDs.detectpath);
		gtk_widget_hide (plan_dialog);
		strcpy (pDs.pscheck, "N");
		chk_detect = 1; //  정기검사 
		d_detect_btn_clicked();
		chk_detect = 0; // 초기화 (수동검사)
		memset (pDs.detectpriority, 0x00, 2);
		strcpy (pDs.detectpriority, "W");
		func_UpdatePolicy();
	}
	else if ((strncmp (pDs.pwcheck, "Y", 1) == 0) && (strncmp (pDs.detectpriority, "W", 1) == 0))
	{
		gtk_widget_hide (plan_dialog);
		strcpy (pDs.pwcheck, "N");
		chk_vdetect = 1; //  정기검사 
		v_detect_btn_clicked();
		chk_vdetect = 0; // 초기화 (수동검사)
		memset (pDs.detectpriority, 0x00, 2);
		strcpy (pDs.detectpriority, "S");
		func_UpdatePolicy();
	}
}
/* end of plan_dialog_btn_clicked function */


/*---------------------------------------------------------------------------*/
/* Common Dialog Button												            */
/*---------------------------------------------------------------------------*/
void common_dialog_btn_clicked (GtkButton *common_dialog_btn, gpointer *data)
{
	gtk_widget_hide (data);

}
/* end of common_dialog_btn_clicked function */



/* void func_btime()
{
	struct stat fstat;

	stat ("/test", &fstat);

	printf ("inode stat: [%d]\n", fstat.st_ino);
	printf ("mtime stat: [%d]\n", fstat.st_mtime);
	printf ("ctime stat: [%d]\n\n", fstat.st_ctime);

	DIR *dp;
	char filepath[MAX_PATH] = {"/"};
	//int atflag = AT_SYMLINK_NOFOLLOW;
	unsigned int mask = STATX_ALL;
	

	if ((dp = opendir(filepath)) == NULL)
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_FOLDER_OPEN_ERROR", path);
		return -1;
	}

	struct statx stx;

	//statx (dp, "/test", atflag, mask, &stx);
	printf ("inode statx: [%d]\n", stx.stx_ino);
	printf ("btime statx: [%d]\n", stx.stx_btime);
	printf ("mtime statx: [%d]\n", stx.stx_mtime);
	printf ("ctime statx: [%d]\n", stx.stx_ctime);

} */

static gboolean timer (gpointer user_data)
{
	GTimeVal tm;
	g_get_current_time (&tm);
	printf (" a   \n");
	gdk_pixbuf_animation_iter_advance (aniter, &tm);
	printf ("  b  \n");
	g_timeout_add (gdk_pixbuf_animation_iter_get_delay_time (aniter), timer, NULL);
	printf ("   c \n");
	gtk_widget_queue_draw (st_drawing_area);
	printf ("  d  \n");

	return FALSE;
}

/* This is called when we need to draw the windows contents */
static gboolean on_st_drawing_area_draw (GtkWidget *widget,  cairo_t *cr, gpointer userdata)
{
	guint width, height;
	GdkColor color;
	/* GtkStyleContext *context;
	

	context = gtk_widget_get_style_context (widget);

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);

	gtk_render_background (context, cr, 0, 0, width, height);

	cairo_arc (cr,
				width / 2.0, height / 2.0,
				MIN (width, height) / 2.0,
				0, 2 * G_PI);

	gtk_style_context_get_color (context,
								gtk_style_context_get_state (context),
								&color);
	gdk_cairo_set_source_rgba (cr, &color);

	cairo_fill (cr);

	return FALSE; */

	/* GdkPixbuf *pixbuf;
	printf ("  1  \n");
	cr = gdk_cairo_create (widget);
	printf ("  2  \n");

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

	printf ("  3  \n");
	pixbuf = gdk_pixbuf_animation_iter_get_pixbuf (aniter);

	printf ("  4  \n");
	gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
	printf ("  5  \n");
	cairo_paint (cr);
	printf ("  6  \n");

    cairo_destroy(cr);
    return FALSE; */
}

/*---------------------------------------------------------------------------*/
/* MAIN                                                     								  */
/*---------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
	GTimeVal tm;
	GdkColor color;
	int chkconf, threaderr;	
	char *glade_path[1024] = {0,};
	char *web_uri_login[1024] = {0,};
	char *web_uri[1024] = {0,};
	char *conf_path[1024] = {0,};
	//char *vdatebuf[40] = {0, };
	pthread_t hb, ps, pw;
	pthread_attr_t hbattr, psattr, pwattr;

	gtk_init (&argc, &argv);

	e_webview = WEBKIT_WEB_VIEW (webkit_web_view_new());
	c_calendar = GTK_CALENDAR (gtk_calendar_new());

	builder = gtk_builder_new();

	LogName = basename (argv[0]); // basename (argv[0]): 디렉토리 제외한 파일이름
	
	func_GetHomePath(); // 사용자 HomePath
	func_Uuid();				// 사용자 UUID
	func_ChkConf();		// 사용자 ConfFile Check

	sprintf (policy_path, "%s%s", home_path, POLICY);

	func_ChkPolicy();		// Policy Check

	sprintf (set_nng_server, "tcp://%s:%s", cDs.sip, cDs.sport);
	sprintf (web_uri, "https://%s:%s", cDs.sip, cDs.wport); 
	sprintf (web_uri_login, "%s/client/created?uuid=%s", web_uri, uDs.uuid); 
	sprintf (glade_path, "%s", dpDs.gladepath); // 정책에서 내려오면 그 값으로 바꿔주는거 추가해야함, 값 정하고 추가 예정
	
	gtk_builder_add_from_file(builder, glade_path, NULL);
	
	main_window				 = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	enrollment_window	 = GTK_WIDGET (gtk_builder_get_object (builder, "enrollment_window"));
	detect_window			= GTK_WIDGET (gtk_builder_get_object (builder, "detect_window"));
	vdetect_window		   = GTK_WIDGET (gtk_builder_get_object (builder, "vdetect_window"));
	detectlog_window	  = GTK_WIDGET (gtk_builder_get_object (builder, "detectlog_window"));
	vdetectlog_window	 = GTK_WIDGET (gtk_builder_get_object (builder, "vdetectlog_window"));
	decrypt_window		   = GTK_WIDGET (gtk_builder_get_object (builder, "decrypt_window"));
	setting_window			= GTK_WIDGET (gtk_builder_get_object (builder, "setting_window"));
	start_window			 = GTK_WIDGET (gtk_builder_get_object (builder, "start_window"));

	dec_progressbar			  = GTK_WIDGET (gtk_builder_get_object (builder, "dec_progressbar"));
	dec_scrolledwindow	  = GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "dec_scrolledwindow"));
	dec_detect_entry		 = GTK_WIDGET (gtk_builder_get_object (builder, "dec_detect_entry"));

	d_progressbar			   = GTK_WIDGET (gtk_builder_get_object (builder, "d_progressbar"));
	d_scrolledwindow	   = GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "d_scrolledwindow"));
	d_detect_entry			  = GTK_WIDGET (gtk_builder_get_object (builder, "d_detect_entry"));

	v_scrolledwindow	   = GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "v_scrolledwindow"));

	m_version_label			 = GTK_WIDGET (gtk_builder_get_object (builder, "m_version_label"));
	m_userinfo_label		= GTK_WIDGET (gtk_builder_get_object (builder, "m_userinfo_label"));
	m_combobox				= GTK_WIDGET (gtk_builder_get_object (builder, "m_combobox"));

	e_webview				   = GTK_WIDGET (gtk_builder_get_object (builder, "e_webview"));

	s_detect_entry			  = GTK_WIDGET (gtk_builder_get_object (builder, "s_detect_entry"));
	s_move_entry			  = GTK_WIDGET (gtk_builder_get_object (builder, "s_move_entry"));
	s_encrypt_entry			  = GTK_WIDGET (gtk_builder_get_object (builder, "s_encrypt_entry"));
	s_decrypt_entry			  = GTK_WIDGET (gtk_builder_get_object (builder, "s_decrypt_entry"));
	s_ip_entry					  = GTK_WIDGET (gtk_builder_get_object (builder, "s_ip_entry"));
	s_port_entry				= GTK_WIDGET (gtk_builder_get_object (builder, "s_port_entry"));
	s_pagelink_label		 = GTK_WIDGET (gtk_builder_get_object (builder, "s_pagelink_label"));
	s_pagelink_btn		 	 = GTK_WIDGET (gtk_builder_get_object (builder, "s_pagelink_btn"));

	calendar_dialog			 = GTK_WIDGET (gtk_builder_get_object (builder, "calendar_dialog"));
	c_calendar					= GTK_WIDGET (gtk_builder_get_object (builder, "c_calendar"));

	dl_scrolledwindow	 = GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "dl_scrolledwindow"));
	dl_date1_label			 = GTK_WIDGET (gtk_builder_get_object (builder, "dl_date1_label"));
	dl_date2_label			 = GTK_WIDGET (gtk_builder_get_object (builder, "dl_date2_label"));

	v_status_label				= GTK_WIDGET (gtk_builder_get_object (builder, "v_status_label"));
	v_item_label				= GTK_WIDGET (gtk_builder_get_object (builder, "v_item_label"));
	v_safe_label				= GTK_WIDGET (gtk_builder_get_object (builder, "v_safe_label"));
	v_vurnelability_label	= GTK_WIDGET (gtk_builder_get_object (builder, "v_vurnelability_label"));
	v_imposiible_label		= GTK_WIDGET (gtk_builder_get_object (builder, "v_impossible_label"));
	v_date_label				 = GTK_WIDGET (gtk_builder_get_object (builder, "v_date_label"));
	v_progressbar			   = GTK_WIDGET (gtk_builder_get_object (builder, "v_progressbar"));

	vl_scrolledwindow	 = GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "vl_scrolledwindow"));
	vl_date1_label			  = GTK_WIDGET (gtk_builder_get_object (builder, "vl_date1_label"));
	vl_date2_label			  = GTK_WIDGET (gtk_builder_get_object (builder, "vl_date2_label"));

	plan_dialog			 		= GTK_WIDGET (gtk_builder_get_object (builder, "plan_dialog"));
	plan_dialog_label		= GTK_WIDGET (gtk_builder_get_object (builder, "plan_dialog_label"));

	st_label						= GTK_WIDGET (gtk_builder_get_object (builder, "st_label"));
	st_version_label			= GTK_WIDGET (gtk_builder_get_object (builder, "st_version_label"));
	st_spinner					= GTK_WIDGET (gtk_builder_get_object (builder, "st_spinner"));
	//st_drawing_area	 		= GTK_DRAWING_AREA (gtk_builder_get_object (builder, "st_drawing_area"));

	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (enrollment_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (detect_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position	(GTK_WINDOW (vdetect_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (detectlog_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position	(GTK_WINDOW (vdetectlog_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position	(GTK_WINDOW (decrypt_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (setting_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (start_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (calendar_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (plan_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	gdk_color_parse ("#FFFFFF", &color);
	gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (enrollment_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (detect_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (vdetect_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (detectlog_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (vdetectlog_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (decrypt_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (setting_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (start_window, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (calendar_dialog, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg (plan_dialog, GTK_STATE_NORMAL, &color);

	// Setting Widown 값 설정
	gtk_entry_set_text (GTK_ENTRY (d_detect_entry), dpDs.detectpath);
	gtk_entry_set_text (GTK_ENTRY (s_detect_entry), dpDs.detectpath);
	gtk_entry_set_text (GTK_ENTRY (s_move_entry), dpDs.movepath);
	gtk_entry_set_text (GTK_ENTRY (s_encrypt_entry), dpDs.encpath);
	gtk_entry_set_text (GTK_ENTRY (s_decrypt_entry), dpDs.decpath);
	gtk_label_set_text (GTK_LABEL (m_version_label), VERSION);
	gtk_label_set_text (GTK_LABEL (st_version_label), VERSION);
	gtk_entry_set_text (GTK_ENTRY (s_ip_entry), cDs.sip);
	gtk_entry_set_text (GTK_ENTRY (s_port_entry), cDs.sport);
	gtk_label_set_text (GTK_LABEL (s_pagelink_label), web_uri);
	gtk_link_button_set_uri (GTK_LINK_BUTTON (s_pagelink_btn), web_uri);
	//sprintf (vdatebuf, "%s (%d 회차)",  vDs.end, pDs.pwround);
	//gtk_label_set_text (GTK_LABEL (v_date_label), vdatebuf);
	// Setting Window 값 설정 끝
	
	/* printf ("  @  \n");
	st_drawing_area = gtk_drawing_area_new();
	printf ("  @@  \n");
	pixbuf_animation = gdk_pixbuf_animation_new_from_file ("/home/bluexg/.bxrG/b.gif", NULL);

	g_get_current_time (&tm);
	printf ("  @@@  \n");
	aniter = gdk_pixbuf_animation_get_iter (pixbuf_animation, &tm);
	timer (NULL);
	printf ("  @@@@  \n");

	gtk_widget_set_app_paintable (st_drawing_area, TRUE);
	
	printf ("  @@@@@  \n");
	g_signal_connect (G_OBJECT (st_drawing_area), "draw", G_CALLBACK (on_st_drawing_area_draw), NULL);
	printf ("  @@@@@@ \n"); */

	gtk_builder_connect_signals (builder, NULL);
	g_object_unref (builder);
	
	memset (chk_online, 0x00, strlen (chk_online));
	strcpy (chk_online, "Y"); // 초기 구동시 온라인 모드로 설정하고 실행
	strcpy (chk_fheart, "Y"); // 첫 HeartBeat Check
	strcpy (chk_fnonoff, "N"); // 초기 구동시 N으로 설정하고 실행
	chk_detect = 0; // 초기 구동시 민감정보 수동검사로 설정하고 실행
	chk_vdetect = 0; // 초기 구동시 취약점 수동검사로 설정하고 실행
	for (int i = 0; i < 10; i ++) // 취약점 검사 화면 초기 설정
	{
		strcpy( viDs[i].result, "미수행");
		strcpy (viDs[i].stat, "미수행");
	}

	func_set_display();

	func_nng_socket (set_nng_server);

	func_Initiate();		// Initiate Client

	//gtk_widget_show (main_window);			// 메인 창
	//gtk_widget_show (start_window);


	/* pthread_attr_init (&hbattr);
	pthread_attr_setdetachstate (&hbattr, PTHREAD_CREATE_DETACHED);

	if (threaderr = pthread_create(&hb, &hbattr, func_heartbeat_thread, NULL))
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_HEART_THREAD_ERROR", strerror (threaderr));
	}

	pthread_attr_init (&psattr);
	pthread_attr_setdetachstate (&psattr, PTHREAD_CREATE_DETACHED);
	if (threaderr = pthread_create(&ps, &psattr, func_sensitive_thread, NULL))
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_SENSITIVE_THREAD_ERROR", strerror (threaderr));
	}

	pthread_attr_init (&pwattr);
	pthread_attr_setdetachstate (&pwattr, PTHREAD_CREATE_DETACHED);
	if (threaderr = pthread_create(&pw, &pwattr, func_warning_thread, NULL))
	{
		BXLog (DBG, "%-30s	[%s]\n", "BXR_WARNING_THREAD_ERROR", strerror (threaderr));
	} */
	gtk_main();

	return 0;
}
/* END OF MAIN */