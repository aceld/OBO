package com.example.ace.obo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.TextView;

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
import com.amap.api.maps.overlay.DrivingRouteOverlay;
import com.amap.api.services.core.LatLonPoint;
import com.amap.api.services.core.PoiItem;
import com.amap.api.services.help.Inputtips;
import com.amap.api.services.help.InputtipsQuery;
import com.amap.api.services.help.Tip;
import com.amap.api.services.poisearch.PoiResult;
import com.amap.api.services.poisearch.PoiSearch;
import com.amap.api.services.route.BusRouteResult;
import com.amap.api.services.route.DrivePath;
import com.amap.api.services.route.DriveRouteResult;
import com.amap.api.services.route.RideRouteResult;
import com.amap.api.services.route.RouteSearch;
import com.amap.api.services.route.WalkRouteResult;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class PassengerMapActivity extends AppCompatActivity {

    private MapView _mapView = null;
    private AMap _amap = null;
    private AMapLocationClient _amapLocationClient = null;
    private AMapLocationClientOption _amapLocationClientOption = null;
    private Marker locationMarker = null;
    private UiSettings _uiSettings = null;
    private boolean postMyLocationCenter = true;

    //ui
    private AutoCompleteTextView autotv_finalAddr = null;
    private Button bt_startOrder = null;
    private TextView tv_passenger_info = null;

    //地理数据
    private String _city = null;
    //起始出发坐标点
    private LatLonPoint _startPoint = null;
    //终止目的地坐标点
    private LatLonPoint _endPoint = null;
    //起始地点
    private String _startAddr = null;
    //终止地点
    private String _finalAddr = null;

    //是否有司机接单
    private boolean _driverGet = false;
    private String bt_status = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_passenger_map);

        //初始化mapView 容器
        _mapView = (MapView) findViewById(R.id.passenger_map);
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

        autotv_finalAddr = (AutoCompleteTextView)findViewById(R.id.autotv_passgener_finalAddr);
        bt_startOrder = (Button)findViewById(R.id.bt_startOrder);
        tv_passenger_info = (TextView)findViewById(R.id.tv_passgener_info);

        bt_status = getString(R.string.PASSENGER_BUTTON_STATUS_IDLE);

        bt_startOrder.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (OBOJni.getInstence().getStatus().toString().equals(getString(R.string.DRIVER_STATUS_IDLE))) {
                    _finalAddr = autotv_finalAddr.getText().toString().trim();
                    if (_finalAddr == null) {
                        return;
                    }

                    doSearchPoi(_finalAddr);

                }

            }
        });

        /*--------  设置关键字模糊搜索 --------*/
        setAutoCompleteAddress();
    }


    public void doSearchPoi(String targetAddr) {
                /* ---------------  获取POI数据 (begin)-----------------*/
        //1 定义查询条件

        // 参数1 查询关键字
        // 参数2 隶属范围， 如果填"" 表示不关心
        // 参数3 查询关键字地点所属城市
        PoiSearch.Query poiSearchQuery = new PoiSearch.Query(targetAddr,"", _city);

        //2 根据查询条件 创建 查询句柄
        PoiSearch poiSearch = new PoiSearch(getApplicationContext(), poiSearchQuery);

        //3 设置搜索回调
        poiSearch.setOnPoiSearchListener(new PoiSearch.OnPoiSearchListener() {
            @Override
            public void onPoiSearched(PoiResult poiResult, int i) {
                //针对搜索 PoiItem集合的回调
                if (i != 1000) {
                    Log.e("Amap", "onPoiSearched error");
                    return;
                }
                //获取搜索poi集合
                List<PoiItem> poiList = poiResult.getPois();

                int listnum = poiList.size();
                for (int index = 0;index < listnum; index++) {
                    PoiItem item = poiList.get(index);

                    Log.e("Amap", index +
                            " onPoiSearched  distance:"
                            +item.getDistance()+
                            " [AdName]: "
                            +item.getAdName() +
                            " [AdCode]: "
                            +item.getAdCode()+
                            " [AdBusinessArea]: "
                            +item.getBusinessArea()+
                            " [Direction]: "
                            +item.getDirection() +
                            " [Enter]: "
                            +item.getEnter() +
                            " [Title]: "
                            +item.getTitle() +
                            " [PoiId]: "
                            +item.getPoiId());
                }

                /* 1. 取出第一个兴趣点 添加到 _endPoint中*/
                PoiItem poi = poiList.get(0);
                _endPoint = poi.getLatLonPoint();

                // Log.e("Amap", "StartPoint "+_startPoint.getLatitude() +", "+_startPoint.getLongitude());


                // =========== 得到规划路线 ==========
                // -------(1) 得到路线搜索句柄 ----------
                RouteSearch routeSearch = new RouteSearch(getApplication());
                // -------(2) 设置路径源点和目的坐标点
                RouteSearch.FromAndTo ft = new RouteSearch.FromAndTo(_startPoint,_endPoint);

                Log.e("Amap", "StartPoint "+_startPoint.getLatitude() +", "+_startPoint.getLongitude());
                Log.e("Amap", "EndPoint " + _endPoint.getLatitude() +", " +_endPoint.getLongitude());
                // fromAndTo包含路径规划的起点和终点，drivingMode表示驾车模式
                // 第三个参数表示途经点（最多支持16个），
                // 第四个参数表示避让区域（最多支持32个），第五个参数表示避让道路
                // -------(3) 设置查询条件 -----------
                RouteSearch.DriveRouteQuery driver_query
                        = new RouteSearch.DriveRouteQuery(ft, RouteSearch.DrivingDefault, null, null, "");

                // -------(4) 设置查询条件回调（主要在地图上绘图） -----------
                routeSearch.setRouteSearchListener(new RouteSearch.OnRouteSearchListener() {
                    @Override
                    public void onBusRouteSearched(BusRouteResult busRouteResult, int i) {
                        //公交查询路线规划完成
                    }

                    @Override
                    public void onDriveRouteSearched(DriveRouteResult driveRouteResult, int rCode) {
                        //驾车查询路线规划完成
                        Log.e("Amap", "++++ calc end " + rCode);

                        if (rCode != 1000) {
                            Log.e("Amap", "onDriveRouteSearched error rcode = "+rCode);
                            return ;
                        }

                        // 主要在地图上 描绘出路径
                        // 首先清楚地图上之前绘图
                        _amap.clear();

                        //这里面可能会有很多种查询的路径，我们默认使用第一个driveRouteResult.getPaths().get(0)
                        DrivePath drivePath = driveRouteResult.getPaths().get(0);
                        //开始绘图 创建路径覆盖物
                        DrivingRouteOverlay drivingRouteOverlay = new DrivingRouteOverlay(
                                getApplication(),
                                _amap,
                                drivePath,
                                _startPoint,
                                _endPoint,
                                null
                        );

                        //将路径中转图标去掉，否则影响视图效果
                        drivingRouteOverlay.setNodeIconVisibility(false);

                        //将路线覆盖物添加到地图中
                        drivingRouteOverlay.removeFromMap();
                        drivingRouteOverlay.addToMap();

                        //以最适应的缩进展示效果
                        drivingRouteOverlay.zoomToSpan();

                        tv_passenger_info.setText("约"+driveRouteResult.getTaxiCost()+" 元");
                       // drivePath.getDuration(); //时间

                        bt_startOrder.setText("等待司机接单...");

                        //给服务器发送请求下单消息
                        _driverGet = OBOJni.getInstence().StartOrder(_startPoint.getLongitude()+"",
                                                        _startPoint.getLatitude()+"",
                                                        _startAddr,
                                                        _endPoint.getLongitude()+"",
                                                        _endPoint.getLatitude()+"",
                                                        _finalAddr,
                                                        driveRouteResult.getTaxiCost()+"");


                        Log.e("PassgerMap", "StartOrder: longitude:"+_startPoint.getLongitude() +", latitude:"+_startPoint.getLatitude());
                    }

                    @Override
                    public void onWalkRouteSearched(WalkRouteResult walkRouteResult, int i) {
                        //步行查询路线规划完成
                    }

                    @Override
                    public void onRideRouteSearched(RideRouteResult rideRouteResult, int i) {
                        //骑行查询路线规划完成
                    }
                });

                //--------(5) 启动路线规划查询
                routeSearch.calculateDriveRouteAsyn(driver_query);
            }



            @Override
            public void onPoiItemSearched(PoiItem poiItem, int i) {
                //针对搜索 出每个PoiItem的回调

                if (i != 1000) {
                    Log.e("Amap", "onPoiItemSearched error");
                    return;
                }

            }

        });

        //4 开始异步搜索
        poiSearch.searchPOIAsyn();

        /* ---------------  获取POI数据 (end)-----------------*/
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

                        //获取当前所属城市
                        _city = aMapLocation.getCity();
                        _startAddr = aMapLocation.getAddress();

                        //获取当前坐标
                        if (_startPoint == null) {
                            _startPoint = new LatLonPoint(0, 0);
                        }

                        _startPoint.setLatitude(aMapLocation.getLatitude());
                        _startPoint.setLongitude(aMapLocation.getLongitude());


                        //取出经纬度
                        LatLng latLng = new LatLng(aMapLocation.getLatitude(),
                                aMapLocation.getLongitude());

                        //添加Marker显示定位位置
                        if (locationMarker == null) {
                            //如果是空的添加一个新的,icon方法就是设置定位图标，可以自定义
                            locationMarker = _amap.addMarker(new MarkerOptions()
                                    .position(latLng)
                                    .icon(BitmapDescriptorFactory.fromResource(R.drawable.location_marker)));
                        } else {
                            //已经添加过了，修改位置即可
                            locationMarker.setPosition(latLng);
                        }

                        // 以自我为中心 只执行一次
                        if (postMyLocationCenter == true) {
                            //得到当前坐标点
                            CameraPosition cp = _amap.getCameraPosition();
                            //然后可以移动到定位点,使用animateCamera就有动画效果
                            _amap.animateCamera(CameraUpdateFactory.newLatLngZoom(latLng, cp.zoom));
                            postMyLocationCenter = false;
                        }


                        //开始上传乘客地理位置信息 locationChanged业务
                        if (OBOJni.getInstence().getStatus().toString().equals(getResources().getString(R.string.PASSENGER_STATUS_WAIT))) {

                            OBOJni.getInstence().PassengerLocationChanged(aMapLocation.getLongitude()+"",
                                                                          aMapLocation.getLatitude()+"",
                                                                          aMapLocation.getAddress()+"",
                                                                          _endPoint.getLongitude()+"",
                                                                          _endPoint.getLatitude()+"",
                                                                          _finalAddr);
                            _driverGet = false;
                        }
                        else {

                            OBOJni.getInstence().PassengerLocationChanged(aMapLocation.getLongitude()+"",
                                    aMapLocation.getLatitude()+"",
                                    aMapLocation.getAddress()+"",
                                    "",
                                    "",
                                    "");
                        }

                        //设置乘客按钮文本
                        if(OBOJni.getInstence().getStatus().toString().equals(getString(R.string.PASSENGER_STATUS_IDLE))) {
                            if (_driverGet == false) {
                                bt_startOrder.setText("开始约车");
                            }
                            else {
                                bt_startOrder.setText("等待司机接单...");
                            }
                        }
                        else if (OBOJni.getInstence().getStatus().toString().equals(getString(R.string.PASSENGER_STATUS_WAIT))) {
                            bt_startOrder.setText("司机已经接单,等待司机...");
                        }
                        else if (OBOJni.getInstence().getStatus().toString().equals(getString(R.string.PASSENGER_STATUS_TRAVEL))) {
                            bt_startOrder.setText("司机已确认上车,正在行驶中...");
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


    void setAutoCompleteAddress() {

        //默认2个字才会开启匹配列表显示，这里改为1个字就开始显示
        autotv_finalAddr.setThreshold(1);

        autotv_finalAddr.addTextChangedListener(new TextWatcher() {
            //控件内容 改变之前会触发此回调
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            //当ui控件的内容发生改变，会触发此回调
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                //得到用户输入的 关键字
                final String keyword = autotv_finalAddr.getText().toString().trim();


                if (keyword ==null || keyword.length() == 0) {
                    return;
                }

                Log.e("Amap", "乘客匹配到的模糊关键字是 " + keyword);


                /*
                得到根据此模糊关键字搜索到的全部模糊匹配的地址 集合
                此集合由高德地图提供，不需要我们自己写模糊算法
                */

                // 得到查询条件
                InputtipsQuery query = new InputtipsQuery(keyword, _city);
                // 得到查询点搜索句柄（实际上高德地图会根据 搜索关键字 来定位匹配的地址）
                Inputtips search = new Inputtips(getApplicationContext(), query);

                // 设置 定位监听回调
                search.setInputtipsListener(new Inputtips.InputtipsListener() {
                    @Override
                    public void onGetInputtips(List<Tip> list, int rCode) {
                        // 高德地图会将全部定位到的 地址 放在list列表中，每个元素类型是Tip

                        // 下面就是将全部的地址信息，放在我们的_autotv_keyword控件中

                        // 1 首先将Tip list信息封装到一个 ArrayList<String>中

                        ArrayList<String> resultList = new ArrayList<String>();
                        for (int i = 0;i < list.size(); i++) {
                            resultList.add(list.get(i).getName());
                            Log.e("Amap", "根据 " + keyword + "匹配到的结果" + list.get(i).toString());

                        }

                        // 2 创建一个根据ArrayList<String>的ArrayAdapter<String>
                        ArrayAdapter<String> resultAdapter = new ArrayAdapter<String>(
                                getApplicationContext(),
                                android.R.layout.simple_list_item_1,
                                resultList
                        );

                        // 3 将这个Adapter设置给AutoCompleteTextView控件
                        autotv_finalAddr.setAdapter(resultAdapter);

                        // 4 启动通知 ，一旦这个Adapter数据内容发生改变，告知控件
                        resultAdapter.notifyDataSetChanged();
                    }
                });

                // 开始 定位
                search.requestInputtipsAsyn();
            }

            //控件内容 改变之后会触发此回调
            @Override
            public void afterTextChanged(Editable s) {

            }
        });
    }

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
