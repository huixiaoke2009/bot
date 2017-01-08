
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

    //{"result":[{"client_type":1,"status":"online","uin":1786202149}],"retcode":0}
    
    int friend_num = root["result"].size();
    for(int i = 0; i < friend_num; i++)
    {
        OnlineFriend o;
        o.uin = root["result"][i]["uin"].asDouble();
        o.client_type = root["result"][i]["client_type"].asInt();
        snprintf(o.status, sizeof(o.status), "%s", root["result"][i]["status"].asString().c_str());

        p->m_mapOnlineFriend[o.uin] = o;
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
    p->m_mapCookie["uin"] = root["result"]["uin"].asString();
    
    return 0;
}


inline int Callback4GetQQNumByUin(CCarrot* p, const string& strHeader, const string& strResult)
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

    map<uint64_t, OnlineFriend>::iterator iter = p->m_mapOnlineFriend.begin();
    for(; iter != p->m_mapOnlineFriend.end(); iter++)
    {
        if(iter->first == uint64_t(root["result"]["uin"].asDouble()))
        {
            iter->second.qqnum = root["result"]["account"].asDouble();
        }
    }
    
    return 0;
}


inline int Callback4FetchMessage(CCarrot* p, const string& strHeader, const string& strResult)
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

    //文字
    //{"result":[{"poll_type":"message","value":{"content":[["font",{"color":"000000","name":"微软雅黑","size":10,"style":[0,0,0]}],"123"],"from_uin":1786202149,"msg_id":26684,"msg_type":0,"time":1483846258,"to_uin":229845213}}],"retcode":0}
    //自带表情
    //{"result":[{"poll_type":"message","value":{"content":[["font",{"color":"000000","name":"微软雅黑","size":10,"style":[0,0,0]}],["face",98]],"from_uin":1786202149,"msg_id":26707,"msg_type":0,"time":1483848513,"to_uin":229845213}}],"retcode":0}
    //群消息
    //{"result":[{"poll_type":"group_message","value":{"content":[["font",{"color":"000000","name":"微软雅黑","size":10,"style":[0,0,0]}],"123"],"from_uin":858811018,"group_code":858811018,"msg_id":3373,"msg_type":0,"send_uin":1786202149,"time":1483848903,"to_uin":229845213}}],"retcode":0}
    //只考虑文字的情况，没时间调表情
    string strMsgType = root["result"][0]["poll_type"].asString();
    Json::Value content = root["result"][0]["value"]["content"];
    int size = content.size();
    for(int i = 1; i < size; i++)
    {
        Json::Value tmp = content[i];
        if(tmp.type() == Json::arrayValue)
        {
            p->m_message.message.debris[i-1].type = 1;
            snprintf(p->m_message.message.debris[i-1].buff, sizeof(p->m_message.message.debris[i-1].buff), "%s", tmp[0].asString().c_str());
            p->m_message.message.debris[i-1].code = tmp[1].asInt();
        }
        else
        {
            p->m_message.message.debris[i-1].type = 0;
            snprintf(p->m_message.message.debris[i-1].buff, sizeof(p->m_message.message.debris[i-1].buff), "%s", tmp.asString().c_str());
        }
    }
    
    snprintf(p->m_message.msg_type, sizeof(p->m_message.msg_type), "%s", strMsgType.c_str());
    p->m_message.send_uin = root["result"][0]["value"]["from_uin"].asDouble();
    p->m_message.to_qqnum = root["result"][0]["value"]["to_uin"].asDouble();
    p->m_message.send_time = root["result"][0]["value"]["time"].asDouble();
    if(strMsgType == "group_message")
    {
        p->m_message.group_code = root["result"][0]["value"]["group_code"].asDouble();
    }
    else if(strMsgType == "discu_message")
    {
        p->m_message.did = root["result"][0]["value"]["did"].asDouble();
    }
    
    return 0;
}





CCarrot::CCarrot()
{
    m_handle = NULL;
    m_pHeaders = NULL;
    m_HttpStatus = 0;
    m_bKeepAlive = false;
}

CCarrot::~CCarrot()
{
    
    
}

int CCarrot::Init()
{
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
            map<uint64_t, OnlineFriend>::iterator iter = m_mapOnlineFriend.begin();
            for(; iter != m_mapOnlineFriend.end(); iter++)
            {
                GetQQNumByUin(iter->first, iter->second.qqnum);
            }
            
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
    mapParam["r"] = writer.write(req);
    
    int Ret = Post(FETCH_PN, mapParam, Callback4FetchCookiePN, REFERER_PN);
    
    printf("FetchCookiePN end ...\n");
    
    return Ret;
}

