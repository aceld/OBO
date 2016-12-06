//
// Created by Ace on 2016/11/30.
//

#ifndef OBO_DATA_H
#define OBO_DATA_H

#include <string>

using namespace std;

#define OBO_STATUS_DRIVER_IDLE      "idle"
#define OBO_STATUS_DRIVER_READY     "ready"
#define OBO_STATUS_DRIVER_CATCH     "catching"
#define OBO_STATUS_DRIVER_DRIVE     "driving"
#define OBO_STATUS_PASSNGER_IDLE    "idle"
#define OBO_STATUS_PASSNGER_WAIT    "waiting"
#define OBO_STATUS_PASSNGER_TRAVEL  "traveling"

#define OBO_ORDER_ID_NONE           "NONE"

class Data {
public:
    static Data* getInstance() {
        if (instance == NULL)
        {
            instance = new Data();
        }

        return instance;
    }

    void setSessionid(string sessionid)
    {
        this->sessionid = sessionid;
    }
    string getSessionid() {
        return this->sessionid;
    }

    void setIsDriver(string isDriver) {
        this->isDriver = isDriver;
    }

    string getIsDriver() {
        return this->isDriver;
    }

    void setOrderid(string orderid) {
        this->orderid = orderid;
    }

    string getOrderid() {
        return this->orderid;
    }

    void setStatus(string status) {
        this->status = status;
    }
    string getStatus() {
        return this->status;
    }

    void set_ptemp_location(string longitude, string latitude)
    {
        this->ptemp_longitude = longitude;
        this->ptemp_latitude = latitude;
    }

    string get_ptemp_longitude() {
        return this->ptemp_longitude;
    }

    string get_ptemp_latitude() {
        return this->ptemp_latitude;
    }

    void set_dtemp_location(string longitude, string latitude)
    {
        this->dtemp_longitude = longitude;
        this->dtemp_latitude = latitude;
    }

    string get_dtemp_longitude() {
        return this->dtemp_longitude;
    }

    string get_dtemp_latitude() {
        return this->dtemp_latitude;
    }

private:
    static Data *instance;

    //当前用户的sessionid
    string sessionid;

    //当前用户的orderid 订单号
    string orderid;

    //当前用户是否是司机
    string isDriver;

    //当前用户的状态
    string status; //如果是司机 具有如下赋值:["idle","ready","catching","driving"]
                   //如果是乘客 具有如下赋值:["idle","waiting","traveling"]


    //乘客当前坐标：供driving和catching状态的司机使用
    string ptemp_longitude;
    string ptemp_latitude;

    //司机当前坐标，供waiting和traveling的乘客使用
    string dtemp_longitude;
    string dtemp_latitude;
};


#endif //OBO_DATA_H
