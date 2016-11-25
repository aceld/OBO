//
// Created by Ace on 2016/11/24.
//

#ifndef OBO_CURL_H
#define OBO_CURL_H

#include <string>
#include "curl/curl.h"
#include "OBOJni.h"

using namespace std;

class Curl {
public:
    Curl(string url, bool ignoreCert = true);
    ~Curl();

    bool execute(string requestData);
    string responseData();

    static ssize_t dealResponse(char* ptr, size_t m, size_t n, void* arg);

private:
    string _responseData;
    CURL *_curl;
};


#endif //OBO_CURL_H
