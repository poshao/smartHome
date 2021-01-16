# 智能开关设计说明

### 功能设计
* 上电自动开启STA+AP模式,尝试使用默认配置连接路由器,在STA模式下成功连接路由器且在20秒内没有链接自动切换为STA模式
* 内置WebServer进行IO控制及相关配置
* 支持定时开关设定

### api列表
#### AP相关配置
[GET] /apis?m=ap
[POST] /apis?m=ap

#### Station相关配置
* 连接配置
[GET,POST] /apis?m=sta

``` json
{
    "ssid":"Snoopy",
    "security":"WPK2",
    "password":"helloboy"
}
```

* 设备名称
/apis?m=sta&key=hostname

* 修改接口mac地址
/apis?m=sta&key=mac

* 修改ip获取方式为dhcp
[POST] /apis?m=sta&t=auto

* 修改固定ip
/apis?m=sta&t=static

``` json
{
    "ip":"192.168.10.2",
    "mask":24,
    "gateway":"192.168.10.1"
}
```
#### IO相关配置
/light 显示灯状态
/light?on 开灯
/light?off 关灯
/light?on=n n秒后自动关灯
#### 登录相关配置