
#ifndef __CONFIG_H__
#define __CONFIG_H__

// smart api 接口
//获取二维码
const char* GET_QR = "https://ssl.ptlogin2.qq.com/ptqrshow";
//判断二维码状态
const char* SCAN_STATE = "https://ssl.ptlogin2.qq.com/ptqrlogin";
const char* SCAN_STATE_REFERER = "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http://w.qq.com/proxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001";
//获取鉴权参数ptwebqq
const char* REFERER_PT = "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1";
//获取鉴权参数vfwebqq
const char* WEBQQ_VERIFY = "http://s.web2.qq.com/api/getvfwebqq";
const char* REFERER_VF = "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1";
//获取鉴权参数uin和psessionid
const char* FETCH_PN = "http://d1.web2.qq.com/channel/login2";
const char* REFERER_PN = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

//判断是否需要重新扫码登录，并获取在线好友列表
const char* GET_ONLINE = "http://d1.web2.qq.com/channel/get_online_buddies2";
const char* REFERER_OL = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";


//轮循消息
const char* FETCH_MSG = "http://d1.web2.qq.com/channel/poll2";
const char* REFERER_FETCH_MSG = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

//发送好友消息
const char* SEND_MSG = "http://d1.web2.qq.com/channel/send_buddy_msg2";
const char* REFERER_SEND_MSG = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

//发送群消息
const char* SEND_GROUP_MSG = "http://d1.web2.qq.com/channel/send_qun_msg2";
const char* REFERER_SEND_GROUP_MSG = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

//发送讨论组消息
const char* SEND_DISCU_MSG = "http://d1.web2.qq.com/channel/send_discu_msg2";
const char* REFERER_SEND_DISCU_MSG = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";


//获取在线好友列表
const char* GET_USER_FRIEND = "http://s.web2.qq.com/api/get_user_friends2";
const char* REFERER_GET_USER_FRIEND = "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=3";

//通过uin获取QQ号
const char* GET_QQNUM_BY_UIN = "http://s.web2.qq.com/api/get_friend_uin2";
const char* REFERER_GET_QQNUM_BY_UIN = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

const char* QRPATH = "/tmp/qr.png";
const char* COOKIEPATH = "/tmp/smartqq.cookie";
const char* SELFCOOKIEPATH = "/tmp/selfsmartqq.cookie";


//数据库配置
const char* MYSQLDB_IP = "127.0.0.1";
const char* MYSQLDB_USER = "root";
const char* MYSQLDB_PASSWD = "bhgame@123";
const char* MYSQLDB_DBNAME = "robot";
const unsigned short int MYSQLDB_PORT = 3306;
const char* MYSQLDB_CHAT_TABLE = "chat";
#endif
