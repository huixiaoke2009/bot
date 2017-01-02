
#include "carrot.h" 
#include "config.h"


using namespace std;

static size_t OnHeaderData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    string* str = dynamic_cast<string*>((string *)lpVoid);
    if( NULL == str || NULL == buffer )  
    {  
        return -1;
    }
    
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
} 

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{  
    string* str = dynamic_cast<string*>((string *)lpVoid);
    if( NULL == str || NULL == buffer )  
    {  
        return -1;
    }
    
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
} 


static size_t OnReadData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{  
    return nmemb;
}

static size_t OnDownloadFile(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    int written = fwrite(buffer, size, nmemb, (FILE *)lpVoid);
    if(written <= 0)
    {
        printf("fwrite failed, written=%d\n", written);
        return -1;
    }
    
    return written;
}

static int split(const string& str, vector<string>& ret_, string sep = ",")
{
    if (str.empty())
    {
        return 0;
    }

    string tmp;
    string::size_type pos_begin = str.find_first_not_of(sep);
    string::size_type comma_pos = 0;

    while (pos_begin != string::npos)
    {
        comma_pos = str.find(sep, pos_begin);
        if (comma_pos != string::npos)
        {
            tmp = str.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + sep.length();
        }
        else
        {
            tmp = str.substr(pos_begin);
            pos_begin = comma_pos;
        }

        if (!tmp.empty())
        {
            ret_.push_back(tmp);
            tmp.clear();
        }
    }
    return 0;
}

static string strip(const string& str, const char c = ' ')
{
    string::size_type pos = str.find_first_not_of(c);
    if (pos == string::npos)
    {
        return str;
    }
    string::size_type pos2 = str.find_last_not_of(c);
    if (pos2 != string::npos)
    {
        return str.substr(pos, pos2 - pos + 1);
    }
    return str.substr(pos);
}

static int Callback4VerifyLogin(const string& strHeader, const string& strResult)
{
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    return 0;
}

static int Callback4GetScanState(const string& strHeader, const string& strResult)
{
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    return 0;
}

CCarrot::CCarrot()
{
    m_UserID = 0;
    m_LoginStatus = 0;
}

CCarrot::~CCarrot()
{
    
    
}

int CCarrot::Init()
{
    m_UserID = 229845213;
    m_strPasswd = "Hui900705";

    //全局初始化
    CURLcode CURLRet = curl_global_init(CURL_GLOBAL_ALL);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_global_init failed, %s\n", curl_easy_strerror(CURLRet));
        return -1;
    }
    
    printf("libcur初始化成功，版本信息为:%s\n", curl_version());
    
    printf("Init success\n");
    
    return 0;
}

int CCarrot::Get(const char* pUrl, const map<string,string>& mapParam, fnc_callback_t func, const char* pRerfer)
{
    printf("Get begin ...\n");
    
    //启用一个会话
    m_handle = curl_easy_init();
    if(m_handle == NULL)
    {
        printf("curl_easy_init failed\n");
        return -1;
    }
    
    string strUrl = pUrl;
    map<string,string>::const_iterator iter = mapParam.begin();
    while(iter != mapParam.end())
    {
        if(iter == mapParam.begin())
        {
            strUrl.append("?");
        }
        
        char tmp[1024] = {0};
        snprintf(tmp, sizeof(tmp), "%s=%s", iter->first.c_str(), iter->second.c_str());
        strUrl.append(tmp);
        iter++;
        if(iter != mapParam.end())
        {
            strUrl.append("&");
        }
        else
        {
            break;
        }
    }
    
    printf("url=%s\n", strUrl.c_str());
    
    CURLcode CURLRet = CURLE_OK;
    
    string strHeaderRsp;
    string strReadRsp;
    string strWriteRsp;
    
    //设置url
    curl_easy_setopt(m_handle, CURLOPT_URL, strUrl.c_str());
    
    //设置http头部信息
    struct curl_slist* pHeaders = SetHttpHeader(pRerfer);
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, pHeaders);
    
    //一旦接收到http 头部数据后将调用该函数
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, OnHeaderData);
    //CURLOPT_HEADERFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, (void *)&strHeaderRsp);

    //需要读取数据传递给远程主机时将调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb,void *stream).
    curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, OnReadData);
    //CURLOPT_READFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_READDATA, (void *)&strReadRsp);
    
    //函数将在libcurl接收到数据后被调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, OnWriteData);
    //CURLOPT_WRITEFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void *)&strWriteRsp);
    
    //设置连接等待时间
    curl_easy_setopt(m_handle, CURLOPT_CONNECTTIMEOUT, 30);
    
    //设置传输时间
    curl_easy_setopt(m_handle, CURLOPT_TIMEOUT, 30);

    //设置cookie
    SetHttpCookie();
    
    //执行
    CURLRet = curl_easy_perform(m_handle);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_easy_perform failed, %s\n", curl_easy_strerror(CURLRet));
        return -1;
    }
    
    //保存cookie
    SaveHttpCookie();

    //关闭一个会话
    curl_easy_cleanup(m_handle);
    
    int Ret = func(strHeaderRsp, strWriteRsp);
    
    printf("Get end ...\n");
    
    return Ret;
}


