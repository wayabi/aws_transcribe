#include "aws_util.h"
#include <iostream>
#include "HttpClient.h"

using namespace std;

int main(int argc, char** argv){
/*
	bool ret = aws_util::put_s3("tmp/a", "prot0", "a");
	aws_util::rm_s3("prot0", "a");
	cout << ret << endl;
*/

	//aws_util::start_transcribe_job("~/a.mp3", "prot0", "0");
/*
	auto ss = aws_util::list_transcribe_job_completed();
	for(auto ite = ss.begin();ite != ss.end();++ite){
		cout << *ite << endl;
	}
*/
	string url = aws_util::get_transcribe_transcription_url("0");
	cout << url << endl;
	HttpClient http_client(url);
	http_client.execute();
	string res = http_client.getStringResponse();
	cout << "response:" << res << endl;


	return 0;
}
