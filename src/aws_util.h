#ifndef __U_AWS_UTIL__
#define __U_AWS_UTIL__

#include <string>
#include <vector>

class aws_util {
private:
	static std::string exec(const char* cmd);
	static std::string get_file_name(const char* path);
	static std::string command_start_transcribe_job(const char* path_json);
	static std::string json_start_transcribe_job(const char* request_id, const char* s3_dir, const char* s3_file);
	static bool check_start_transcribe_job(const std::string& json);
	static std::string get_transcribe_transcription_url(const std::string& request_id);

public:
	static bool put_s3(const char* path_local, const char* s3_dir, const char* s3_file);
	static bool rm_s3(const char* s3_dir, const char* s3_file);
	static bool start_transcribe_job(const char* path_mp3, const char* s3_dir, const char* request_id);
	static std::vector<std::string> list_transcribe_job_completed();
	static std::string get_transcribe_transcription(const std::string& request_id);
};

#endif
