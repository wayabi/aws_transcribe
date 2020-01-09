#include "aws_util.h"
#include <iostream>
#include "HttpClient.h"
#include <time.h>
#include <sstream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
	if(argc < 3){
		printf("%s <path_mp3> <dir_s3>\n", *(argv+0));
		return 1;
	}
	const char* path_mp3 = *(argv+1);
	const char* s3_dir = *(argv+2);
	string file_name_mp3 = aws_util::get_file_name(path_mp3);

	long t = time(NULL);
	stringstream ss;
	ss << t;
	string request_id = ss.str();
	aws_util::start_transcribe_job(path_mp3, s3_dir, request_id.c_str(), "/tmp");
	cout << "request_id:" << request_id << endl;
	
	for(int i=0;i<30;++i){
		usleep(10 * 1000 * 1000);
		cout << i << endl;
		string url = aws_util::get_transcribe_transcription_url(request_id);
		cout << "url:" << url << endl;
		if(url.size() > 0){
			string transcription = aws_util::get_transcribe_transcription(url);
			cerr << transcription << endl;
			break;
		}
	}
	aws_util::delete_transcribe_job(request_id);
	aws_util::rm_s3(s3_dir, file_name_mp3.c_str());
	return 0;
}
