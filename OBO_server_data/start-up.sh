

kill -9 `ps aux | grep "OBO_data_server" |grep -v grep | awk '{print $2}'`

#启动redis-server
redis-server ./conf/redis.conf


#启动web服务器
./OBO_data_server 18889 &