int CCarrot::ParserSelfCookieFile()
{
    FILE* cookiefile = fopen(SELFCOOKIEPATH, "r");
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
            if(result.size() == 2)
            {
                m_mapCookie[result[0]] = result[1];
                printf("\t%s=%s\n", result[0].c_str(), result[1].c_str());
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

    fclose(cookiefile);
   
    return 0;
}

int CCarrot::SaveSelfCookieFile()
{
    FILE* cookiefile = fopen(SELFCOOKIEPATH, "w");
    if(cookiefile == NULL)
    {
        return 0;
    }

    map<string,string>::iterator iter = m_mapCookie.begin();
    for(; iter != m_mapCookie.end(); iter++)
    {
        char buff[10240] = {0};
        snprintf(buff, sizeof(buff), "%s\t%s\n", iter->first.c_str(), iter->second.c_str());
        fwrite(buff, strlen(buff), 1, cookiefile);
    }

    fclose(cookiefile);

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

int CCarrot::GetUserFriend()
{
    Json::Value req;
    Json::FastWriter writer;
    req["psessionid"] = m_mapCookie["psessionid"];
    req["clientid"] = 53999199;

    map<string, string> mapParam;
    mapParam["r"] = writer.write(req);

    
    Post(GET_USER_FRIEND, mapParam, Callback4Default, REFERER_GET_USER_FRIEND);

    return 0;
}

int CCarrot::GetQQNumByUin(uint64_t uin, uint64_t& qqnum)
{
    printf("GetQQNumByUin begin ...\n");

    char buff[32] = {0};
    snprintf(buff, sizeof(buff), "%ld", uin);
    map<string, string> mapParam;
    mapParam["tuin"] = buff;
    mapParam["type"] = "1";
    mapParam["vfwebqq"] = m_mapCookie["vfwebqq"];
    mapParam["t"] = "0.1";

    int Ret = Get(GET_QQNUM_BY_UIN, mapParam, Callback4GetQQNumByUin, REFERER_GET_QQNUM_BY_UIN);

    printf("GetQQNumByUin end ...\n");
    
    return Ret;
}

int CCarrot::FetchMessage()
{
    printf("FetchMessage begin ...\n");

    Json::Value req;
    Json::FastWriter writer;
    req["ptwebqq"] = m_mapCookie["ptwebqq"];
    req["clientid"] = 53999199;
    req["psessionid"] = m_mapCookie["psessionid"];
    req["key"] = "";

    map<string, string> mapParam;
    mapParam["r"] = writer.write(req);

    memset(&m_message, 0x0, sizeof(m_message));
    int Ret = Post(FETCH_MSG, mapParam, Callback4FetchMessage, REFERER_FETCH_MSG);
    
    printf("FetchMessage end ...\n");
    
    return Ret;
}

//下面四个函数是发送好友消息
int CCarrot::SendFriendMsgByQQnum(uint64_t qqnum, const char* message)
{
    map<uint64_t, OnlineFriend>::iterator iter = m_mapOnlineFriend.begin();
    for(; iter != m_mapOnlineFriend.end(); iter++)
    {
        if(iter->second.qqnum == qqnum)
        {
            return SendFriendMsgByUin(iter->first, message);
        }
    }

    return -1;
}

int CCarrot::SendFriendMsgByUin(uint64_t uin, const char* message)
{
    return SendMsg(uin, message, 0);
}


int CCarrot::SendFriendMsgUnitByQQnum(uint64_t qqnum, const MessageUnit& o)
{
    map<uint64_t, OnlineFriend>::iterator iter = m_mapOnlineFriend.begin();
    for(; iter != m_mapOnlineFriend.end(); iter++)
    {
        if(iter->second.qqnum == qqnum)
        {
            return SendFriendMsgUnitByUin(iter->first, o);
        }
    }

    return -1;
}

int CCarrot::SendFriendMsgUnitByUin(uint64_t uin, const MessageUnit& o)
{
    return SendMsgByMsgUnit(uin, o, 0);
}

//下面四个函数是发送群消息
int CCarrot::SendGroupMsgByGroupnum(uint64_t groupnum, const char* message)
{
    return 0;
}

int CCarrot::SendGroupMsgByUin(uint64_t uin, const char* message)
{
    return SendMsg(uin, message, 1);
}

int CCarrot::SendGroupMsgUnitByGroupnum(uint64_t qqnum, const MessageUnit& o)
{
    return 0;
}


int CCarrot::SendGroupMsgUnitByUin(uint64_t uin, const MessageUnit& o)
{
    return SendMsgByMsgUnit(uin, o, 1);
}


//下面四个函数是发送讨论组消息
int CCarrot::SendDiscuMsgByDiscunum(uint64_t did, const char* message)
{
    return 0;
}

int CCarrot::SendDiscuMsgByUin(uint64_t uin, const char* message)
{
    return SendMsg(uin, message, 2);
}

int CCarrot::SendDiscuMsgUnitByGroupnum(uint64_t qqnum, const MessageUnit& o)
{
    return 0;
}

int CCarrot::SendDiscuMsgUnitByUin(uint64_t uin, const MessageUnit& o)
{
    return SendMsgByMsgUnit(uin, o, 2);
}


int CCarrot::SendMsg(uint64_t uin, const char* message, int type)
{
    printf("SendMsg begin ...\n");
            
    Json::Value req;
    Json::FastWriter writer;
    
    Json::Value content;
    Json::Value font;
    Json::Value fontmap;
    Json::Value style;
    content.append(message);
    font.append("font");
    fontmap["name"] = "宋体";
    fontmap["size"] = 10;
    style.append(0);
    style.append(0);
    style.append(0);
    fontmap["style"] = style;
    fontmap["color"] = "000000";
    font.append(fontmap);
    content.append(font);

    if(type == 0)
    {
        req["to"] = uin;
    }
    else if(type == 1)
    {
        req["group_uin"] = uin;
    }
    else if(type == 2)
    {
        req["did"] = uin;
    }
    else
    {
        return -1;
    }
    
    req["clientid"] = 53999199;
    req["face"] = 522;
    req["msg_id"] = time(NULL);
    req["psessionid"] = m_mapCookie["psessionid"];
    req["content"] = writer.write(content);

    map<string, string> mapParam;
    mapParam["r"] = writer.write(req);
    printf("%s\n", writer.write(req).c_str());

    int Ret = 0;
    if(type == 0)
    {
        Ret = Post(SEND_MSG, mapParam, Callback4Default, REFERER_SEND_MSG);
    }
    else if(type == 1)
    {
        Ret = Post(SEND_GROUP_MSG, mapParam, Callback4Default, REFERER_SEND_GROUP_MSG);
    }
    else
    {
        Ret = Post(SEND_DISCU_MSG, mapParam, Callback4Default, REFERER_SEND_DISCU_MSG);
    }
    
    printf("SendMsg end ...\n");
    
    return Ret;
}

int CCarrot::SendMsgByMsgUnit(uint64_t uin, const MessageUnit& o, int type)
{
    printf("SendMsgByMsgUnit begin ...\n");
            
    Json::Value req;
    Json::FastWriter writer;
    
    Json::Value content;
    Json::Value font;
    Json::Value fontmap;
    Json::Value style;
    
    for(unsigned int i = 0; i < sizeof(o.debris)/sizeof(MessageDebris); i++)
    {
        if(o.debris[i].buff[0] == '\0')
        {
            break;
        }

        Json::Value msg_debris;
        if(o.debris[i].type == 0)
        {
            content.append(o.debris[i].buff);
        }
        else
        {
            Json::Value tmp;
            tmp.append(o.debris[i].buff);
            tmp.append(o.debris[i].code);
            content.append(tmp);
        }
    }
    
    font.append("font");
    fontmap["name"] = "宋体";
    fontmap["size"] = 10;
    style.append(0);
    style.append(0);
    style.append(0);
    fontmap["style"] = style;
    fontmap["color"] = "000000";
    font.append(fontmap);
    content.append(font);

    if(type == 0)
    {
        req["to"] = uin;
    }
    else if(type == 1)
    {
        req["group_uin"] = uin;
    }
    else if(type == 2)
    {
        req["did"] = uin;
    }
    else
    {
        return -1;
    }
    
    req["clientid"] = 53999199;
    req["face"] = 522;
    req["msg_id"] = time(NULL);
    req["psessionid"] = m_mapCookie["psessionid"];
    req["content"] = writer.write(content);

    map<string, string> mapParam;
    mapParam["r"] = writer.write(req);
    printf("%s\n", writer.write(req).c_str());

    int Ret = 0;
    if(type == 0)
    {
        Ret = Post(SEND_MSG, mapParam, Callback4Default, REFERER_SEND_MSG);
    }
    else if(type == 1)
    {
        Ret = Post(SEND_GROUP_MSG, mapParam, Callback4Default, REFERER_SEND_GROUP_MSG);
    }
    else
    {
        Ret = Post(SEND_DISCU_MSG, mapParam, Callback4Default, REFERER_SEND_DISCU_MSG);
    }
    
    printf("SendMsgByMsgUnit end ...\n");
    
    return Ret;
}


int CCarrot::Run()
{
    //加载本地cookie文件
    if(ParserSelfCookieFile() != 0)
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

    SaveSelfCookieFile();
    
    while(true)
    {
        try
        {
            if(FetchMessage() == 0)
            {
                
                string strMsgType = m_message.msg_type;
                if(strMsgType == "message")
                {
                    SendFriendMsgUnitByUin(m_message.send_uin, m_message.message);
                }
                else if(strMsgType == "group_message")
                {
                    SendGroupMsgUnitByUin(m_message.group_code, m_message.message);
                }
                else if(strMsgType == "discu_message")
                {
                    SendDiscuMsgUnitByUin(m_message.did, m_message.message);
                }
                else
                {
                    continue;
                }
            }
        }
        catch(...)
        {
            printf("except\n");
        }
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



