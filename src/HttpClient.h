#ifndef __U_HTTP_CLIENT__
#define __U_HTTP_CLIENT__

#include <curl/curl.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

class HttpClient {
public:
	HttpClient(const std::string& url);
	~HttpClient();
	void setURL(const std::string& url);
	void setHeader(const std::string& header){header_http_ = header;}
	void setPostString(const std::string& string_post);
	void setSSLCert(const std::string& path_cert, const std::string& path_private_key, const std::string& pass_private_key);
	size_t getSizeBinary(void){return size_response_callback_;}
	const char* getPBinary(void){return buf_response_callback_;}
	void execute(void);
	std::string getCookie(void){return cookie_got_;}
	void setCookie(const std::string& cookie){cookie_set_ = cookie;}
	std::string getStringError();
	void setTimeout(int sec){timeout_ = sec;}
	void setMultipartPost(struct curl_httppost* post){multipart_post_ = post;}
	std::string getStringResponse(void){return response_;}
	int getCodeResponse(void){return code_response_;}
	int getCurlReturnCode(void){return curl_return_code_;}

private:
	static size_t writeMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
	static int debugCallback(CURL* curl_handle, curl_infotype infotype, char* chr, size_t size, void* unused);
	//証明書のファイルフォーマット
	static const char* FORMAT_CERT;
	//秘密鍵のファイルフォーマット
	static const char* FORMAT_KEY;
	void makeCookie(CURL* curl);
	void setCurlEasy(CURL* curl);

	//通信対象URL
	std::string url_;
	//送信ヘッダ
	std::string header_http_;
	//HttpClient use either of p_post or string_post or multipart_post
	std::string string_post_;
	struct curl_httppost* multipart_post_;
	//証明書
	std::string path_cert_;
	std::string path_private_key_;
	std::string pass_private_key_;
	//送信時に使うクッキー
	std::string cookie_set_;
	//送信後にサーバから取得したクッキー
	std::string cookie_got_;
	//通信エラーが起きたかどうか
	bool flag_communication_error_;
	//通信エラー内容の文字列
	std::string string_communication_error_;
	//タイムアウト
	int timeout_;

	std::string response_;
	int code_response_;
	int curl_return_code_;

	char* buf_response_callback_;
	size_t size_response_callback_;
	struct curl_slist* headerlist_;

};

#endif
