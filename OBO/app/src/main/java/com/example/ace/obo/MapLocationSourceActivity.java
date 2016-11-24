package com.example.ace.obo;

import android.os.PersistableBundle;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.CameraUpdate;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.LocationSource;
import com.amap.api.maps.MapView;
import com.amap.api.maps.UiSettings;
import com.amap.api.maps.model.BitmapDescriptor;
import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.CameraPosition;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Marker;
import com.amap.api.maps.model.MarkerOptions;

public class MapLocationSourceActivity extends AppCompatActivity {


    protected MapView mMapView = null;
    protected AMap mMap = null;


    //定位客户端对象
    protected AMapLocationClient mLocationClient = null;

    //声明AMapLocationClientOption对象
    protected AMapLocationClientOption mLocationOption = null;

    protected LocationSource mLocationSource = null;

    //声明位置源位置信息改变监听对象
    protected LocationSource.OnLocationChangedListener mLocationChangeListener = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map_location_source);

        //初始化mapView 容器
        mMapView = (MapView) findViewById(R.id.scoure_map);
        //启动mapView显示主地图
        mMapView.onCreate(savedInstanceState);


        //获取mMap操作对象
        if (mMap == null) {
            mMap = mMapView.getMap();

        }


        /*1 ----- 设置一个位置信息源 ---- */
        mLocationSource = new LocationSource() {


            @Override
            public void activate(OnLocationChangedListener onLocationChangedListener) {
                mLocationChangeListener = onLocationChangedListener;

                if (mLocationClient == null) {
                    /*------1.1 获取客户端定位对象 ---- */
                    mLocationClient = new AMapLocationClient(getApplicationContext());


                    /*------1.2 配置定位属性 -------*/
                    //初始化AMapLocationClientOption对象
                    mLocationOption = new AMapLocationClientOption();

                    //设定持续定位
                    //设置定位间隔,单位毫秒,默认为2000ms，最低1000ms
                    mLocationOption.setInterval(2000);

                    //给定位客户端对象设置定位参数
                    mLocationClient.setLocationOption(mLocationOption);

                    /* -----1.3 设置定位回调监听 ---- */
                    mLocationClient.setLocationListener(new AMapLocationListener() {
                        @Override
                        public void onLocationChanged(AMapLocation aMapLocation) {
                            if (aMapLocation != null) {
                                if (aMapLocation.getErrorCode() == 0) {
                                    //可在其中解析amapLocation获取相应内容。
                                    Log.e("Amap", "address: " + aMapLocation.getAddress());
                                    mLocationChangeListener.onLocationChanged(aMapLocation);
                                    Log.e("Amap", "onLocationChanged ---> aMapLocation");

                                }else {
                                    //定位失败时，可通过ErrCode（错误码）信息来确定失败的原因，errInfo是错误信息，详见错误码表。
                                    Log.e("Amap","location Error, ErrCode:"
                                            + aMapLocation.getErrorCode() + ", errInfo:"
                                            + aMapLocation.getErrorInfo());
                                }
                            }
                        }
                    });

                    /* 1.4 启动定位 */
                    mLocationClient.startLocation();
                    Log.e("Amap", "mLocationClient.startLocation");
                }
            }

            @Override
            public void deactivate() {
                mLocationChangeListener = null;
                if (mLocationClient != null) {
                    mLocationClient.stopLocation();
                    mLocationClient.onDestroy();
                }
                mLocationClient = null;
                mLocationOption = null;
            }
        };

        /* ------   2 ---------------
                 给map对象设置locationSource对象，始终告诉
               map是以locationScource为中心
         */
        mMap.setLocationSource(mLocationSource);
        mMap.setMyLocationEnabled(true);
        mMap.setMyLocationType(AMap.LOCATION_TYPE_LOCATE);



    }


    @Override
    protected void onResume() {
        super.onResume();
        mMapView.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mMapView.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mMapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        mMapView.onSaveInstanceState(outState);
    }
}
