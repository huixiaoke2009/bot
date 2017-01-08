
#ifndef __CARROT_H__
#define __CARROT_H__

#include<stdint.h>
#include<string>
#include<map>
#include<vector>
#include<curl/curl.h>
#include <json/json.h>
#include <string.h>


using namespace std;

class CCarrot;
typedef int (*fnc_callback_t)(CCarrot* p, const string& str1, const string& str2);

typedef struct tagOnlineFriend
{
    uint64_t uin;
    uint64_t qqnum;
    int client_type;
    char status[64];

    tagOnlineFriend()
    {
        memset(this, 0x0, sizeof(tagOnlineFriend));
    }
}OnlineFriend;

typedef struct tagMessage
{
    uint64_t send_uin;
    uint64_t to_qqnum;
    uint64_t group_code;
    char message[102400];
    char msg_type[64];
    time_t send_time;
    tagMessage()
    {
        memset(this, 0x0, sizeof(tagMessage));
    }
}Message;

class CCarrot
{

friend int Callback4Default(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4VerifyLogin(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4GetScanState(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookieVF(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookiePN(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4GetQQNumByUin(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchMessage(CCarrot* p, const string& strHeader, const string& strResult);


public:
    CCarrot();
    ~CCarrot();
    
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
    
    //轮循消息
    int FetchMessage();
    //发送好友消息
    int SendMessageByQQnum(uint64_t qqnum, const char* message);
    int SendMessageByUin(uint64_t uin, const char* message);
    //发送群消息
    int SendGroupMsgByGroup(uint64_t groupnum, const char* message);
    int SendGroupMsgByUin(uint64_t uin, const char* message);
private:
    //每次是否是相同的会话
    bool m_bKeepAlive;

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

    //临时存储消息用的
    Message m_message;
};


#endif
