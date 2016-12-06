package com.example.ace.obo;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    public static String LogTag="OBO-MainActivity";

    private Button bt_login = null;
    private Button bt_reg = null;
    private EditText et_username = null;
    private EditText et_passwd = null;
    private CheckBox cb_isDriver_login = null;
    private boolean isDriver = false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        bt_login = (Button)findViewById(R.id.bt_login);
        bt_reg = (Button)findViewById(R.id.bt_reg);
        et_username = (EditText)findViewById(R.id.et_username);
        et_passwd = (EditText)findViewById(R.id.et_passwd);
        cb_isDriver_login = (CheckBox)findViewById(R.id.login_cb_isDriver);

        cb_isDriver_login.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    isDriver = true;
                }
                else {
                    isDriver = false;
                }
            }
        });


        //绑定登陆按钮的点击事件
        bt_login.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String username = et_username.getText().toString();
                String passwd = et_passwd.getText().toString();
                Log.e(LogTag,"username:"+ username);
                Log.e(LogTag, "passwd:" + passwd);

                if (username.length() == 0 || passwd.length() == 0) {
                    Toast.makeText(getApplicationContext(),
                            "用户名或密码为空",Toast.LENGTH_SHORT).show();
                    return;
                }

                boolean login_result = OBOJni.getInstence().Login(username, passwd,isDriver);
                Log.e(LogTag, "Login result is " + login_result);

                if (login_result == true) {
                    if (OBOJni.getInstence().getIsDriver().equals("no")) {
                        Intent intent = new Intent();
                        intent.setClass(MainActivity.this, PassengerMapActivity.class);
                        startActivity(intent);
                    }
                    else {
                        Intent intent = new Intent();
                        intent.setClass(MainActivity.this, DriverMapActivity.class);
                        startActivity(intent);
                    }
                }
                else {
                    Toast.makeText(getApplicationContext(),
                            "登陆失败-无法连接服务器",Toast.LENGTH_SHORT).show();
                    Log.e(LogTag, "Login error！");
                }

            }
        });


        bt_reg.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setClass(MainActivity.this, RegActivity.class);
                startActivity(intent);
            }
        });
    }
}
