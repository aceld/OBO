package com.example.ace.obo;

import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.MapView;
import com.amap.api.maps.UiSettings;
import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.CameraPosition;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Marker;
import com.amap.api.maps.model.MarkerOptions;
import com.amap.api.services.core.LatLonPoint;

public class DriverMapActivity extends AppCompatActivity {


    private MapView _mapView = null;
    private AMap _amap  = null;
    private UiSettings _uiSettings = null;
    private AMapLocationClient _amapLocationClient = null;
    private AMapLocationClientOption _amapLocationClientOption = null;

    private boolean _postMyLocationCenter = true;
    private String _autoSend = "no";
    private String _bt_status = null;

    //ui
    private Button _bt_getOrder = null;
    //司机自己的位置覆盖物
    private Marker _locationMarker = null;
    //已经接单的乘客覆盖物
    private Marker _passengerMarker = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_driver_map);

        //初始化mapView 容器
        _mapView = (MapView) findViewById(R.id.driver_map);
        //启动mapView显示主地图
        _mapView.onCreate(savedInstanceState);


        //获取mMap操作对象
        if (_amap == null) {
            _amap = _mapView.getMap();

            if (_uiSettings == null) {
                _uiSettings = _amap.getUiSettings();
            }
        }

        /* -------------- 初始化搜索布局 -------------- */
        initSearchBarLayout();

        //开始定位业务
        doLocation();
    }

    //初始化搜索菜单栏
    public void initSearchBarLayout() {


        _bt_getOrder = (Button)findViewById(R.id.bt_getOrder);

        _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_IDLE);

        _bt_getOrder.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (_bt_status.toString().equals(getResources().getString(R.string.DRIVER_BUTTON_STATUS_IDLE))) {
                    //司机为idle状态, 更变司机为ready状态，此刻代表司机已经 决定开始接单

                    //更变司机的ready状态
                    OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_READY));
                    _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_READY);
                    _bt_getOrder.setText("停止接单");
                }
                else if (_bt_status.toString().equals(getResources().getString(R.string.DRIVER_BUTTON_STATUS_READY))) {
                    //司机已经为确定停止接单

                    //更变司机为idle状态
                    OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_IDLE));
                    _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_IDLE);
                    _bt_getOrder.setText("开始接单");
                }
                else if (_bt_status.toString().equals(getResources().getString(R.string.DRIVER_BUTTON_STATUS_CATCHING))) {
                    //说明已经有乘客上车

                    //更变司机为driving状态
                    OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_DRIVE));
                    _autoSend = "yes";

                    _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_DRIVE);
                    _bt_getOrder.setText("已抵达目的地");

                }
                else if (_bt_status.toString().equals(getResources().getString(R.string.DRIVER_BUTTON_STATUS_DRIVE))) {
                    //说明乘客已抵达目的地

                    //生成最终订单 完成订单

                    OBOJni.getInstence().FinishOrder();

                    //更变司机为idle状态
                    OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_IDLE));
                    _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_IDLE);
                    _bt_getOrder.setText("开始接单");
                }

            }
        });
    }


    //在指定坐标添加一个 覆盖物 图标
    public void addDriverMakerToMap( double latitude, double longitude)
    {
        //添加Marker显示定位位置
        if (_locationMarker == null) {
            //如果是空的添加一个新的,icon方法就是设置定位图标，可以自定义
            _locationMarker = _amap.addMarker(new MarkerOptions()
                    .position(new LatLng(latitude, longitude))
                    .icon(BitmapDescriptorFactory.fromResource(R.drawable.location_marker)));
            Log.e("Amap", "首次添加Drivermarker. 到 "+ latitude+", "+longitude);
        } else {
            //已经添加过了，修改位置即可
            _locationMarker.setPosition(new LatLng(latitude, longitude));
            Log.e("Amap", "重新设置Drivermarker位置 到 "+ latitude+", "+longitude);
        }
    }

    //在指定坐标添加一个 覆盖物 图标
    public void addPassengerMakerToMap( double latitude, double longitude)
    {
        //添加Marker显示定位位置
        if (_passengerMarker == null) {
            //如果是空的添加一个新的,icon方法就是设置定位图标，可以自定义
            _passengerMarker = _amap.addMarker(new MarkerOptions()
                    .position(new LatLng(latitude, longitude))
                    .icon(BitmapDescriptorFactory.fromResource(R.drawable.location_marker)));
            Log.e("Amap", "首次添加PassengerMarker. 到 "+ latitude+", "+longitude);
        } else {
            //已经添加过了，修改位置即可
            _passengerMarker.setPosition(new LatLng(latitude, longitude));
            Log.e("Amap", "重新设置PassengerMarker位置 到 "+ latitude+", "+longitude);
        }
    }

    // ============ 开启定位服务接口 ============
    public void doLocation() {

        /*--------------  定位接口操作（start） ------------ */

        /*------1 获取客户端定位对象 ---- */
        _amapLocationClient = new AMapLocationClient(getApplicationContext());


        /*------2 配置定位属性 -------*/
        //初始化AMapLocationClientOption对象

        _amapLocationClientOption = new AMapLocationClientOption();

        //设定持续定位
        //设置定位间隔,单位毫秒,默认为2000ms，最低1000ms
        _amapLocationClientOption.setInterval(2000);

        //_amapLocationClientOption.setOnceLocation(true);

        //给定位客户端对象设置定位参数
        _amapLocationClient.setLocationOption(_amapLocationClientOption);


        /* -----3 设置定位回调监听 ---- */
        _amapLocationClient.setLocationListener(new AMapLocationListener() {
            @Override
            public void onLocationChanged(AMapLocation aMapLocation) {
                if (aMapLocation != null) {
                    if (aMapLocation.getErrorCode() == 0) {
                        //可在其中解析amapLocation获取相应内容。
                        Log.e("Amap", "address: " + aMapLocation.getAddress());



                        //取出经纬度
                        LatLng latLng = new LatLng(aMapLocation.getLatitude(),
                                aMapLocation.getLongitude());

                        /*
                        //添加Marker显示定位位置
                        if (_locationMarker == null) {
                            //如果是空的添加一个新的,icon方法就是设置定位图标，可以自定义
                            _locationMarker = _amap.addMarker(new MarkerOptions()
                                    .position(latLng)
                                    .icon(BitmapDescriptorFactory.fromResource(R.drawable.location_marker)));
                        } else {
                            //已经添加过了，修改位置即可
                            _locationMarker.setPosition(latLng);
                        }
                        */
                        addDriverMakerToMap(aMapLocation.getLatitude(),aMapLocation.getLongitude());

                        // 以自我为中心 只执行一次
                        if (_postMyLocationCenter == true) {
                            //得到当前坐标点
                            CameraPosition cp = _amap.getCameraPosition();
                            //然后可以移动到定位点,使用animateCamera就有动画效果
                            _amap.animateCamera(CameraUpdateFactory.newLatLngZoom(latLng, cp.zoom));
                            _postMyLocationCenter = false;
                        }


                        //开始上传司机地理位置信息 locationChanged业务
                        OBOJni.getInstence().DriverLocationChanged(aMapLocation.getLongitude()+"",
                                aMapLocation.getLatitude()+"",
                                aMapLocation.getAddress()+"",
                                _autoSend);


                        if (_autoSend.equals("yes") == true) {
                            _autoSend = "no";
                        }

                        if (OBOJni.getInstence().getStatus().equals(getResources().getString(R.string.DRIVER_STATUS_DRIVE)) ||
                                OBOJni.getInstence().getStatus().equals(getResources().getString(R.string.DRIVER_STATUS_CATCH))) {
                            //正在拉客，或者 寻找乘客， 可以得到乘客坐标地址

                            Log.e("Amap", "ptemp_longitude = " + OBOJni.getInstence().getPtempLongitude());
                            Log.e("Amap", "ptemp_latitude = " + OBOJni.getInstence().getPtempLatitude());

                            //将乘客的坐标地址临时添加覆盖物
                            addPassengerMakerToMap(Double.parseDouble(OBOJni.getInstence().getPtempLatitude()),
                                         Double.parseDouble(OBOJni.getInstence().getPtempLongitude()));
                        }



                        else if (OBOJni.getInstence().getStatus().toString().equals(getResources().getString(R.string.DRIVER_STATUS_READY)) &&
                            !OBOJni.getInstence().getOrderid().toString().equals("NONE")) {
                            //已经有乘客下单，并且选择该司机，提示司机是否接受订单
                            Log.e("DriverMap", "已经有乘客下单");
                            AlertDialog.Builder builder = new AlertDialog.Builder(DriverMapActivity.this);
                            builder.setTitle("确认" );
                            builder.setMessage("确定接单吗？" );

                            builder.setPositiveButton("是", new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog, int which) {
                                            OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_CATCH));
                                            _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_CATCHING);
                                            _bt_getOrder.setText("乘客已上车");
                                        }
                            });
                            builder.setNegativeButton("否", new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog, int which) {
                                            OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_IDLE));
                                            _bt_status = getResources().getString(R.string.DRIVER_BUTTON_STATUS_IDLE);
                                            _bt_getOrder.setText("开始接单");

                                        }
                            });
                            builder.show();

                        }
                        else if (OBOJni.getInstence().getStatus().toString().equals(getResources().getString(R.string.DRIVER_STATUS_DRIVE)) &&
                                OBOJni.getInstence().getOrderid().toString().equals("NONE")) {
                            //该订单已经完成，更改司机为"idle"状态
                            OBOJni.getInstence().setStatus(getResources().getString(R.string.DRIVER_STATUS_IDLE));
                        }


                    } else {
                        //定位失败时，可通过ErrCode（错误码）信息来确定失败的原因，errInfo是错误信息，详见错误码表。
                        Log.e("Amap", "location Error, ErrCode:"
                                + aMapLocation.getErrorCode() + ", errInfo:"
                                + aMapLocation.getErrorInfo());
                    }
                }
            }
        });



        /*--- 4 启动定位---*/
        _amapLocationClient.startLocation();
    }
    /*--------------  定位接口操作（end） ------------ */


    @Override
    protected void onResume() {
        super.onResume();
        _mapView.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        _mapView.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        _mapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        _mapView.onSaveInstanceState(outState);
    }
}
