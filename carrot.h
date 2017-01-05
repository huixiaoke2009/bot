
#ifndef __CARROT_H__
#define __CARROT_H__

#include<stdint.h>
#include<string>
#include<map>
#include<vector>
#include<curl/curl.h>

class CCarrot;
typedef int (*fnc_callback_t)(CCarrot* p, const std::string& str1, const std::string& str2);

class CCarrot
{
public:
    CCarrot();
    ~CCarrot();
    
    int Init();
    int Run();
    void Finish();

public:
    void SetUrl(const std::string& str);
    
private:
    bool CreateSession();
    void FinishSession();


//http相关逻辑    
private:
    // http get
    int Get(const char* pUrl, const std::map<std::string,std::string>& mapParam, fnc_callback_t fun, const char* pRerfer = NULL);
    // http post
    int Post(const char* pUrl, const std::map<std::string,std::string>& mapParam, fnc_callback_t fun, const char* pRerfer = NULL);
    // http download file
    int Download2File(const char* pUrl, const char* file_path, const std::map<std::string,std::string>& mapParam);
    //set the http header for a request
    void SetHttpHeader();
    //set the http cookie for a request
    void SetHttpCookie();
    //save the http cookie
    void SaveHttpCookie();
    //解析本地cookie文件
    int ParserCookieFile();

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

private:
    uint64_t m_UserID;
    std::string m_strPasswd;
    
    CURL* m_handle;
    struct curl_slist* m_pHeaders;
    std::map<std::string, std::string> m_mapCookie;
    std::string m_strUrl;
    
    int m_LoginStatus;
    bool m_bKeepAlive;
};


#endif