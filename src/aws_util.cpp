#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string.h>
#include <array>
#include <sstream>
#include <fstream>

#include "picojson.h"
#include "HttpClient.h"
#include "aws_util.h"

using namespace std;


std::string replace_all(std::string &replacedStr, std::string from, std::string to) {
    unsigned int pos = replacedStr.find(from);
    int toLen = to.length();
 
    if (from.empty()) {
        return replacedStr;
    }
 
    while ((pos = replacedStr.find(from, pos)) != std::string::npos) {
        replacedStr.replace(pos, from.length(), to);
        pos += toLen;
    }
    return replacedStr;
}

std::string aws_util::exec(const char* cmd) {
	cout << "exec [" << cmd << "]" << endl;
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	string replaced = replace_all(result, "\r", "\n");
	cout << "result [" << replaced << "]" << endl;
	return result;
}

std::string aws_util::get_file_name(const char* path)
{
	int length = strlen(path);
	if(length == 0) return string("");
	for(int i=length-1;i>=0;--i){
		if(*(path+i) == '/'){
			if(i != length-1){
				return string(path, i+1, length-i-1);
			}
		}
	}
	return string(path);
}

std::string aws_util::command_start_transcribe_job(const char* path_json)
{
	stringstream ss;
	ss << "aws transcribe start-transcription-job"
	<< " --region " << "ap-northeast-1"
	<< " --cli-input-json file://" << path_json;
	return ss.str();
}

std::string aws_util::json_start_transcribe_job(const char* request_id, const char* s3_dir, const char* s3_file)
{
	stringstream ss;
	ss << "{\n\"TranscriptionJobName\": \"" << request_id << "\",\n"
	<< "\"LanguageCode\": \"ja-JP\",\n" 
  << "\"MediaFormat\": \"mp3\",\n"
  << "\"Media\": {\n"
  << "\"MediaFileUri\": \"https://s3.ap-northeast-1.amazonaws.com/" << s3_dir << "/" << s3_file << "\"\n"
  << "}\n}";
	return ss.str();
}

bool aws_util::check_start_transcribe_job(const std::string& json)
{
	return (json.find("IN_PROGRESS") != std::string::npos);
}

bool aws_util::put_s3(const char* path_local, const char* s3_dir, const char* s3_file)
{
	stringstream ss;
	ss << "aws s3 cp " << path_local << " s3://" << s3_dir << "/" << s3_file;
	string ret = aws_util::exec(ss.str().c_str());
	return (ret.find("upload:") != std::string::npos);
}

bool aws_util::rm_s3(const char* s3_dir, const char* s3_file)
{
	stringstream ss;
	ss << "aws s3 rm"  " s3://" << s3_dir << "/" << s3_file;
	string ret = aws_util::exec(ss.str().c_str());
	return (ret.find("delete:") != std::string::npos);
}

bool aws_util::start_transcribe_job(const char* path_mp3, const char* s3_dir, const char* request_id)
{
	string name = get_file_name(path_mp3);
	bool ret0 = put_s3(path_mp3, s3_dir, name.c_str());
	if(!ret0){
		return false;
	}
	string json = json_start_transcribe_job(request_id, s3_dir, name.c_str());

	stringstream ss;
	ss << "tmp/json/" << request_id;
	string path_json = ss.str();
	ofstream f_json(path_json.c_str());
	f_json << json;
	f_json.close();

	string command = command_start_transcribe_job(path_json.c_str());
	string ret1 = aws_util::exec(command.c_str());
	bool ret2 = check_start_transcribe_job(ret1.c_str());
	if(!ret2){
		rm_s3(s3_dir, name.c_str());
	}
	return ret2;
}

std::vector<std::string> aws_util::list_transcribe_job_completed()
{
	vector<string> ret;
	stringstream ss;
	ss << "aws transcribe list-transcription-jobs"
	<< " --region ap-northeast-1"
	<< " --status COMPLETED";
	string ret0 = aws_util::exec(ss.str().c_str());
	return ret;
}

std::string aws_util::get_transcribe_transcription_url(const std::string& request_id)
{
	stringstream ss;
	ss << "aws transcribe get-transcription-job "
	<< "--transcription-job-name " << request_id;
	string ret = aws_util::exec(ss.str().c_str());
	
	picojson::value val;
	picojson::parse(val, ret);

	string status = val.get<picojson::object>()
	["TranscriptionJob"].get<picojson::object>()
	["TranscriptionJobStatus"].get<std::string>();
	if(status != "COMPLETED"){
		return "";
	}

	string url = val.get<picojson::object>()
	["TranscriptionJob"].get<picojson::object>()
	["Transcript"].get<picojson::object>()
	["TranscriptFileUri"].get<std::string>();
	return url;
}

std::string aws_util::get_transcribe_transcription(const std::string& s3_url)
{
	HttpClient http_client(s3_url);
	http_client.execute();
	string json = http_client.getStringResponse();

	picojson::value val;
	picojson::parse(val, json);
	picojson::array aa = val.get<picojson::object>()
	["results"].get<picojson::object>()
	["transcripts"].get<picojson::array>();
	for(picojson::array::iterator ite = aa.begin();ite != aa.end();++ite){
		picojson::object obj = ite->get<picojson::object>();
		return obj["transcript"].get<std::string>();
	}
	return "";
}

bool aws_util::delete_transcribe_job(const std::string& request_id)
{
	stringstream ss;
	ss << "aws transcribe delete-transcription-job "
	<< "--transcription-job-name " << request_id;
	string ret = aws_util::exec(ss.str().c_str());
	
	return true;
}

