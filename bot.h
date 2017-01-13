
#ifndef __BOT_H__
#define __BOT_H__

#include<stdint.h>
#include<string>
#include<map>
#include<vector>
#include<curl/curl.h>
#include <json/json.h>
#include <string.h>
#include "mysql_wrap.h"


using namespace std;
using namespace mmlib;

class CBot;
typedef int (*fnc_callback_t)(CBot* p, const string& str1, const string& str2);

typedef struct tagOnlineFriend
{
    uint64_t uin;       //服务器上的临时ID
    uint64_t qqnum;     //QQ号
    int client_type;    //客户端类型
    char status[64];    //状态
    
    tagOnlineFriend()
    {
        memset(this, 0x0, sizeof(tagOnlineFriend));
    }
}OnlineFriend;

typedef struct tagMessageDebris
{
    char type;  //是否为表情，0文字，1表情
    char buff[10240]; //类型
    int code;      //编码

    tagMessageDebris()
    {
        memset(this, 0x0, sizeof(tagMessageDebris));
    }
}MessageDebris;

typedef struct tagMessageUnit
{
    unsigned int debris_num;
    MessageDebris debris[32];
    
    tagMessageUnit()
    {
        memset(this, 0x0, sizeof(tagMessageUnit));
    }

    void Copy(const tagMessageUnit& o)
    {
        memcpy(this, &o, sizeof(tagMessageUnit));
    }

}MessageUnit;

typedef struct tagMessage
{
    uint64_t send_uin;      //发送消息的用户临时ID
    uint64_t to_qqnum;      //接收消息的QQ号，正常就是自己的QQ号
    uint64_t group_code;    //群临时ID
    uint64_t did;           //讨论组临时ID
    char msg_type[64];      //服务器上的消息类型，用于区分是群消息还是好友消息
    time_t send_time;       //消息发送时间

    MessageUnit message;
    
    tagMessage()
    {
        memset(this, 0x0, sizeof(tagMessage));
    }
}Message;

class CBot
{

friend int Callback4Default(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4VerifyLogin(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4GetScanState(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookieVF(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookiePN(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4GetQQNumByUin(CBot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchMessage(CBot* p, const string& strHeader, const string& strResult);


public:
    CBot();
    ~CBot();
    
    int Init();
    int Run();
    void Finish();
    
    
private:
    bool CreateSession();
    void FinishSession();


//http相关逻辑    
private:
    // http get
    int Get(const char* pUrl, const map<string,string>& mapParam, fnc_callback_t fun, const char* pRerfer = NULL);
    // http post
    int Post(const char* pUrl, const map<string,string>& mapParam, fnc_callback_t fun, const char* pRerfer = NULL);
    int Post(const char* pUrl, const char* p, fnc_callback_t fun, const char* pRerfer = NULL);
    // http download file
    int Download2File(const char* pUrl, const char* file_path, const map<string,string>& mapParam);
    //set the http header for a request
    void SetHttpHeader();
    //set the http cookie for a request
    void SetHttpCookie();
    //save the http cookie
    void SaveHttpCookie();
    //解析本地cookie文件
    int ParserSelfCookieFile();
    //保存本地cookie文件
    int SaveSelfCookieFile();
    //获取http返回状态
    int GetHttpStatus();
    //设置htt返回状态
    int SetHttpStatus();
    
//业务逻辑
private:
    /****************************************************
    登录的目的是获得以下五个参数，用于之后请求其它接口：
    ptwebqq：保存在Cookie中的鉴权信息
    vfwebqq：类似于Token的鉴权信息
    psessionid：类似于SessionId的鉴权信息
    clientid：设备id，为固定值53999199
    uin：登录用户id（其实就是当前登录的QQ号）
    ****************************************************/
    //获取二维码
    int GetQR();
    //判断是否需要重新扫码登录，获取好友在线列表
    bool VerifyLogin();
    //判断二维码状态
    int GetScanState();
    //获取鉴权参数ptwebqq
    int FetchCookiePT();
    //获取鉴权参数vfwebqq
    int FetchCookieVF();
    //获取鉴权参数uin和psessionid
    int FetchCookiePN();

    //获取在线好友列表
    int GetUserFriend();
    //由uin拿到QQ号
    int GetQQNumByUin(uint64_t uin, uint64_t& qqnum);
    
    //接收消息
    int FetchMessage();
    
    //发送好友消息
    int SendFriendMsgByQQnum(uint64_t qqnum, const char* message);
    int SendFriendMsgByUin(uint64_t uin, const char* message);
    int SendFriendMsgUnitByQQnum(uint64_t qqnum, const MessageUnit& o);
    int SendFriendMsgUnitByUin(uint64_t uin, const MessageUnit& o);
    
    //发送群消息
    int SendGroupMsgByGroupnum(uint64_t groupnum, const char* message);
    int SendGroupMsgByUin(uint64_t uin, const char* message);
    int SendGroupMsgUnitByGroupnum(uint64_t qqnum, const MessageUnit& o);
    int SendGroupMsgUnitByUin(uint64_t uin, const MessageUnit& o);
    
    //发送讨论组消息
    int SendDiscuMsgByDiscunum(uint64_t did, const char* message);
    int SendDiscuMsgByUin(uint64_t uin, const char* message);
    int SendDiscuMsgUnitByGroupnum(uint64_t qqnum, const MessageUnit& o);
    int SendDiscuMsgUnitByUin(uint64_t uin, const MessageUnit& o);
    
    //发送消息底层代码
    int SendMsg(uint64_t uin, const char * message, int type);
    int SendMsgByMsgUnit(uint64_t uin, const MessageUnit& o, int type);

    //判断一条消息是否为命令
    int IsCommand(const MessageUnit& o, MessageUnit& oKey, MessageUnit& oValue);

    //string与messageUnit互转
    int String2MessageUnit(const string& s, MessageUnit& o);
    int MessageUnit2String(const MessageUnit& o, string& s);
    int MessageUnit2String2(const MessageUnit& o, string& s);
private:

    //libcurl句柄
    CURL* m_handle;
    //http 头部
    struct curl_slist* m_pHeaders;
    int m_HttpStatus;
    //http cookie信息
    map<std::string, string> m_mapCookie;
    //FetchCookiePT函数使用的一个url，从GetScanState函数获得
    std::string m_strUrl;
    
    //在线好友列表状态
    map<uint64_t, OnlineFriend> m_mapOnlineFriend;

    CMySQL m_mysql;
    
    //临时存储消息用的
    Message m_message;
};


#endif
