//
// Created by Ace on 2016/11/24.
//

#ifndef OBO_JSON_H
#define OBO_JSON_H

#include <string>

#include "cJSON.h"

using namespace std;

class Json {
public:
    Json();
    ~Json();

    void insert(string key, string value);
    void insertInt(string key, int value);
    string print();
    void parse(string json_str);
    string value(string key);

private:
    Json(const Json&);
    Json& operator=(const Json&);

    cJSON *_root;
};


#endif //OBO_JSON_H
