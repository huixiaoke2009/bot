
#include "carrot.h" 
#include "config.h"

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

    if(str.length() == 1)
    {
        return string();
    }

    string::size_type pos2 = str.find_last_not_of(c);
    if (pos2 != string::npos)
    {
        return str.substr(pos, pos2 - pos + 1);
    }
    return str.substr(pos);
}

void parser_ptuicb(const string& str, vector<string>& vct)
{
    string str2 = strip(strip(str, '\n'));
    
    if(str.substr(0, 6) != "ptuiCB")
    {
        vct.push_back(str2);
    }

    string str3 = str2.substr(7, str2.length()-10);
    split(str3, vct, ",");
    for(unsigned int i = 0; i < vct.size(); i++)
    {
        vct[i] = strip(vct[i], '\'');
    }
}

unsigned char ToHex(unsigned char x)   
{   
    return  x > 9 ? x + 55 : x + 48;   
}  
  
unsigned char FromHex(unsigned char x)   
{   
    unsigned char y;  
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;  
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;  
    else if (x >= '0' && x <= '9') y = x - '0';  
    return y;  
}  
  
std::string UrlEncode(const std::string& str)  
{  
    std::string strTemp = "";  
    size_t length = str.length();  
    for (size_t i = 0; i < length; i++)  
    {  
        if (isalnum((unsigned char)str[i]) ||   
            (str[i] == '-') ||  
            (str[i] == '_') ||   
            (str[i] == '.') ||   
            (str[i] == '~'))  
            strTemp += str[i];  
        else if (str[i] == ' ')  
            strTemp += "+";  
        else  
        {  
            strTemp += '%';  
            strTemp += ToHex((unsigned char)str[i] >> 4);  
            strTemp += ToHex((unsigned char)str[i] % 16);  
        }  
    }  
    return strTemp;  
}  
  
std::string UrlDecode(const std::string& str)  
{  
    std::string strTemp = "";  
    size_t length = str.length();  
    for (size_t i = 0; i < length; i++)  
    {  
        if (str[i] == '+') strTemp += ' ';  
        else if (str[i] == '%')  
        {   
            unsigned char high = FromHex((unsigned char)str[++i]);  
            unsigned char low = FromHex((unsigned char)str[++i]);  
            strTemp += high*16 + low;  
        }  
        else strTemp += str[i];  
    }  
    return strTemp;  
}  


inline int Callback4Default(CCarrot* p, const string& strHeader, const string& strResult)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    printf("##############################\n");
    
    int HttpStatus = p->GetHttpStatus();
    if(HttpStatus != 200 and HttpStatus != 302)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Http Status is illeage, %d\n", HttpStatus);
        printf("##############################\n");
        return -1;
    }
    
    return 0;
}


inline int Callback4VerifyLogin(CCarrot* p, const string& strHeader, const string& strResult)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    printf("##############################\n");
    
    int HttpStatus = p->GetHttpStatus();
    if(HttpStatus != 200)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Http Status is illeage, %d\n", HttpStatus);
        printf("##############################\n");
        return -1;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strResult, root, false))
    {
        printf("json parse failed\n");
        return -1;
    }

    int retcode = root["retcode"].asInt();
    if(retcode != 0)
    {
        return -1;
    }
    
    return 0;
}

inline int Callback4GetScanState(CCarrot* p, const string& strHeader, const string& strResult)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    printf("##############################\n");

    int HttpStatus = p->GetHttpStatus();
    if(HttpStatus != 200)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Http Status is illeage, %d\n", HttpStatus);
        printf("##############################\n");
        return -1;
    }

    vector<string> vct;
    parser_ptuicb(strResult, vct);

    if(vct[0] == "0")
    {
        //登录成功
        p->m_strUrl = vct[2];
        printf("Login processing...\n");
        return 0;
    }
    else if(vct[0] == "65")
    {
        //二维码已失效
        printf("QR expired, downloading it again...\n");
        return 1;
    }
    else if(vct[0] == "66")
    {
        //二维码未失效
        printf("Please scan the QR code...\n");
        return 2;
    }
    else if(vct[0] == "67")
    {
        //二维码认证中
        printf("Authing...\n");
        return 3;
    }
    else
    {
        printf("HTTP request error...retrying...\n");
        return -1;
    }

    return -1;
}

inline int Callback4FetchCookieVF(CCarrot* p, const string& strHeader, const string& strResult)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    printf("##############################\n");

    int HttpStatus = p->GetHttpStatus();
    if(HttpStatus != 200)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Http Status is illeage, %d\n", HttpStatus);
        printf("##############################\n");
        return -1;
    }
    
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strResult, root, false))
    {
        printf("json parse failed\n");
        return -1;
    }

    int retcode = root["retcode"].asInt();
    if(retcode != 0)
    {
        return -1;
    }
    
    p->m_mapCookie["vfwebqq"] = root["result"]["vfwebqq"].asString();
    
    return 0;
}

