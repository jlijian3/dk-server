/*
	统计调试成对日志类 by 陈文清 07.7.26 Ver: 2.1

	update 09.6.16 by cwq: 增加按日期换文件名
*/
#include "logbase.h"
#include "util.h"


LogBase::LogBase(void) : _debug_level(1), _is_print(false), _is_flush(false), 
						_flog(0), _fdebug(0),
						_now_day_log(0), _now_day_debug(0),
						_max_log_file_size(1024*1024*1024),
						_max_debug_file_size(1024*1024*1024),
            _log_file_size(0),
            _debug_file_size(0),
						_log_curr_num(0), _debug_curr_num(0)
{}


LogBase::~LogBase(void)
{
	Close();
}


// 初始化创建
// logFilePrefix: 统计日志文件名前缀
// debugFilePrefix: 调试日志文件名前缀
// isPrint: 是否输出屏幕
// isFlush: 是否即时刷新
// debugLevel: 调试信息输出的级别，0=无输出，1=基本输出，2=详细
// maxLogFileSize: 日志文件大小限制，byte
int LogBase::Create(const char *logFilePrefix,
					const char *debugFilePrefix,
					 bool isPrint,
					 bool isFlush,
					 size_t debugLevel,
					 size_t maxLogFileSize,
					 size_t maxDebugFileSize)
{
	_is_flush = isFlush;
	_is_print = isPrint;
	_debug_level = debugLevel;
	_max_log_file_size = maxLogFileSize;
	_max_debug_file_size = maxDebugFileSize;
	_log_prefix = logFilePrefix;
	_debug_prefix = debugFilePrefix;

	_now_day_log = GetNowDay();
	_now_day_debug = GetNowDay();
	string log_path = MakeFileName_Init(_log_prefix, _max_log_file_size, _log_curr_num);
	string debug_path = MakeFileName_Init(_debug_prefix, _max_debug_file_size, _debug_curr_num);
	
	if( log_path.empty() || debug_path.empty() )
		return -1;

	if( !OpenFile(_flog, log_path.c_str())
		|| ( _debug_level && !OpenFile(_fdebug, debug_path.c_str()) ) )
		return -1;

	_log_file_size = Util::GetFileSize(log_path.c_str());
	_debug_file_size = Util::GetFileSize(debug_path.c_str());
	return 0;
}


bool LogBase::OpenFile(FILE*& f, const char* filePathName)
{
	return (f = fopen(filePathName, "a"));
}


// 构造以日期变化的文件名：前缀 + 日期 + 序号 + ".log"
string LogBase::MakeFileName_Init(const string& prefix, size_t maxLogFileSize, int& currNum)
{
	string filename;
	int curr_num = SeekMaxFileNo(prefix, filename);

	if( curr_num == -2 )
		return string();

	if( curr_num == -1 ) {
		currNum = 0;
		return prefix + GetDate() + "_0.log";
	}

	filename = prefix + GetDate() + "_" + Util::ToString(curr_num) + ".log";
	size_t fsize = Util::GetFileSize(filename.c_str());
	currNum = ( fsize < maxLogFileSize ? curr_num : currNum + 1 );
	return prefix + GetDate() + "_" + Util::ToString(currNum) + ".log";
}


string LogBase::MakeFileName_New(const string& prefix, size_t num)
{
	return prefix + GetDate() + "_" + Util::ToString(num) + ".log";
}


int LogBase::SeekMaxFileNo(const string& prefix, string& fileName)
{
	string prefix_dir_path = Util::GetPath(prefix);
	string prefix_name = Util::GetFileName(prefix) + GetDate() + "_";

	DIR* dir = opendir(prefix_dir_path.c_str());
	if( !dir )	return -2;

	int no, maxno = -1;
	string name;
	struct dirent* ptr;
	
	while( (ptr = readdir(dir)) )
	{
		if( ptr->d_type == 8
			&& !strncmp(ptr->d_name, prefix_name.c_str(), prefix_name.length()) )
		{
			name = ptr->d_name + prefix_name.length();
			no = atoi( name.substr(0, name.find_first_of(".")).c_str() );

			if( no > maxno ) {
				maxno = no;
				fileName = ptr->d_name;
			}
		}
	}
	closedir(dir);
	
	return maxno;
}


void LogBase::Close()
{
	if( _flog )  {  fclose(_flog);  _flog = 0;  }
	if( _fdebug )  {  fclose(_fdebug);  _fdebug = 0;  }
}


string LogBase::MakeStr(char* format,...)
{
	char msg[MAX_LOG_LEN];
	size_t fmtlen;
	VASTR(msg, sizeof(msg), fmtlen, format);
	return msg;
}


