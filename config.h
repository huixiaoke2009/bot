
#ifndef __CONFIG_H__
#define __CONFIG_H__

// smart api 接口
const char* GET_QR = "https://ssl.ptlogin2.qq.com/ptqrshow";
const char* SCAN_STATE = "https://ssl.ptlogin2.qq.com/ptqrlogin";
const char* SCAN_STATE_REFERER = "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http://w.qq.com/proxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001";
const char* REFERER_PT = "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1";
const char* WEBQQ_VERIFY = "http://s.web2.qq.com/api/getvfwebqq";
const char* REFERER_VF = "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1";
const char* FETCH_PN = "http://d1.web2.qq.com/channel/login2";
const char* REFERER_PN = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";
const char* FETCH_MSG = "http://d1.web2.qq.com/channel/poll2";
const char* REFERER_MSG = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";
const char* GET_ONLINE = "http://d1.web2.qq.com/channel/get_online_buddies2";
const char* REFERER_OL = "http://d1.web2.qq.com/proxy.html?v=20151105001&callback=1&id=2";

const char* POLL2 = "http://d1.web2.qq.com/channel/poll2";
const char* SEND_QUN_MSG2 = "http://d1.web2.qq.com/channel/send_qun_msg2";
const char* REC_LIST2 = "http://d1.web2.qq.com/channel/get_recent_list2";


const char* QRPATH = "/tmp/qr.png";
const char* COOKIEPATH = "/tmp/smartqq.cookie";
#endif