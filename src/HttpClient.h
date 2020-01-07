#ifndef __U_HTTP_CLIENT__
#define __U_HTTP_CLIENT__

#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <stdlib.h>

using namespace std;

class HttpClient {
public:
	HttpClient(string url_);
	~HttpClient();
	void setURL(string url_);
	void setHeader(string header_){this->header_http = header_;}
	void setPostString(string string_post_);
	void setSSLCert(string path_cert_, string path_private_key_, string pass_private_key_);
	size_t getSizeBinary(void){return this->size_response_callback;}
	const char *getPBinary(void){return this->buf_response_callback;}
	void execute(void);
	string getCookie(void){return this->cookie_got;}
	void setCookie(string cookie){this->cookie_set = cookie;}
	string getStringError();
	void setTimeout(int sec){this->timeout = sec;}
	void setMultipartPost(struct curl_httppost *post){this->multipart_post = post;}
	string getStringResponse(void){return this->response;}
	int getCodeResponse(void){return this->code_response;}
	int getCurlReturnCode(void){return this->curl_return_code;}

private:
	static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
	static int debugCallback (CURL* curl_handle, curl_infotype infotype, char* chr, size_t size, void* unused);
	//証明書のファイルフォーマット
	static const char * FORMAT_CERT;
	//秘密鍵のファイルフォーマット
	static const char * FORMAT_KEY;
	void makeCookie(CURL *curl);
	void setCurlEasy(CURL* curl);

	//通信対象URL
	string url;
	//送信ヘッダ
	string header_http;
	//HttpClient use either of p_post or string_post or multipart_post
	string string_post;
	struct curl_httppost *multipart_post;
	//証明書
	string path_cert;
	string path_private_key;
	string pass_private_key;
	//送信時に使うクッキー
	string cookie_set;
	//送信後にサーバから取得したクッキー
	string cookie_got;
	//通信エラーが起きたかどうか
	bool flag_communication_error;
	//通信エラー内容の文字列
	string string_communication_error;
	//タイムアウト
	int timeout;

	string response;
	int code_response;
	int curl_return_code;

	char* buf_response_callback;
	size_t size_response_callback;
	struct curl_slist* headerlist;

};

#endif
