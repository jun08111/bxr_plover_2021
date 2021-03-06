INSERT INTO bluexg_v2.tb_info_manager (userId, userPwd, userName, rankCode, positionCode, deptCode, officeTel, officeAddrCode, officeAddrDetail, phone, email, homeTel, homeAddrCode, homeAddrDetail, cDate, uDate, status) VALUES('admin', '12qw12qw`', '관리자', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

INSERT INTO `tb_info_dept` (`deptCode`,`deptName`,`pDeptCode`,`cDate`,`inOrder`,`status`) VALUES ('-1','미확인 부서','00000000',now(3),0,NULL);
INSERT INTO `tb_info_dept` (`deptCode`,`deptName`,`pDeptCode`,`cDate`,`inOrder`,`status`) VALUES ('00000000','조은아이앤에스','0',now(3),0,NULL);

INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('CHA1', 'C001', '비밀번호 변경', 'pwdChange', NULL, NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('HIS1', 'H001', '로그인', 'login', NULL, NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('HIS1', 'H002', '로그아웃', 'logout', NULL, NULL);

INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('MDT1', 'M001', '웹', 'Web', NULL, '매체종류(웹)');
INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('MDT1', 'M002', '클라이언트', 'Client', NULL, '매체종류(클라이언트)');
INSERT INTO bluexg_v2.tb_info_code (pKey, cKey, kValue, eValue, inOrder, info) VALUES('MDT1', 'M003', '서버', 'Server', NULL, '매체종류(서버)');

INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC1', 'N001', '공지사항', 'Notice');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC2', 'F001', '관리콘솔', 'FAQ Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC2', 'F002', '클라이언트', 'FAQ Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC2', 'F003', '기타', 'FAQ Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC3', 'D001', '문서', 'Data Categofy');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC3', 'D002', '설치', 'Data Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC3', 'D003', '이미지', 'Data Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC3', 'D004', '기타', 'Data Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC4', 'Q001', '관리콘솔', 'QNA Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC4', 'Q002', '클라이언트', 'QNA Category');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`, info) VALUES('NTC4', 'Q003', '기타', 'QNA Category');

INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C001','계정 원격접속 제한','Account remote access restrictions',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C002','패스워드 복잡성 설정','Set password complexity',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C003','계정 잠금 임계값 설정','Account Lockout Threshold Settings',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C004','패스워드 파일 보호','Password file protection',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C005','root홈, 패스 디렉터리 권한 및 패스 설정','Root home, path directory',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C006','파일 및 디렉터리 소유자 설정','Set file and directory owners',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C007','로그인 패스워드 안전성 여부 점검','Check if the login password is secure',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C008','로그인 패스워드의 분기 1회 이상 변경 여부 점검','Change login password once a quarter',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C009','화면보호기 설정 여부 점검','Check whether the screen saver is set',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLC','C010','사용자 공유 폴더 설정 여부 점검','Check whether user shared folder is set',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F001','파워포인트','ppt',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F002','파워포인트','pptx',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F003','한글','hwp',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F004','PDF','pdf',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F005','워드','doc',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F006','워드','docx',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F007','엑셀','xls',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F008','엑셀','xlsx',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLF','F009','텍스트','txt',NULL,NULL,NULL,'사용');
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N001','주민등록번호','jumin',NULL,'','(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))([\\s\\D]{0,3})[1-4][0-9]{6}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N002','외국인등록번호','foreigner',NULL,NULL,'(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))([\\s\\D]{0,3})[5-8][0-9]{6}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N003','여권번호','passport',NULL,'','(?lt;=[^0-9a-zA-Z])([M|S|R|O|D|m|s|r|o|d][0-9]{8})(?=[^0-9a-zA-Z])',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N004','운전면허번호','driver',NULL,'','([가-힙]{2})([\\s\\D]{2,3})\\d{2}(\\s|-)?\\d{6}(\\s|-)?\\d{2}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N005','카드번호','card',NULL,'','(\\d{4}[\\s-:\\.])(\\d{4}[\\s-:\\.])(\\d{4}[\\s-:\\.])(\\d{4})',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N006','사업자등록번호','biz',NULL,'','(?:[0-9]{3})([\\s|-])([0-9]{2})([\\s|-])[0-9]{5}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N007','전화번호','tel',NULL,'','[0][0-9]{1,2}-\\d{3,4}-\\d{4}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N008','핸드폰번호','cell',NULL,'','[01]{3}-\\d{3,4}-\\d{4}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N009','통장번호','bank',NULL,'','[0-9{4}-[0-9]{3}-[0-9]{6}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N010','E-Mail','email',NULL,'','[0-9a-zA-Z]([-_.]?[0-9a-zA-Z]){0,100}@[0-9a-zA-Z]([-_.]?[0-9a-zA-Z]){0,100}.[a-zA-Z]{2,3}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N011','법인번호','corp',NULL,'','(?:[0-9]{4}(?:[1-9]{2}))([\\s|-])[0-9]{7}',NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N012','사용자정의패턴1','custom1',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N013','사용자정의패턴2','custom2',NULL,'',NULL,NULL);
INSERT INTO bluexg_v2.tb_info_code (pKey,cKey,kValue,eValue,inOrder,info,regex,fileFlag) VALUES ('POLN','N014','사용자정의패턴3','custom3',NULL,'',NULL,NULL);

INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R001', '회장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R002', '부회장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R003', '사장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R004', '부사장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R005', '전무');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R006', '상무');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R007', '이사');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R008', '부장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R009', '차장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R010', '과장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R011', '대리');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R012', '주임');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R013', '사원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R014', '인턴');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R015', '수석연구원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R016', '책임연구원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R017', '전임연구원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R018', '주임연구원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('RNK1', 'R019', '연구원');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('POS1', 'P001', '본부장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('POS1', 'P002', '팀장');
INSERT INTO `bluexg_v2`.`tb_info_code` (`pKey`, `cKey`, `kValue`) VALUES ('POS1', 'P003', '파트장');

INSERT INTO bluexg_v2.tb_cfg_bxrg (`optKey`, `optValue`, `optFlag`) VALUES ('CLIENT_VER', '', 'N');