int CCarrot::Post(const char* pUrl, fnc_callback_t func, const char* pRerfer)
{
    printf("Post begin ...\n");
    
    //启用一个会话
    m_handle = curl_easy_init();
    if(m_handle == NULL)
    {
        printf("curl_easy_init failed\n");
        return -1;
    }
    
    CURLcode CURLRet = CURLE_OK;
    
    string strHeaderRsp;
    string strReadRsp;
    string strWriteRsp;
    
    //设置url
    curl_easy_setopt(m_handle, CURLOPT_URL, pUrl);
    
    //设置为POST方式
    curl_easy_setopt(m_handle, CURLOPT_POST, 1);
    
    //设置http头部信息
    struct curl_slist* pHeaders = SetHttpHeader(pRerfer);
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, pHeaders);
    
    //设置POST参数
    //"wd=hava&hehe=123456"
    //VER=1.1&CMD=Login&SEQ=&UIN=&PS=&M5=1&LC=9326B87B234E7235
    char PostBuff[1024] = {0};
    //snprintf(PostBuff, sizeof(PostBuff), "%s=%ld&%s=%s", "userid", m_UserID, "passwd", m_strPasswd.c_str());
    snprintf(PostBuff, sizeof(PostBuff), "VER=1.1&CMD=Login&SEQ=%ld&UIN=%ld&PS=%s&M5=1&LC=9326B87B234E7235",time(NULL),m_UserID,m_strPasswd.c_str());
    curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, PostBuff); 
    
    //一旦接收到http 头部数据后将调用该函数
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, OnHeaderData);
    //CURLOPT_HEADERFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, (void *)&strHeaderRsp);

    //需要读取数据传递给远程主机时将调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb,void *stream).
    curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, NULL);
    //CURLOPT_READFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_READDATA, (void *)&strReadRsp);
        
    //函数将在libcurl接收到数据后被调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, OnWriteData);
    //CURLOPT_WRITEFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void *)&strWriteRsp);
    
    
    //设置连接等待时间
    curl_easy_setopt(m_handle, CURLOPT_CONNECTTIMEOUT, 30);
    
    //设置传输时间
    curl_easy_setopt(m_handle, CURLOPT_TIMEOUT, 30);

    //设置cookie
    SetHttpCookie();
    
    //执行
    CURLRet = curl_easy_perform(m_handle);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_easy_perform failed, %s\n", curl_easy_strerror(CURLRet));
        return -1;
    }
    
    //保存cookie
    SaveHttpCookie();
    
    //关闭一个会话
    curl_easy_cleanup(m_handle);
    
    int Ret = func(strHeaderRsp, strWriteRsp);
    
    printf("Post end ...\n");
    
    return Ret;
}

