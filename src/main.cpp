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
	string transcription = aws_util::get_transcribe_transcription("0");
	cout << transcription << endl;

	return 0;
}
