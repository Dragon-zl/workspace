CREATE TABLE pcba(
    barcode VARCHAR(30) NOT NULL,
    date VARCHAR(30) NOT NULL,
    status VARCHAR(30) NOT NULL,
    pat_1 FLOAT,
    pat_2 FLOAT,
    pat_3 FLOAT,
    PRIMARY KEY(barcode))ENGINE = InnoDB;