int CCarrot::Download2File(const char* pUrl, const char* file_path, const map<string,string>& mapParam)
{
    printf("Download2File begin ...\n");
    
    //启用一个会话
    m_handle = curl_easy_init();
    if(m_handle == NULL)
    {
        printf("curl_easy_init failed\n");
        return -1;
    }
    
    FILE* outfile = fopen(file_path, "w+");
    if(outfile == NULL)
    {
        printf("open file %s failed\n", file_path);
        return -1;
    }
    
    string strUrl = pUrl;
    map<string,string>::const_iterator iter = mapParam.begin();
    while(iter != mapParam.end())
    {
        if(iter == mapParam.begin())
        {
            strUrl.append("?");
        }
        
        char tmp[1024] = {0};
        snprintf(tmp, sizeof(tmp), "%s=%s", iter->first.c_str(), iter->second.c_str());
        strUrl.append(tmp);
        iter++;
        if(iter != mapParam.end())
        {
            strUrl.append("&");
        }
        else
        {
            break;
        }
    }
    
    printf("url=%s\n", strUrl.c_str());
    printf("file_path=%s\n", file_path);
    
    CURLcode CURLRet = CURLE_OK;
    
    string strHeaderRsp;
    string strReadRsp;
    
    //设置url
    curl_easy_setopt(m_handle, CURLOPT_URL, pUrl);
    
    //设置http头部信息
    struct curl_slist* pHeaders = SetHttpHeader();
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, pHeaders);
    
    //一旦接收到http 头部数据后将调用该函数
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, OnHeaderData);
    //CURLOPT_HEADERFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, (void *)&strHeaderRsp);

    //需要读取数据传递给远程主机时将调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb,void *stream).
    curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, OnReadData);
    //CURLOPT_READFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_READDATA, (void *)&strReadRsp);
    
    //函数将在libcurl接收到数据后被调用
    //函原型:size_t function(void *ptr, size_t size, size_t nmemb, void *stream); 
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, OnDownloadFile);
    //CURLOPT_WRITEFUNCTION函数中的stream指针的来源
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void *)outfile);
    
    //设置连接等待时间
    curl_easy_setopt(m_handle, CURLOPT_CONNECTTIMEOUT, 30);
    
    //设置传输时间
    curl_easy_setopt(m_handle, CURLOPT_TIMEOUT, 30);

    //设置cookie
    SetHttpCookie();
    
    //执行
    CURLRet = curl_easy_perform(m_handle);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_easy_perform failed, %s\n", curl_easy_strerror(CURLRet));
        return -1;
    }
    
    fclose(outfile);
    
    printf("file %s download ok\n", file_path);
    
    //保存cookie
    SaveHttpCookie();
    
    //关闭一个会话
    curl_easy_cleanup(m_handle);
    
    printf("Download2File end ...\n");
    
    return 0;
}

struct curl_slist* CCarrot::SetHttpHeader(const char* pRerfer)
{
    struct curl_slist* pHeaders = NULL;
    pHeaders = curl_slist_append(pHeaders, "Accept:application/json");
    pHeaders = curl_slist_append(pHeaders, "Content-Type:text/html");
    pHeaders = curl_slist_append(pHeaders, "charset:utf-8");
    pHeaders = curl_slist_append(pHeaders, "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36");
    if(pRerfer)
    {
        char buff[10240] = {0};
        snprintf(buff, sizeof(buff), "Referer:%s", pRerfer);
        pHeaders = curl_slist_append(pHeaders, buff);
    }
    
    return pHeaders;
}

void CCarrot::SetHttpCookie()
{
    printf("SetHttpCookie begin ...\n");
    
    CURLcode CURLRet = CURLE_OK;

    //会使curl下一次发请求时从指定的文件中读取cookie
    //CURLRet = curl_easy_setopt(m_handle, CURLOPT_COOKIEFILE, COOKIEPATH);

    //会使curl在调用curl_easy_cleanup的时候把cookie保存到指定的文件中
    CURLRet = curl_easy_setopt(m_handle, CURLOPT_COOKIEJAR, COOKIEPATH);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_easy_perform failed, %s\n", curl_easy_strerror(CURLRet));
    }
    
    map<string,string>::iterator iter = m_mapCookie.begin();
    for(; iter != m_mapCookie.end(); iter++)
    {
        char buff[2048] = {0};
        snprintf(buff, sizeof(buff), "%s=%s", iter->first.c_str(), iter->second.c_str());
        curl_easy_setopt(m_handle, CURLOPT_COOKIE, buff);
    }
    
    
    //会把指定的cookie字符串列表加入easy handle维护的cookie列表中(相当于curl_easy_cleanup之前一直有效)
    //curl_easy_setopt(m_handle, CURLOPT_COOKIELIST, "a=b;c=d");
    
    //用于设置一个分号分隔的“NAME=VALUE”列表，用于在HTTP request header中设置Cookie header。
    //curl_easy_setopt(m_handle, CURLOPT_COOKIE, "a=b");
    printf("SetHttpCookie end ...\n");
}

