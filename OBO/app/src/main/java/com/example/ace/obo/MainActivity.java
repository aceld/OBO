package com.example.ace.obo;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity {

    public static String LogTag="OBO-MainActivity";

    private Button bt_login = null;
    private Button bt_reg = null;
    private EditText et_username = null;
    private EditText et_passwd = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        bt_login = (Button)findViewById(R.id.bt_login);
        bt_reg = (Button)findViewById(R.id.bt_reg);
        et_username = (EditText)findViewById(R.id.et_username);
        et_passwd = (EditText)findViewById(R.id.et_passwd);

        //绑定登陆按钮的点击事件
        bt_login.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String username = et_username.getText().toString();
                String passwd = et_passwd.getText().toString();
                Log.e(LogTag,"username:"+ username);
                Log.e(LogTag, "passwd:" + passwd);

                boolean login_result = OBOJni.getInstence().Login(username, passwd);
                Log.e(LogTag, "Login result is " + login_result);

                if (login_result == true) {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, MapMainActivity.class);
                    startActivity(intent);
                }
                else {
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
