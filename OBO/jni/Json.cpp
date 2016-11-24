//
// Created by Ace on 2016/11/24.
//

#include "Json.h"


Json::Json()
{
    _root = cJSON_CreateObject();
}

Json::~Json()
{
    cJSON_Delete(_root);
}

void Json::insert(string key, string value)
{
    cJSON_AddStringToObject(_root, key.c_str(), value.c_str());
}

void Json::insertInt(string key, int value) {
    cJSON_AddNumberToObject(_root, key.c_str(), value);
}

void Json::parse(string json_str) {
    cJSON *root = cJSON_CreateObject();
    root = cJSON_Parse(json_str.c_str());
    cJSON_Delete(_root);
    _root = root;
}

string Json::print() {
    char *p = cJSON_Print(_root);

    string ret_str(p);
    free(p);

    return ret_str;
}

string Json::value(string key) {
    cJSON *obj = cJSON_GetObjectItem(_root, key.c_str());
    if (obj == NULL) {
        return string();
    }

    return string(obj->valuestring);
}