void CCarrot::SaveHttpCookie()
{
    printf("SaveHttpCookie begin ...\n");
    
    CURLcode CURLRet = CURLE_OK;
    struct curl_slist *cookies;
    struct curl_slist *nc;
    
    CURLRet = curl_easy_getinfo(m_handle, CURLINFO_COOKIELIST, &cookies);            //获取cookielist信息
    if( CURLRet != CURLE_OK)
    {
       printf("get cookie info failed\n");
       return;
    }
    
    nc = cookies;
    while(nc)
    {
        vector<string> result;
        split(nc->data, result, "\t");
        if(result.size() == 7)
        {
            m_mapCookie[strip(result[5])] = strip(result[6]);
        }
        else
        {
            printf("cookie len is invalid\n");
        }
        
        nc = nc->next;
    }
    
    curl_slist_free_all(cookies);
    
    printf("SaveHttpCookie end ...\n");
}

int CCarrot::GetQR()
{
    printf("GetQR begin ...\n");
    
    //获取二维码
    map<string,string> mapParam;
    mapParam["appid"] = "501004106";
    mapParam["e"] = "0";
    mapParam["l"] = "M";
    mapParam["s"] = "5";
    mapParam["d"] = "72";
    mapParam["v"] = "4";
    mapParam["t"] = "0.1";
    
    int Ret = Download2File(GET_QR, QRPATH, mapParam);
    
    printf("GetQR end ...\n");
    
    return Ret;
}

bool CCarrot::VerifyLogin()
{
    printf("VerifyLogin begin ...\n");

    bool flag = false;
    if (m_mapCookie.find("psessionid") != m_mapCookie.end() and m_mapCookie.find("vfwebqq") != m_mapCookie.end())
    {
        map<string,string> mapParam;
        mapParam["vfwebqq"] = m_mapCookie["vfwebqq"];
        mapParam["clientid"] = "53999199";
        mapParam["psessionid"] = m_mapCookie["psessionid"];
        mapParam["t"] = m_mapCookie["0.1"];
        if(Get(GET_ONLINE, mapParam, Callback4VerifyLogin, REFERER_OL))
        {
            return true;
        }
    }
    else
    {
        flag = false;
    }
    
    printf("VerifyLogin end ...\n");
    
    return flag;
}

int CCarrot::GetScanState()
{
    map<string,string> mapParam;
    mapParam["webqq_type"] = "10";
    mapParam["remember_uin"] = "1";
    mapParam["login2qq"] = "1";
    mapParam["aid"] = "501004106";
    mapParam["u1"] = "http://w.qq.com/proxy.html?login2qq=1&webqq_type=10";
    mapParam["ptredirect"] = "0";
    mapParam["ptlang"] = "2052";
    mapParam["daid"] = "164";
    mapParam["from_ui"] = "1";
    mapParam["pttype"] = "1";
    mapParam["dumy"] = "";
    mapParam["fp"] = "loginerroralert";
    mapParam["action"] = "0-0-157510";
    mapParam["mibao_css"] = "m_webqq";
    mapParam["t"] = "1";
    mapParam["g"] = "1";
    mapParam["js_type"] = "0";
    mapParam["js_ver"] = "10143";
    mapParam["login_sig"] = "";
    mapParam["pt_randsalt"] = "0";
    
    //while(true)
    {
        sleep(2);
        
        Get(SCAN_STATE, mapParam, Callback4GetScanState, SCAN_STATE_REFERER);
    }
    
    return 0;
}

int CCarrot::ParserCookieFile()
{
    FILE* cookiefile = fopen(COOKIEPATH, "r");
    if(cookiefile == NULL)
    {
        printf("open %s failed\n", COOKIEPATH);
        return -1;
    }
    
    while(true)
    {
        char buff[10240] = {0};
        if(fgets(buff, sizeof(buff), cookiefile))
        {
            if(strip(buff)[0] == '#' or strip(buff)[0] == '\n')
            {
                continue;
            }
            
            vector<string> result;
            split(strip(buff), result, "\t");
            if(result.size() == 7)
            {
                m_mapCookie[strip(result[5], '\n')] = strip(result[6], '\n');
            }
            else
            {
                printf("cookie len is invalid\n");
                return -1;
            }
        }
        else
        {
            break;
        }
    }
    
    return 0;
}

int CCarrot::Run()
{
    //加载本地cookie文件
    if(ParserCookieFile() != 0)
    {
        return -1;
    }
    
    //登录
    while(!VerifyLogin())
    {
        GetQR();
        GetScanState();
    }
    
    //判断登录状态
    
    return 0;
}


void CCarrot::Finish()
{
    //全局清理
    curl_global_cleanup();
}