inline int Callback4FetchCookiePN(CCarrot* p, const string& strHeader, const string& strResult)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("strHeader=%s\n", strHeader.c_str());
    printf("strResult=%s\n", strResult.c_str());
    printf("##############################\n");
    
    int HttpStatus = p->GetHttpStatus();
    if(HttpStatus != 200)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("Http Status is illeage, %d\n", HttpStatus);
        printf("##############################\n");
        return -1;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(strResult.c_str(), root, false))
    {
        printf("json parse failed\n");
        return -1;
    }

    int retcode = root["retcode"].asInt();
    if(retcode != 0)
    {
        return -1;
    }

    p->m_mapCookie["psessionid"] = root["result"]["psessionid"].asString();
    p->m_mapCookie["uin"] = root["result"]["p_uin"].asString();
    
    return 0;
}


CCarrot::CCarrot()
{
    m_handle = NULL;
    m_pHeaders = NULL;
    m_HttpStatus = 0;
    m_UserID = 0;
    m_LoginStatus = 0;
    m_bKeepAlive = true;
}

CCarrot::~CCarrot()
{
    
    
}

int CCarrot::Init()
{
    m_UserID = 229845213;
    m_strPasswd = "HUI900705";

    //全局初始化
    CURLcode CURLRet = curl_global_init(CURL_GLOBAL_ALL);
    if(CURLRet != CURLE_OK)
    {
        printf("curl_global_init failed, %s\n", curl_easy_strerror(CURLRet));
        return -1;
    }
    
    printf("libcur初始化成功，版本信息为:%s\n", curl_version());
    
    printf("Init success, m_bKeepAlive=%d\n", m_bKeepAlive);

    return 0;
}

