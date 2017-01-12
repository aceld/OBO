

kill -9 `ps aux | grep "OBO_web_server" |grep -v grep | awk '{print $2}'`


#启动web服务器
./OBO_web_server 18888 &
