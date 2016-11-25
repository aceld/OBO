create table OBO_TABLE_USER
(
    u_id        bigint not null primary key AUTO_INCREMENT, 
    u_name      VARCHAR(32) not null, 
    password    VARCHAR(32) not null, 
    phone       VARCHAR(11) not null, 
    createtime  timestamp not NULL DEFAULT CURRENT_TIMESTAMP, 
    email       VARCHAR(64), 
    id_card     VARCHAR(18) not null,
    driver      VARCHAR(6) not null DEFAULT "no",
    constraint uq_u_name unique(u_name), constraint uq_id_card unique(id_card)
);