int CCarrot::Get(const char* pUrl, const map<string,string>& mapParam, fnc_callback_t func, const char* pRerfer)
{
    printf("Get begin ...\n");
    
    if(!CreateSession())
    {
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

    //printf("url=%s\n", strUrl.c_str());
    
    CURLcode CURLRet = CURLE_OK;
    
    string strHeaderRsp;
    string strReadRsp;
    string strWriteRsp;
    
    //设置url
    curl_easy_setopt(m_handle, CURLOPT_URL, strUrl.c_str());

    //设置http头部信息
    SetHttpHeader();
    
    //设置referer
    if(pRerfer)
    {
        curl_easy_setopt(m_handle, CURLOPT_REFERER, pRerfer);
    }
    
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

    SetHttpStatus();
    
    FinishSession();

    int Ret = 0;
    if(func)
    {
        Ret = func(this, strHeaderRsp, strWriteRsp);
    }
    
    printf("Get end ...\n");
    
    return Ret;
}


int CCarrot::Post(const char* pUrl, const map<string,string>& mapParam, fnc_callback_t func, const char* pRerfer)
{
    //设置POST参数
    string strParam;
    map<string,string>::const_iterator iter = mapParam.begin();
    while(iter != mapParam.end())
    {
        char tmp[10240] = {0};
        snprintf(tmp, sizeof(tmp), "%s=%s", iter->first.c_str(), iter->second.c_str());
        strParam.append(tmp);
        iter++;
        if(iter != mapParam.end())
        {
            strParam.append("&");
        }
        else
        {
            break;
        }
    }

    return Post(pUrl, strParam.c_str(), func, pRerfer);
}


int CCarrot::Post(const char* pUrl, const char* pParam, fnc_callback_t func, const char* pRerfer)
{
    printf("Post begin ...\n");
    
    if(!CreateSession())
    {
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
    SetHttpHeader();

    //设置referer
    if(pRerfer)
    {
        curl_easy_setopt(m_handle, CURLOPT_REFERER, pRerfer);
    }
    
    //设置POST参数
    if(pParam)
    {
        curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, pParam); 
    }
    
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

    SetHttpStatus();
    
    FinishSession();

    int Ret = 0;
    if(func)
    {
        Ret = func(this, strHeaderRsp, strWriteRsp);
    }

    printf("Post end ...\n");
    
    return Ret;
}


int CCarrot::Download2File(const char* pUrl, const char* file_path, const map<string,string>& mapParam)
{
    printf("Download2File begin ...\n");
    
    if(!CreateSession())
    {
        return -1;
    }
    
    FILE* outfile = fopen(file_path, "w");
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
    
    CURLcode CURLRet = CURLE_OK;
    
    string strHeaderRsp;
    string strReadRsp;
    
    //设置url
    curl_easy_setopt(m_handle, CURLOPT_URL, strUrl.c_str());
    
    //设置http头部信息
    SetHttpHeader();
    
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
    
    FinishSession();
    
    printf("Download2File end ...\n");
    
    return 0;
}

void CCarrot::SetHttpHeader()
{
    printf("SetHttpHeader begin ...\n");

    m_pHeaders = curl_slist_append(m_pHeaders, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    m_pHeaders = curl_slist_append(m_pHeaders, "Accept-Language: zh-CN,zh;q=0.8");
    m_pHeaders = curl_slist_append(m_pHeaders, "Accept: */*");
    m_pHeaders = curl_slist_append(m_pHeaders, "Connection: Keep-Alive");  
    m_pHeaders = curl_slist_append(m_pHeaders, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36");
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, m_pHeaders);
    
    printf("SetHttpHeader end ...\n");
}

void CCarrot::SetHttpCookie()
{
    printf("SetHttpCookie begin ...\n");
    
    CURLcode CURLRet = CURLE_OK;

    //会使curl在调用curl_easy_cleanup的时候把cookie保存到指定的文件中
    //m_bKeepAlive为true的话，这里相当于没用，仅激活cookie功能
    CURLRet = curl_easy_setopt(m_handle, CURLOPT_COOKIEJAR, COOKIEPATH);

    map<string,string>::iterator iter = m_mapCookie.begin();
    for(; iter != m_mapCookie.end(); iter++)
    {
        //会把指定的cookie字符串列表加入easy handle维护的cookie列表中
        /*
        #define SEP  "\t"  // Tab separates the fields
        char *my_cookie =
        "example.com"    //Hostname
        SEP "FALSE"      //Include subdomains
        SEP "/"          //Path
        SEP "FALSE"      //Secure
        SEP "0"          //Expiry in epoch time format. 0 == Session 
        SEP "foo"       //Name
        SEP "bar";       //Value
        */
        //printf("\t%s=%s\n", iter->first.c_str(), iter->second.c_str());
        //curl_easy_setopt(m_handle, CURLOPT_COOKIELIST, iter->second.c_str());

        //buff格式:key=value
        //char buff[1024] = {0};
        //snprintf(buff, sizeof(buff), "%s=%s", iter->first.c_str(), iter->second.c_str());
        //curl_easy_setopt(m_handle, CURLOPT_COOKIE, buff);
    }

    
    //会使curl下一次发请求时从指定的文件中读取cookie
    CURLRet = curl_easy_setopt(m_handle, CURLOPT_COOKIEFILE, COOKIEPATH);
 
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
        string str = strip(nc->data);
        if(str[0] == '#')
        {
            nc = nc->next;
            continue;
        }
        
        vector<string> result;
        split(str, result, "\t");
        if(result.size() == 7)
        {
            m_mapCookie[result[5]] = result[6];
            printf("\t%s=%s\n", result[5].c_str(), result[6].c_str());
        }
        else if(result.size() == 6)
        {
            
        }
        else
        {
            printf("cookie len is invalid:\n\t%s\n", nc->data);
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
    mapParam["t"] = "0.5689602857912557";
    
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
        mapParam["t"] = "0.1";
        
        if(Get(GET_ONLINE, mapParam, Callback4VerifyLogin, REFERER_OL) == 0)
        {
            flag = true;
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
    printf("GetScanState end ...\n");
    
    map<string,string> mapParam;
    
    mapParam["webqq_type"] = "10";
    mapParam["remember_uin"] = "1";
    mapParam["login2qq"] = "1";
    mapParam["appid"] = "501004106";
    mapParam["u1"] = "http://w.qq.com/proxy.html?login2qq=1&webqq_type=10";
    mapParam["ptredirect"] = "0";
    mapParam["ptlang"] = "2052";
    mapParam["daid"] = "164";
    mapParam["from_ui"] = "1";
    mapParam["pttype"] = "1";
    mapParam["dumy"] = "";
    mapParam["fp"] = "loginerroralert";
    mapParam["action"] = "0-0-4045";
    mapParam["mibao_css"] = "m_webqq";
    mapParam["t"] = "undefined";
    mapParam["g"] = "1";
    mapParam["js_type"] = "0";
    mapParam["js_ver"] = "10191";
    mapParam["login_sig"] = "";
    mapParam["pt_randsalt"] = "0";
    
    while(Get(SCAN_STATE, mapParam, Callback4GetScanState, SCAN_STATE_REFERER) != 0)
    {
        sleep(2);
    }

    printf("GetScanState end ...\n");
    
    return 0;
}

int CCarrot::FetchCookiePT()
{
    printf("FetchCookiePT begin ...\n");
    map<string, string> mapParam;
    Get(m_strUrl.c_str(), mapParam, Callback4Default, REFERER_PT);
    printf("FetchCookiePT end ...\n");
    return 0;
}

int CCarrot::FetchCookieVF()
{
    printf("FetchCookieVF begin ...\n");
    
    map<string, string> mapParam;
    mapParam["ptwebqq"] = m_mapCookie["ptwebqq"];
    mapParam["clientid"] = "53999199";
    mapParam["psessionid"] = "";
    mapParam["t"] = "0.1";
    
    int Ret = Get(WEBQQ_VERIFY, mapParam, Callback4FetchCookieVF, REFERER_VF);
    
    printf("FetchCookieVF end ...\n");
    
    return Ret;
}

int CCarrot::FetchCookiePN()
{
    printf("FetchCookiePN begin ...\n");

    Json::Value req;
    Json::FastWriter writer;
    req["ptwebqq"] = m_mapCookie["ptwebqq"];
    req["clientid"] = 53999199;
    req["psessionid"] = "";
    req["status"] = "online";

    map<string, string> mapParam;
    mapParam["r"] = UrlEncode(writer.write(req));
    
    int Ret = Post(FETCH_PN, mapParam, Callback4FetchCookiePN, REFERER_PN);
    
    printf("FetchCookiePN end ...\n");
    
    return Ret;
}

int CCarrot::ParserCookieFile()
{
    printf("ParserCookieFile begin ...\n");
    FILE* cookiefile = fopen(COOKIEPATH, "r");
    if(cookiefile == NULL)
    {
        return 0;
    }
    
    while(true)
    {
        char buff[10240] = {0};
        if(fgets(buff, sizeof(buff), cookiefile))
        {
            string str = strip(strip(buff), '\n');
            if(str.empty() or str[0] == '#')
            {
                continue;
            }
            
            vector<string> result;
            split(str, result, "\t");
            if(result.size() == 7)
            {
                m_mapCookie[result[5]] = result[6];
                printf("\t%s=%s\n", result[5].c_str(), result[6].c_str());
            }
            else if(result.size() == 6)
            {

            }
            else
            {
                printf("cookie len is invalid:\n\t%s\n", str.c_str());
                return -1;
            }
        }
        else
        {
            break;
        }
    }

    printf("ParserCookieFile end ...\n");
    
    return 0;
}

int CCarrot::GetHttpStatus()
{
    return m_HttpStatus;
}

int CCarrot::SetHttpStatus()
{
    CURLcode CURLRet = CURLE_OK;
    
    CURLRet = curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &m_HttpStatus);            //获取cookielist信息
    if( CURLRet != CURLE_OK)
    {
       printf("get response code failed\n");
       return -1;
    }

    //printf("HttpStatus=%d\n", HttpStatus);

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
        int Ret = 0;
        Ret = GetQR();
        if(Ret != 0)
        {
            printf("GetQR failed, Ret=%d\n", Ret);
            continue;
        }
        
        Ret = GetScanState();
        if(Ret != 0)
        {
            printf("GetScanState failed, Ret=%d\n", Ret);
            continue;
        }
        
        Ret = FetchCookiePT();
        if(Ret != 0)
        {
            printf("FetchCookiePT failed, Ret=%d\n", Ret);
            continue;
        }
        
        Ret = FetchCookieVF();
        if(Ret != 0)
        {
            printf("FetchCookieVF failed, Ret=%d\n", Ret);
            continue;
        }

        Ret = FetchCookiePN();
        if(Ret != 0)
        {
            printf("FetchCookiePN failed, Ret=%d\n", Ret);
            continue;
        }
    }

    while(true)
    {
        sleep(2);
    }
    
    //判断登录状态
    
    return 0;
}


bool CCarrot::CreateSession()
{
    if(m_pHeaders != NULL)
    {
        curl_slist_free_all(m_pHeaders);
        m_pHeaders = NULL;
    }
    
    if(m_handle == NULL)
    {
        //启用一个会话
        m_handle = curl_easy_init();
        if(m_handle == NULL)
        {
            printf("curl_easy_init failed\n");
            return false;
        }
    }
    else
    {
        if(!m_bKeepAlive)
        {
            //关闭一个会话
            if(m_handle != NULL)
            {
                curl_easy_cleanup(m_handle);
                m_handle = NULL;
            }

            //启用一个会话
            m_handle = curl_easy_init();
            if(m_handle == NULL)
            {
                printf("curl_easy_init failed\n");
                return false;
            }
        }
    }
    

    return true;
}


void CCarrot::FinishSession()
{
    //清理头部结构
    if(m_pHeaders)
    {
        curl_slist_free_all(m_pHeaders);
        m_pHeaders = NULL;
    }
    
    if(!m_bKeepAlive)
    {
        //关闭一个会话
        if(m_handle)
        {
            curl_easy_cleanup(m_handle);
            m_handle = NULL;
        }
    }
}

void CCarrot::Finish()
{
    //全局清理
    curl_global_cleanup();
}



