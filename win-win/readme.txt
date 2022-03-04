CREATE TABLE pcba_state(barcode VARCHAR(20) NOT NULL,date int NOT NULL, pro_1 VARCHAR(4) NULL,pro_2 VARCHAR(4) NULL,pro_3 VARCHAR(4) NULL,pro_4 VARCHAR(4) NULL,pro_5 VARCHAR(4) NULL,PRIMARY KEY(barcode))ENGINE = InnoDB;

CREATE TABLE pcba_pro_1(barcode VARCHAR(20) NOT NULL, WorkingVoltage float NOT NULL, DmPDissipayion float NOT NULL, R2 float NOT NULL, PCMRESIS_N float NOT NULL, PCMRESIS_P float NOT NULL, PCMRESIS float NOT NULL, PRIMARY KEY(barcode))ENGINE = InnoDB;
CREATE TABLE pcba_pro_2(barcode VARCHAR(20) NOT NULL, OVPRotTime float NOT NULL, OVPRotVol float NOT NULL, OverLVolL float NOT NULL, UVPRotTime float NOT NULL, UVPRotVol float NOT NULL,UVRElVol float NOT NULL, PRIMARY KEY(barcode))ENGINE = InnoDB;
CREATE TABLE pcba_pro_3(barcode VARCHAR(20) NOT NULL, OVPRotTime_2 float NOT NULL, OVPRotVol_2 float NOT NULL, OVRElVol_2 float NOT NULL, UVPRotTime_2 float NOT NULL, UVPRotVol_2 float NOT NULL,UVRElVol_2 float NOT NULL, PRIMARY KEY(barcode))ENGINE = InnoDB;
CREATE TABLE pcba_pro_4(barcode VARCHAR(20) NOT NULL, ODCPRotTime float NOT NULL, ODCCurrEnt float NOT NULL, OCCPRotTime float NOT NULL, OCCCurrEnt float NOT NULL, PRIMARY KEY(barcode))ENGINE = InnoDB;
CREATE TABLE pcba_pro_5(barcode VARCHAR(20) NOT NULL, ODCPRotTime_2 float NOT NULL, ODCCurrEnt_2 float NOT NULL, OCCPRotTime_2 float NOT NULL, OCCCurrEnt_2 float NOT NULL, PRIMARY KEY(barcode))ENGINE = InnoDB;
