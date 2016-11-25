//
// Created by Ace on 2016/11/24.
//

#include "Curl.h"

Curl::Curl(string url, bool ignoreCert) {

    _curl = curl_easy_init();

    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    JNIINFO("curl:%s", url.c_str());

    //是否忽略 CA 认证
    if (ignoreCert == true) {
        JNIINFO("%s","ignore CA");
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, false);
    }
}

Curl::~Curl() {
    curl_easy_cleanup(_curl);
}

bool Curl::execute(string requestData) {

    curl_easy_setopt(_curl, CURLOPT_POST, true);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, requestData.c_str());


    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, Curl::dealResponse);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);

    CURLcode code = curl_easy_perform(_curl);
    if (code != CURLE_OK) {
        JNIINFO("curl easy perform error code = %d", code);
        return false;
    }

    return true;
}


string Curl::responseData() {

    return this->_responseData;
}

ssize_t Curl::dealResponse(char *ptr, size_t m, size_t n, void *arg) {

    Curl* This = (Curl*)arg;
    int count = m*n;

    // 拷贝ptr中的数据到_responseData

    // NO
    //  string data(ptr);//有溢出越界风险
    //  This->_responseData += data;

/*
    效率太低
    for(int i=0; i<count; ++i)
    {
        This->_responseData += ptr[i];
    }
*/

    copy(ptr, ptr+count, back_inserter(This->_responseData));

    return count;
}
