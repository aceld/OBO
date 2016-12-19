create table OBO_TABLE_ORDER
(
    orderid                 VARCHAR(64) not null primary key, 
    passenger_username      VARCHAR(32), 
    driver_username         VARCHAR(32), 

    create_order_time       timestamp not NULL DEFAULT CURRENT_TIMESTAMP, 
    start_order_time        timestamp, 
    end_time                timestamp, 

    src_address             VARCHAR(256), 
    dst_address             VARCHAR(256), 
    src_longitude           VARCHAR(64), 
    src_latitude            VARCHAR(64), 
    dst_longitude           VARCHAR(64), 
    dst_latitude            VARCHAR(64), 

    src_address_real        VARCHAR(256), 
    dst_address_real        VARCHAR(256), 
    src_longitude_real      VARCHAR(64), 
    src_latitude_real       VARCHAR(64), 
    dst_longitude_real      VARCHAR(64), 
    dst_latitude_real       VARCHAR(64), 

    RMB                     VARCHAR(32)
);
