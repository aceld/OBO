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

import java.sql.BatchUpdateException;

public class RegActivity extends AppCompatActivity {

    public static String LogTag="OBO-RegActivity";


    private EditText et_reg_username = null;
    private EditText et_reg_passwd1 = null;
    private EditText et_reg_passwd2 = null;
    private EditText et_reg_tel = null;
    private EditText et_reg_email = null;
    private EditText et_reg_id_card = null;
    private Button   bt_reg_submit = null;
    private CheckBox cb_reg_isDriver = null;
    private boolean isDriver = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_reg);


        et_reg_username = (EditText)findViewById(R.id.et_reg_username);
        et_reg_passwd1 = (EditText)findViewById(R.id.et_reg_passwd1);
        et_reg_passwd2 = (EditText)findViewById(R.id.et_reg_passwd2);
        et_reg_tel = (EditText)findViewById(R.id.et_reg_tel);
        et_reg_email =(EditText)findViewById(R.id.et_reg_email);
        et_reg_id_card = (EditText)findViewById(R.id.et_reg_id_card);
        bt_reg_submit =(Button)findViewById(R.id.bt_reg_submit);
        cb_reg_isDriver = (CheckBox)findViewById(R.id.cb_is_driver);




        cb_reg_isDriver.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
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


        bt_reg_submit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //注册提交按钮被点击
                String username = et_reg_username.getText().toString().trim();
                String passwd1 = et_reg_passwd1.getText().toString().trim();
                String passwd2 = et_reg_passwd2.getText().toString().trim();
                String tel = et_reg_tel.getText().toString().trim();
                String email = et_reg_email.getText().toString().trim();
                String id_card = et_reg_id_card.getText().toString().trim();

                if (username.length() == 0) {
                    Toast.makeText(getApplicationContext(),
                            "用户名为空或者非法",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (passwd1.length() == 0 || passwd2.length() == 0) {
                    Toast.makeText(getApplicationContext(),
                            "密码非法",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (!passwd1.equals(passwd2)) {
                    Toast.makeText(getApplicationContext(),
                            "两次输入的密码不一样",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (tel.length() == 0) {
                    Toast.makeText(getApplicationContext(),
                            "电话号码为空",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (email.length()==0) {
                    Toast.makeText(getApplicationContext(),
                            "email为空",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (id_card.length() == 0) {
                    Toast.makeText(getApplicationContext(),
                            "身份证号为空",Toast.LENGTH_SHORT).show();
                    return;
                }

                if(OBOJni.getInstence().Reg(username,passwd1,tel,email, id_card,isDriver) == true) {
                    Log.e(LogTag, "Sessionid = "+ OBOJni.getInstence().getSessionid());

                    if (OBOJni.getInstence().getIsDriver().equals("no")) {
                        Intent intent = new Intent();
                        intent.setClass(RegActivity.this, PassengerMapActivity.class);
                        startActivity(intent);
                    }
                    else {
                        Intent intent = new Intent();
                        intent.setClass(RegActivity.this, DriverMapActivity.class);
                        startActivity(intent);
                    }

                }
                else {
                    Toast.makeText(getApplicationContext(),
                            "链接服务器失败",Toast.LENGTH_SHORT).show();
                }
            }
        });
    }
}
