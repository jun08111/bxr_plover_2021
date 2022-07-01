-- bluexg_v2.v_tb_law_ZIPCODE definition

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

