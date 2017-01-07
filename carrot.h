
#ifndef __CARROT_H__
#define __CARROT_H__

#include<stdint.h>
#include<string>
#include<map>
#include<vector>
#include<curl/curl.h>
#include <json/json.h>

using namespace std;

class CCarrot;
typedef int (*fnc_callback_t)(CCarrot* p, const string& str1, const string& str2);

class CCarrot
{

friend int Callback4Default(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4VerifyLogin(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4GetScanState(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookieVF(CCarrot* p, const string& strHeader, const string& strResult);
friend int Callback4FetchCookiePN(CCarrot* p, const string& strHeader, const string& strResult);

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
    int ParserCookieFile();
    //获取http返回状态
    int GetHttpStatus();
    //设置htt返回状态
    int SetHttpStatus();
    
//业务逻辑
private:
    //获取二维码
    int GetQR();
    //验证是否登录成功
    bool VerifyLogin();
    //判断二维码状态
    int GetScanState();
    //这几个函数好像都在获取cookie的样子
    int FetchCookiePT();
    int FetchCookieVF();
    int FetchCookiePN();
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
    map<std::string, string> m_mapCookie2;
    //FetchCookiePT函数使用的一个url，从GetScanState函数获得
    std::string m_strUrl;

    uint64_t m_UserID;
    std::string m_strPasswd;
    //在线状态
    int m_LoginStatus;

    
};


#endif
