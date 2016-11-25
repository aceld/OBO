package com.example.ace.obo;

/**
 * Created by Ace on 2016/11/18.
 */

public class OBOJni {

    public static OBOJni getInstence() {
        if (obj == null) {
            obj = new OBOJni();
        }

        return obj;
    }

    protected static OBOJni obj = null;


    static {
        // libOBOjni.so
        System.loadLibrary("OBOjni");
    }

    public native boolean Login(String username, String passwd);

    public native boolean Reg(String username,
                              String passwd,
                              String tel,
                              String email,
                              boolean isDriver);


    public native void testLibcurl();
    
}
