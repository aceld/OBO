
CC=gcc
CPPFLAGS= -I. -I./include 
CFLAGS=-Wall 
LIBS=-lpthread -levent -lm -lssl -lcrypto -levent_openssl -luuid -lcurl

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))


server=OBO_web_server

target=$(server)


ALL:$(target)


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#OBO_web_server程序
$(server): https-server.o https-common.o cJSON.o login.o reg.o util.o remote_store.o set_order.o location_changed.o finish_order.o make_log.o
	$(CC) $^ -o $@ $(LIBS)





#clean指令

clean:
	-rm -rf $(obj) $(target) 

distclean:
	-rm -rf $(obj) $(target) 

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
