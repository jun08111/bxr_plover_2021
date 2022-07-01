-- bluexg_v2 Database Create Script 20210802

CREATE DATABASE `bluexg_v2` /*!40100 DEFAULT CHARACTER SET utf8mb3 */;


-- bluexg_v2 User Add

CREATE USER 'bxrgadm'@'%' IDENTIFIED BY 'Whdms9500!'; 
CREATE USER 'bxrgweb'@'%' IDENTIFIED BY 'Whdms9500!'; 
CREATE USER 'bxrgsvr'@'%' IDENTIFIED BY 'Whdms9500!'; 


-- bluexg_v2 Policy Add

GRANT ALL PRIVILEGES ON bluexg_v2.* TO 'bxrgadm'@'%';
GRANT ALL PRIVILEGES ON bluexg_v2.* TO 'bxrgweb'@'%';
GRANT ALL PRIVILEGES ON bluexg_v2.* TO 'bxrgsvr'@'%'; 

FLUSH PRIVILEGES;

USE bluexg_v2;


-- bluexg_v2.tb_board_client definition

CREATE TABLE `tb_board_client` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `title` varchar(45) NOT NULL COMMENT 'Title',
  `content` mediumtext NOT NULL COMMENT 'Content',
  `version` varchar(45) DEFAULT NULL COMMENT 'Client Version',
  `userId` varchar(45) NOT NULL COMMENT 'userId',
  `hitCount` int(11) DEFAULT 0 COMMENT 'Client HitCount',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` int(11) NOT NULL COMMENT 'Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_board_clientFile definition

CREATE TABLE `tb_board_clientFile` (
  `fileIdx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'File Index',
  `idx` int(11) NOT NULL COMMENT 'Index',
  `orgFileName` varchar(400) NOT NULL COMMENT 'Original FileName',
  `saveFileName` varchar(400) NOT NULL COMMENT 'Save FileName',
  `fileSize` int(11) NOT NULL COMMENT 'File Size',
  PRIMARY KEY (`fileIdx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_board_notice definition

CREATE TABLE `tb_board_notice` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Notice Index',
  `type` varchar(45) DEFAULT NULL COMMENT 'Notice Type',
  `code` varchar(45) DEFAULT NULL COMMENT 'Notice Code',
  `userId` varchar(45) NOT NULL COMMENT 'UserId',
  `title` varchar(45) NOT NULL COMMENT 'Notice Title',
  `content` mediumtext NOT NULL COMMENT 'Notice Content',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `hitCount` int(11) DEFAULT 0 COMMENT 'Notice HitCount',
  `status` int(11) NOT NULL COMMENT 'Notice Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_board_noticeFile definition

CREATE TABLE `tb_board_noticeFile` (
  `fileIdx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Notice File Index',
  `idx` int(11) DEFAULT NULL COMMENT 'Notice Index',
  `orgFileName` varchar(400) NOT NULL COMMENT 'File OriginalFileName',
  `saveFileName` varchar(400) NOT NULL COMMENT 'File SaveFileName',
  `fileSize` int(11) DEFAULT NULL COMMENT 'File Size',
  PRIMARY KEY (`fileIdx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

-- bluexg_v2.tb_board_qna definition

CREATE TABLE `tb_board_qna` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Qna Index',
  `type` varchar(10) DEFAULT NULL COMMENT 'Notice Type',
  `code` varchar(45) DEFAULT NULL COMMENT 'Notice Code',
  `userId` varchar(45) NOT NULL COMMENT 'UserId',
  `title` varchar(45) NOT NULL COMMENT 'Title',
  `content` mediumtext NOT NULL COMMENT 'Content',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `private` int(11) DEFAULT NULL COMMENT 'Public/Private',
  `pIdx` int(11) DEFAULT NULL COMMENT 'pIdx',
  `status` int(11) NOT NULL COMMENT 'Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_cfg_bxrg definition

CREATE TABLE `tb_cfg_bxrg` (
  `optKey` varchar(16) NOT NULL COMMENT 'Option Key',
  `optValue` varchar(16) DEFAULT NULL COMMENT 'Option Value',
  `optFlag` varchar(1) DEFAULT NULL COMMENT 'Option User Flag',
  `optInfo` varchar(128) DEFAULT NULL COMMENT 'Option Comment',
  PRIMARY KEY (`optKey`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_client definition

CREATE TABLE `tb_info_client` (
  `uuid` varchar(64) NOT NULL COMMENT 'User Uniq ID',
  `cType` varchar(1) DEFAULT NULL COMMENT 'Client type N:Normal, M:Multi User',
  `os` varchar(45) DEFAULT NULL COMMENT 'OS Info',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` varchar(1) DEFAULT NULL COMMENT 'Client Status',
  PRIMARY KEY (`uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_code definition

CREATE TABLE `tb_info_code` (
  `pKey` varchar(4) NOT NULL COMMENT 'Parents Key',
  `cKey` varchar(16) NOT NULL COMMENT 'Child Key',
  `kValue` varchar(45) DEFAULT NULL COMMENT 'Korean Value',
  `eValue` varchar(45) DEFAULT NULL COMMENT 'English Value',
  `inOrder` int(11) DEFAULT NULL,
  `info` varchar(45) DEFAULT NULL COMMENT 'Information',
  `regex` varchar(255) DEFAULT NULL COMMENT 'Policy Regex',
  `fileFlag` varchar(4) DEFAULT NULL COMMENT 'File Flag',
  PRIMARY KEY (`pKey`,`cKey`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_dept definition

CREATE TABLE `tb_info_dept` (
  `deptCode` varchar(32) NOT NULL COMMENT 'Dept Code',
  `deptName` varchar(64) DEFAULT NULL COMMENT 'Dept Name',
  `pDeptCode` varchar(32) DEFAULT NULL COMMENT 'Parent Dept Code',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `inOrder` int(11) DEFAULT NULL COMMENT 'Data Sorting',
  `status` int(11) DEFAULT NULL COMMENT 'Status',
  PRIMARY KEY (`deptCode`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_join definition

CREATE TABLE `tb_info_join` (
  `state` varchar(1) NOT NULL COMMENT 'State New:C, Update:U, Delete:D',
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `uuid` varchar(64) NOT NULL COMMENT 'Universal Unique Identifier',
  `cDate` datetime(3) NOT NULL COMMENT 'Create Datetime',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Datetime',
  `eDate` datetime(3) DEFAULT NULL COMMENT 'Expiry Datetime',
  PRIMARY KEY (`state`,`userId`,`uuid`,`cDate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_manager definition

CREATE TABLE `tb_info_manager` (
  `userId` varchar(45) NOT NULL COMMENT 'Manager ID',
  `userPwd` varchar(64) DEFAULT NULL COMMENT 'Manager Password',
  `userName` varchar(45) DEFAULT NULL COMMENT 'Manager Name',
  `rankCode` varchar(32) DEFAULT NULL COMMENT 'Rank Code',
  `positionCode` varchar(32) DEFAULT NULL COMMENT 'Position Code',
  `deptCode` varchar(32) DEFAULT NULL COMMENT 'Dept Code',
  `officeTel` varchar(45) DEFAULT NULL COMMENT 'Office Tel',
  `officeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Office Address Code',
  `officeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Office Address Detail',
  `phone` varchar(45) DEFAULT NULL COMMENT 'Phone Number',
  `email` varchar(45) DEFAULT NULL COMMENT 'Email',
  `homeTel` varchar(45) DEFAULT NULL COMMENT 'Home Tel',
  `homeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Home Address Code',
  `homeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Home Address Detail',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` int(11) DEFAULT NULL COMMENT 'Status',
  PRIMARY KEY (`userId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_policy definition

CREATE TABLE `tb_info_policy` (
  `policyCode` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Policy Code',
  `policyName` varchar(45) NOT NULL COMMENT 'Policy Name',
  `policyContent` varchar(255) NOT NULL COMMENT 'Policy Content',
  `pKey` varchar(16) DEFAULT NULL COMMENT 'Parents Key',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  PRIMARY KEY (`policyCode`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_policySetting definition

CREATE TABLE `tb_info_policySetting` (
  `policyCode` int(11) NOT NULL COMMENT 'Policy Code',
  `policyCodeName` varchar(4) DEFAULT NULL COMMENT 'Policy Code Name',
  `policyFlag` varchar(4) DEFAULT NULL COMMENT 'Policy Flag',
  `policyCnt` int(11) DEFAULT NULL COMMENT 'Policy Count'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_info_user definition

CREATE TABLE `tb_info_user` (
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `userPwd` varchar(64) DEFAULT NULL COMMENT 'User Password',
  `userName` varchar(45) DEFAULT NULL COMMENT 'User Name',
  `rankCode` varchar(32) DEFAULT NULL COMMENT 'Rank Code',
  `positionCode` varchar(32) DEFAULT NULL COMMENT 'Position Code',
  `deptCode` varchar(32) DEFAULT NULL COMMENT 'Dept Code',
  `officeTel` varchar(45) DEFAULT NULL COMMENT 'Office Tel',
  `officeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Office Address Code',
  `officeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Office Address Detail',
  `phone` varchar(45) DEFAULT NULL COMMENT 'Phone Number',
  `email` varchar(45) DEFAULT NULL COMMENT 'Email',
  `homeTel` varchar(45) DEFAULT NULL COMMENT 'Home Tel',
  `homeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Home Address Code',
  `homeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Home Address Detail',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` int(11) DEFAULT NULL COMMENT 'Status',
  PRIMARY KEY (`userId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_inspect_secu_plan definition

CREATE TABLE `tb_inspect_secu_plan` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `rDate` datetime(3) DEFAULT NULL COMMENT 'Registration date',
  `sDate` datetime(3) DEFAULT NULL COMMENT 'Start Inspection',
  `eDate` datetime(3) DEFAULT NULL COMMENT 'End Inspection',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `rNum` int(11) DEFAULT NULL COMMENT 'Round Num',
  `secuNum` varchar(16) DEFAULT NULL COMMENT 'Security Number',
  `secuStat` varchar(45) DEFAULT NULL COMMENT 'Security Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_inspect_secu_user definition

CREATE TABLE `tb_inspect_secu_user` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `rDate` datetime(3) DEFAULT NULL COMMENT 'Registration date',
  `sDate` datetime(3) DEFAULT NULL COMMENT 'Start Inspection',
  `eDate` datetime(3) DEFAULT NULL COMMENT 'End Inspection',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `detectCnt` int(64) DEFAULT NULL COMMENT 'Detection Count',
  `safeCnt` int(64) DEFAULT NULL COMMENT 'Safe Count',
  `warnCnt` int(64) DEFAULT NULL COMMENT 'Warn Count',
  `impossibleCnt` int(64) DEFAULT NULL COMMENT 'Impossible Count',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_inspect_sens_plan definition

CREATE TABLE `tb_inspect_sens_plan` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `rDate` datetime(3) DEFAULT NULL COMMENT 'Registration date',
  `sDate` datetime(3) DEFAULT NULL COMMENT 'Start Inspection',
  `eDate` datetime(3) DEFAULT NULL COMMENT 'End Inspection',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `fileName` varchar(255) DEFAULT NULL COMMENT 'FileName',
  `fileStat` varchar(100) DEFAULT NULL COMMENT 'File Status',
  `fileSize` int(255) DEFAULT NULL COMMENT 'File Size',
  `fileType` varchar(255) DEFAULT NULL COMMENT 'File Type',
  `rNum` int(11) DEFAULT NULL COMMENT 'Round Num',
  `jumin` int(64) DEFAULT NULL COMMENT 'Resident registration number',
  `foreigner` int(64) DEFAULT NULL COMMENT 'Foreigner registration number',
  `passport` int(64) DEFAULT NULL COMMENT 'Passport number',
  `driver` int(64) DEFAULT NULL COMMENT 'Driver license',
  `card` int(64) DEFAULT NULL COMMENT 'Card number',
  `biz` int(64) DEFAULT NULL COMMENT 'Business number',
  `tel` int(64) DEFAULT NULL COMMENT 'Telephone number',
  `cell` int(64) DEFAULT NULL COMMENT 'Cellphone number',
  `bank` int(64) DEFAULT NULL COMMENT 'Bank number',
  `email` int(64) DEFAULT NULL COMMENT 'Email Address',
  `corp` int(64) DEFAULT NULL COMMENT 'Corporation number',
  `custom1` int(64) DEFAULT NULL COMMENT 'Custom pattern 1',
  `custom2` int(64) DEFAULT NULL COMMENT 'Custom pattern 2',
  `custom3` int(64) DEFAULT NULL COMMENT 'Custom pattern 3',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_inspect_sens_user definition

CREATE TABLE `tb_inspect_sens_user` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `rDate` datetime(3) DEFAULT NULL COMMENT 'Registration date',
  `sDate` datetime(3) DEFAULT NULL COMMENT 'Start Inspection',
  `eDate` datetime(3) DEFAULT NULL COMMENT 'End Inspection',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `targetFileCnt` int(64) DEFAULT NULL COMMENT 'Target File Count',
  `detectFileCnt` int(64) DEFAULT NULL COMMENT 'Detection File Count',
  `jumin` int(64) DEFAULT NULL COMMENT 'Resident registration number',
  `foreigner` int(64) DEFAULT NULL COMMENT 'Foreigner registration number',
  `passport` int(64) DEFAULT NULL COMMENT 'Passport number',
  `driver` int(64) DEFAULT NULL COMMENT 'Driver license',
  `card` int(64) DEFAULT NULL COMMENT 'Card number',
  `biz` int(64) DEFAULT NULL COMMENT 'Business number',
  `tel` int(64) DEFAULT NULL COMMENT 'Telephone number',
  `cell` int(64) DEFAULT NULL COMMENT 'Cellphone number',
  `bank` int(64) DEFAULT NULL COMMENT 'Bank number',
  `email` int(64) DEFAULT NULL COMMENT 'Email Address',
  `corp` int(64) DEFAULT NULL COMMENT 'Corporation number',
  `custom1` int(64) DEFAULT NULL COMMENT 'Custom pattern 1',
  `custom2` int(64) DEFAULT NULL COMMENT 'Custom pattern 2',
  `custom3` int(64) DEFAULT NULL COMMENT 'Custom pattern 3',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_law_ZIPCODE definition

CREATE TABLE `tb_law_ZIPCODE` (
  `ZIP_NO` varchar(5) DEFAULT NULL COMMENT '우편번호',
  `SIDO` varchar(20) DEFAULT NULL COMMENT '시도',
  `SIDO_ENG` varchar(40) DEFAULT NULL COMMENT '시도(영문)',
  `SIGUNGU` varchar(20) DEFAULT NULL COMMENT '시군구',
  `SIGUNGU_ENG` varchar(40) DEFAULT NULL COMMENT '시군구(영문)',
  `EUPMYUN` varchar(20) DEFAULT NULL COMMENT '읍면',
  `EUPMYUN_ENG` varchar(40) DEFAULT NULL COMMENT '읍면(영문)',
  `DORO_CD` varchar(12) DEFAULT NULL COMMENT '도로명코드',
  `DORO` varchar(80) DEFAULT NULL COMMENT '도로명',
  `DORO_ENG` varchar(80) DEFAULT NULL COMMENT '도로명(영문)',
  `UNDERGROUND_YN` char(1) DEFAULT NULL COMMENT '지하여부',
  `BUILD_NO1` decimal(5,0) DEFAULT NULL COMMENT '건물번호본번',
  `BUILD_NO2` decimal(5,0) DEFAULT NULL COMMENT '건물번호부번',
  `BUILD_NO_MANAGE_NO` varchar(25) DEFAULT NULL COMMENT '건물관리번호',
  `DARYANG_NM` varchar(40) DEFAULT NULL COMMENT '다량배달처명',
  `BUILD_NM` varchar(200) DEFAULT NULL COMMENT '시군구용건물명',
  `DONG_CD` varchar(10) DEFAULT NULL COMMENT '법정동코드',
  `DONG_NM` varchar(20) DEFAULT NULL COMMENT '법정동명',
  `RI` varchar(20) DEFAULT NULL COMMENT '리명',
  `H_DONG_NM` varchar(40) DEFAULT NULL COMMENT '행정동명',
  `SAN_YN` varchar(1) DEFAULT NULL COMMENT '산여부',
  `ZIBUN1` decimal(4,0) DEFAULT NULL COMMENT '지번본번',
  `EUPMYUN_DONG_SN` varchar(2) DEFAULT NULL COMMENT '읍면동일련번호',
  `ZIBUN2` decimal(4,0) DEFAULT NULL COMMENT '지번부번',
  `ZIP_NO_OLD` varchar(4) DEFAULT NULL COMMENT '구우편번호',
  `ZIP_SN` varchar(2) DEFAULT NULL COMMENT '우편일련번호',
  KEY `ZIPCODE_IDX` (`SIDO`,`SIGUNGU`,`DONG_NM`,`EUPMYUN`,`ZIP_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_law_dept definition

CREATE TABLE `tb_law_dept` (
  `org_code` varchar(200) NOT NULL,
  `created_at` datetime DEFAULT NULL,
  `modified_at` datetime DEFAULT NULL,
  `order` int(11) DEFAULT NULL,
  `org_full_code` varchar(100) DEFAULT NULL,
  `org_full_name` varchar(300) DEFAULT NULL,
  `org_name` varchar(30) DEFAULT NULL,
  `org_pa_code` varchar(200) DEFAULT NULL,
  `color` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`org_code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_law_result definition

CREATE TABLE `tb_law_result` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `state` varchar(1) NOT NULL COMMENT 'Log State',
  `cDate` datetime(3) NOT NULL COMMENT 'Log Create DateTime',
  `func` varchar(10) NOT NULL COMMENT 'Function Code',
  `iData` varchar(20480) NOT NULL COMMENT 'Log Law Data',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Log Update DateTime',
  `eCode` int(11) DEFAULT NULL COMMENT 'Log Check Error Code',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_law_user definition

CREATE TABLE `tb_law_user` (
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `userPwd` varchar(64) DEFAULT NULL COMMENT 'User Password',
  `userName` varchar(45) DEFAULT NULL COMMENT 'User Name',
  `rankCode` varchar(32) DEFAULT NULL COMMENT 'Rank Code',
  `positionCode` varchar(32) DEFAULT NULL COMMENT 'Position Code',
  `deptCode` varchar(32) DEFAULT NULL COMMENT 'Dept Code',
  `officeTel` varchar(45) DEFAULT NULL COMMENT 'Office Tel',
  `officeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Office Address Code',
  `officeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Office Address Detail',
  `phone` varchar(45) DEFAULT NULL COMMENT 'Phone Number',
  `email` varchar(45) DEFAULT NULL COMMENT 'Email',
  `homeTel` varchar(45) DEFAULT NULL COMMENT 'Home Tel',
  `homeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Home Address Code',
  `homeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Home Address Detail',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` int(11) DEFAULT NULL COMMENT 'Status',
  PRIMARY KEY (`userId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_log_change definition

CREATE TABLE `tb_log_change` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `type` varchar(45) DEFAULT NULL COMMENT 'Connect Media type',
  `plugInOut` datetime(3) NOT NULL COMMENT 'PlugInOut Time',
  `userId` varchar(45) NOT NULL COMMENT 'User Id',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `state` int(11) NOT NULL DEFAULT 0 COMMENT 'Plug_In Time',
  `browser` varchar(255) NOT NULL COMMENT 'Browser',
  `ipAddr` varchar(100) NOT NULL COMMENT 'IpAddr',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.tb_log_history definition

CREATE TABLE `tb_log_history` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `mType` varchar(4) NOT NULL COMMENT 'Connect Media type',
  `mInfo` varchar(45) DEFAULT NULL COMMENT 'Connect Media Info',
  `mVersion` varchar(45) DEFAULT NULL COMMENT 'Connect Media Version',
  `mIp` varchar(16) NOT NULL COMMENT 'Connect Media IP',
  `inOutFlag` varchar(4) NOT NULL COMMENT 'PlugInOut Flag',
  `inOutTime` datetime(3) NOT NULL COMMENT 'PlugInOut Time',
  `userId` varchar(45) DEFAULT NULL COMMENT 'User Id',
  `userName` varchar(45) DEFAULT NULL COMMENT 'User Name',
  `deptName` varchar(45) DEFAULT NULL COMMENT 'Dept Name',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'Universally Unique Identifier',
  `cDate` datetime(3) NOT NULL COMMENT 'Create Date Time',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v2.v_tb_info_user definition

CREATE VIEW `v_tb_info_user` AS
    SELECT 
        `J`.`uuid` AS `uuid`,
        `J`.`userId` AS `userId`,
        `U`.`userName` AS `userName`,
        `D`.`deptCode` AS `deptCode`,
        `D`.`deptName` AS `deptName`
    FROM
        ((`tb_info_join` `J`
        JOIN `tb_info_user` `U`)
        JOIN `tb_info_dept` `D`)
    WHERE
        `J`.`userId` = `U`.`userId`
            AND `U`.`deptCode` = `D`.`deptCode`;