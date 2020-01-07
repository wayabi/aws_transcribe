#include <memory.h>
#include "HttpClient.h"

const char * HttpClient::FORMAT_CERT = "PEM";
const char * HttpClient::FORMAT_KEY = "PEM";

using namespace std;

HttpClient::HttpClient(const std::string& url)
{
	url_ = url;
	header_http_ = "";
	string_post_ = "";

	flag_communication_error_ = false;
	string_communication_error_ = "";

	timeout_ = 60;//default
	multipart_post_ = NULL;

	buf_response_callback_ = NULL;
	size_response_callback_ = 0;
	headerlist_ = NULL;
	response_ = "";
	code_response_ = -1;
	curl_return_code_ = -1;
}

HttpClient::~HttpClient()
{
	if(buf_response_callback_) free(buf_response_callback_);
}

string HttpClient::getStringError()
{
	return string_communication_error_;
}

void HttpClient::setURL(const std::string& url)
{
	url_ = url;
}

void HttpClient::setPostString(const std::string& string_post)
{
	string_post_ = string_post;
}

void HttpClient::setSSLCert(const std::string& path_cert, const std::string& path_private_key, const std::string& pass_private_key)
{
	path_cert_ = path_cert;
	path_private_key_ = path_private_key;
	pass_private_key_ = pass_private_key;
}

void HttpClient::execute(void)
{
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();

	if(curl) {
		setCurlEasy(curl);
		//Log::log(TAG, "http before perform");
		res = curl_easy_perform(curl);
		//Log::log(TAG, "http after perform");
		if(res != CURLE_OK){
			string_communication_error_ = curl_easy_strerror(res);
			//Log::log(TAG, "CURL_NG");
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}else{
			if(size_response_callback_ > 0) response_ = string(buf_response_callback_);
		}
	
		curl_return_code_ = (int)res;
		long code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		code_response_ = (int)code;
		makeCookie(curl);
		curl_easy_cleanup(curl);
		if(headerlist_) curl_slist_free_all(headerlist_);
	}
}

void HttpClient::setCurlEasy(CURL* curl)
{
	curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_);

	//header
	headerlist_ = NULL;
	headerlist_ = curl_slist_append(headerlist_, "Expect:");
	headerlist_ = curl_slist_append(headerlist_, header_http_.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist_);

	//cookie
	if(cookie_set_.length() > 0){
		int length = cookie_set_.length();
		const char *cc = cookie_set_.c_str();
		int start = 0;
		for(int i=0;i<length;++i){
			if(*(cc+i) == '\n'){
				string s(cookie_set_, start, i-start-1);
				start = i+1;
				//printf("%s\n", s.c_str());
				curl_easy_setopt(curl, CURLOPT_COOKIELIST, s.c_str());
			}
		}
	}else{
		curl_easy_setopt(curl, CURLOPT_COOKIELIST, "");
	}

	//ssl
	if(path_cert_.size() > 0){
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, HttpClient::FORMAT_CERT);
		curl_easy_setopt(curl, CURLOPT_SSLCERT, path_cert_.c_str());
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, pass_private_key_.c_str());
		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, HttpClient::FORMAT_KEY);
		curl_easy_setopt(curl, CURLOPT_SSLKEY, path_private_key_.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	if(multipart_post_){
		//multipart
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, multipart_post_);
	}else if(string_post_.length() > 0){
		//string
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string_post_.c_str());
		//post field size -1だとバグる。内部のstrlen()がおかしい？
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, string_post_.length());
	}

	//response callback
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::writeMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);

	//バグ対処　longjump causes
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	//debug
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debugCallback);
}

size_t HttpClient::writeMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	HttpClient* thisa = (HttpClient*)userp;

  thisa->buf_response_callback_ = (char*)realloc(thisa->buf_response_callback_, thisa->size_response_callback_ + realsize + 1);
  if(thisa->buf_response_callback_ == NULL) {
    //out of memory!
	  //Log::log(TAG, "not enough memory (realloc returned NULL)");
    return 0;
  }

  memcpy(&(thisa->buf_response_callback_[thisa->size_response_callback_]), contents, realsize);
  thisa->size_response_callback_ += realsize;
  thisa->buf_response_callback_[thisa->size_response_callback_] = 0;

  return realsize;
}

int HttpClient::debugCallback(CURL* curl_handle, curl_infotype infotype, char* chr, size_t size, void* unused)
{
    size_t i, written;
    char* c = chr;

    (void)sizeof(unused);

    if (!curl_handle || !chr)
    {
        return -1;
    }

    //fprintf(stderr, "\n\n");

    bool flag_show = false;
    switch (infotype)
    {
    /*
        case CURLINFO_TEXT:
            fprintf(stderr, "CURL-debug: Informational data\n");
            fprintf(stderr, "------------------------------\n");
            break;

        case CURLINFO_HEADER_IN:
            fprintf(stderr, "CURL-debug: Incoming header\n");
            fprintf(stderr, "---------------------------\n");
            break;
	*/
        case CURLINFO_HEADER_OUT:
            fprintf(stderr, "CURL-debug: Outgoing header\n");
            fprintf(stderr, "---------------------------\n");
            	flag_show = true;
            break;
/*
        case CURLINFO_DATA_IN:
            fprintf(stderr, "CURL-debug: Incoming data\n");
            fprintf(stderr, "-------------------------\n");
            break;
*/
        case CURLINFO_DATA_OUT:
            fprintf(stderr, "CURL-debug: Outgoing data\n");
            fprintf(stderr, "-------------------------\n");
            	flag_show = true;
            break;
        default:
            //fprintf(stderr, "CURL-debug: Unknown\n");
            //fprintf(stderr, "-------------------\n");
            break;
    }
    if(!flag_show) return 0;

    written = 0;
    for (i = 0; i < size; i++)
    {
        written++;
        if (isprint(*c))
        {
            fprintf(stderr,"%c", *c);
        }
        else if (*c == '\n')
        {
            fprintf(stderr, "\n");
            written = 0;
        }
        else
        {
            fprintf(stderr, ".");
        }

        if (written == 70)
        {
            fprintf(stderr, "\n");
        }
        c++;
    }
    return 0;
}

void HttpClient::makeCookie(CURL* curl)
{
	cookie_got_ = "";
	struct curl_slist* cookies;
	curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if(cookies){
		struct curl_slist *nc = cookies;
		while (nc) {
			cookie_got_.append(nc->data);
			cookie_got_.append("\n");
			nc = nc->next;
		}
	}

	curl_slist_free_all(cookies);
}
