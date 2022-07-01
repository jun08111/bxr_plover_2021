#include <stdio.h>
#include <stdint.h>

/* J:Json, F:Format, S:Sync, A:Async, C:Create, R:Read, U:Update, D:Delete */
/* JF + S or A + C or R or U or D + 4 Char Job */ /* Total 8 Char */ 
//JFSCFPCK: File Passively  Check  민감정보 수동검사
//JFSCFRCK: File Regularly Check  민감정보 정기검사

#define JFSRINIT   "{\
                                \"Head\" : {\
                                                        \"Func\" : \"JFSRINIT\",\
                                                        \"RqRp\" : \"%s\",\
                                                        \"DateTime\" : %d,\
                                                        \"Uuid\" : \"%s\",\
                                                        \"AppVer\": \"%s\"\
                                                        },\
                                \"Data\" : {\
                                                        \"eType\" : \"%s\",\
                                                        \"eCode\" : %03d,\
                                                        \"eMesg\" : \"%s\",\
                                                        \"uID\": \"%s\",\
                                                        \"uPwd\": \"%s\",\
                                                        \"uName\": \"%s\",\
                                                        \"uRankNm\": \"%s\",\
                                                        \"uDeptNm\": \"%s\",\
                                                        }\
                                }";


#define JFSCFRCK   "{\
                                    \"Head\" : {\
                                                            \"Func\" : \"JFSCFRCK\",\
                                                            \"RqRp\" : \"%s\",\
                                                            \"DateTime\" : %d,\
                                                            \"Uuid\" : \"%s\",\
                                                            \"AppVer\": \"%s\"\
                                                            },\
                                    \"Data\" : {\
                                                            \"eType\" : \"%s\",\
                                                            \"eCode\" : %d,\
                                                            \"eMesg\" : \"%s\",\
                                                            \"FileName\" : \"%s\",\
                                                            \"J\" : %d,\
                                                            \"D\" : %d,\
                                                            \"P\" : %d,\
                                                            \"F\" : %d,\
                                                            \"FileStat\" : \"%s\",\
                                                            \"FileSize\" : %d,\
                                                            \"FileType\" : \"%s\",\
                                                            \"WorkStart\" : \"%s\",\
                                                            \"WorkEnd\" : \"%s\"\
                                                            }\
                                }";


#define JFSCFPCK   "{\
                                    \"Head\" : {\
                                                            \"Func\" : \"JFSCFPCK\",\
                                                            \"RqRp\" : \"%s\",\
                                                            \"DateTime\" : %d,\
                                                            \"Uuid\" : \"%s\",\
                                                            \"AppVer\": \"%s\"\
                                                            },\
                                    \"Data\" : {\
                                                            \"eType\" : \"%s\",\
                                                            \"eCode\" : %d,\
                                                            \"eMesg\" : \"%s\",\
                                                            \"FileCount\" : %d,\
                                                            \"PatternCount\" : %d,\
                                                            \"WorkStart\" : \"%s\",\
                                                            \"WorkEnd\" : \"%s\"\
                                                            }\
                                }";