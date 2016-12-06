
CC=gcc
CPPFLAGS= -I. -I./include -I/usr/include/mysql -I/usr/local/include/hiredis
CFLAGS=-Wall 
LIBS=-lpthread -levent -lm -lssl -lcrypto -levent_openssl  -lmysqlclient -lhiredis

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))

test_redis=./test/test_redis

server=OBO_data_server

target=$(server) $(test_redis)


ALL:$(target) 


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#OBO_data_server程序
$(server):  https-server.o \
            https-common.o \
            cJSON.o \
            persistent.o \
            cache.o \
            dao_mysql.o \
            redis_op.o \
            config.o \
            util.o \
            deal_persistent.o \
            deal_cache.o \
            make_log.o
	$(CC) $^ -o $@ $(LIBS)

$(test_redis): cJSON.o \
            redis_op.o \
            config.o \
            util.o \
            make_log.o \
            test/test_redis.o

	$(CC) $^ -o $@ $(LIBS)





#clean指令

clean:
	-rm -rf $(obj) $(target) ./test/*.o

distclean:
	-rm -rf $(obj) $(target) ./test/*.o

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