void LogBase::ChangeLogName(FILE*& f, const string& prefix, int currNum, size_t* fileSize)
{
	if( f ) { fclose(f); f = 0; }
	string log_path = MakeFileName_New(prefix, currNum);

	if( !OpenFile(f, log_path.c_str()) ) {
		cerr<<"can't create new log file: "<<log_path<<", err: "<<Util::Serr(errno)<<endl;
		return;
	}
	*fileSize = 0;
}


int LogBase::SetTimeAndEofCR(char* buf, int bufSize, int remanentSize, int strLen)
{
	if( strLen + 1 == remanentSize )
		*(buf + bufSize - 2) = '\n';

	*(buf + GetTime(buf)) = ' ';
	return strLen + TIME_LEN;
}


// 统计日志，总是即时刷新
void LogBase::LogInfo(int filtType, const string& fileMsg, const string& line)
{
	LOCK lock(_mr);

	if( GetNowDay() != _now_day_log ) {
		_log_curr_num = 0;
		ChangeLogName(_flog, _log_prefix, _log_curr_num, &_log_file_size);
		_now_day_log = GetNowDay();
	}

	if( _log_file_size >= _max_log_file_size ) {
		_log_curr_num++;
		ChangeLogName(_flog, _log_prefix, _log_curr_num, &_log_file_size);
	}

	_log_file_size += line.length() + 1;

	if( _flog )  fprintf(_flog, "%s\n", line.c_str());
	fflush(_flog);
	if( _is_print )  printf("%s\n", line.c_str());
}


void LogBase::DebugInfo(int level, int filtType, const string& fileMsg, char* format,...)
{
	if(	_debug_level < level )
		return;

	char msg[MAX_LOG_LEN];
	size_t fmtlen;
	VASTR(msg + TIME_LEN, sizeof(msg) - TIME_LEN, fmtlen, format)
	Out(msg, SetTimeAndEofCR(msg, sizeof(msg), sizeof(msg) - TIME_LEN, fmtlen));
}


void LogBase::DebugInfo(int level, const string& line)
{
	DebugInfo(level, line.length(), line.c_str());
}


void LogBase::DebugInfo(int level, int lineLen, const char* line)
{
	if( _debug_level < level )
		return;

	char	msg[MAX_LOG_LEN];
	int fmtlen;

	msg[GetTime(msg)] = ' ';

	if( lineLen < sizeof(msg) - TIME_LEN ) {
		memcpy(msg + TIME_LEN, line, lineLen + 1);
		fmtlen = lineLen + TIME_LEN;
	}
	else {
		memcpy(msg + TIME_LEN, line, sizeof(msg) - TIME_LEN - 2);
		msg[sizeof(msg)-2] = '\n';
		msg[sizeof(msg)-1] = '\0';
		fmtlen = sizeof(msg) - 1;
	}

	Out(msg, fmtlen);
}


void LogBase::DebugInfo(int level, char* format,...)
{
	if( _debug_level < level )
		return;

	char	msg[MAX_LOG_LEN];
	size_t fmtlen;
	VASTR(msg + TIME_LEN, sizeof(msg) - TIME_LEN, fmtlen, format)
	Out(msg, SetTimeAndEofCR(msg, sizeof(msg), sizeof(msg) - TIME_LEN, fmtlen));
}


void LogBase::Out(const char *msg, int msgLen)
{
	LOCK lock(_md);
	
	if( GetNowDay() != _now_day_debug ) {
		_debug_curr_num = 0;
		ChangeLogName(_fdebug, _debug_prefix, _debug_curr_num, &_debug_file_size);
		_now_day_debug = GetNowDay();
	}

	if( _debug_file_size >= _max_debug_file_size ) {
		_debug_curr_num++;
		ChangeLogName(_fdebug, _debug_prefix, _debug_curr_num, &_debug_file_size);
	}

	_debug_file_size += msgLen;

	if( _fdebug )  fprintf(_fdebug, "%s", msg);
	if( _is_flush )  fflush(_fdebug);
	if( _is_print )  printf("%s", msg);
}


size_t LogBase::GetNowDay()
{
	time_t now;
	time(&now);
	struct tm res;
	return localtime_r(&now, &res)->tm_mday;
}


int LogBase::GetTime(char *str)
{
	time_t		nowtime;
	struct tm	res;

	time(&nowtime);
	localtime_r(&nowtime, &res);
  /*
	return sprintf(str, "%04d%02d%02d-%02d:%02d:%02d", 
				res.tm_year + 1900,
				res.tm_mon+1,
				res.tm_mday,
				res.tm_hour,
				res.tm_min,
				res.tm_sec);
  */
  return sprintf(str, "%02d:%02d:%02d", 
				res.tm_hour,
				res.tm_min,
				res.tm_sec);
}


string LogBase::GetDate()
{
	time_t		nowtime;
	struct tm	res;
	char		str[20];

	time(&nowtime);
	localtime_r(&nowtime, &res);
	sprintf(str, "%04d%02d%02d",
		res.tm_year + 1900,
		res.tm_mon+1,
		res.tm_mday);
	return str;
}

