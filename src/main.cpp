#include "aws_util.h"
#include <iostream>
#include "HttpClient.h"
#include <time.h>
#include <sstream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
	if(argc < 2){
		printf("%s <path_mp3>\n", *(argv+0));
		return 1;
	}
	const char* path_mp3 = *(argv+1);

	long t = time(NULL);
	stringstream ss;
	ss << t;
	string request_id = ss.str();
	aws_util::start_transcribe_job(path_mp3, "prot0", request_id.c_str());
	cout << "request_id:" << request_id << endl;
	
	for(int i=0;i<100;++i){
		usleep(10 * 1000 * 1000);
		cout << i << endl;
		string url = aws_util::get_transcribe_transcription_url(request_id);
		cout << "url:" << url << endl;
		if(url.size() > 0){
			string transcription = aws_util::get_transcribe_transcription(url);
			cout << transcription << endl;
			break;
		}
	}
	return 0;
}
