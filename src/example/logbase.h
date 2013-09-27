/*
	统计调试成对日志类 by 陈文清 07.7.26 Ver: 2.1

	update 09.6.16 by cwq: 增加按日期换文件名
*/
#ifndef _CWQ_LOG_BASE_H
#define _CWQ_LOG_BASE_H
#include "type.h"
#include "mutex.h"

class LogBase
{
	enum {
		//TIME_LEN = 18,		// 此种时间格式"20100527-14:27:01 "的串长，懒得动态去算
    TIME_LEN = 9,
	};

	int 	_debug_level;
	bool	_is_print, _is_flush;
	
	FILE	*_flog, *_fdebug;
	string	_log_prefix, _debug_prefix;
	size_t	_now_day_log, _now_day_debug;
	size_t	_max_log_file_size, _max_debug_file_size;
	size_t	_log_file_size, _debug_file_size;
	int		_log_curr_num, _debug_curr_num;
	Mutex	_mr, _md;


	void Close();
	int SeekMaxFileNo(const string& prefix, string& fileName);
	string MakeFileName_Init(const string& prefix, size_t maxLogFileSize, int& maxNo);
	string MakeFileName_New(const string& prefix, size_t num);
	void ChangeLogName(FILE*& f, const string& prefix, int currNum, size_t* fileSize);
	bool OpenFile(FILE*& f, const char* filePathName);
	int SetTimeAndEofCR(char* buf, int bufSize, int remanentSize, int strLen);
	void Out(const char *msg, int msgLen);
	int GetTime(char *str);
	string GetDate();
	size_t GetNowDay();	


public:
	LogBase(void);
	virtual ~LogBase(void);

	enum { MAX_LOG_LEN = 16384, };

	int Create(const char *logFilePrefix,
			const char *debugFilePrefix,
			bool isPrint,
			bool isFlush,
			size_t debugLevel,
			size_t maxLogFileSize,
			size_t maxDebugFileSize);
	
	void LogInfo(int filtType, const string& fileMsg, const string& line);
	string MakeStr(char* format,...);
	void DebugInfo(int level, const string& line);
	void DebugInfo(int level, int lineLen, const char* line);
	void DebugInfo(int level, char* format,...);
	void DebugInfo(int level, int filtType, const string& fileMsg, char* format,...);

  FILE *GetDebugFile() {
    return _fdebug; 
  }

public:
	void SetDebugLevel(size_t level) { _debug_level = level; }
  size_t GetDebugLevel() { return _debug_level; }

};


#define VASTR(buf, bufSize, fmtLen, format)	\
{ \
	va_list	va; \
	va_start(va, format); \
	fmtLen = vsnprintf(buf, bufSize, format, va); \
	va_end(va); \
	if( fmtLen >= bufSize ) { \
		*(buf + bufSize - 1) = '\0'; \
		fmtLen = bufSize - 1; \
	} \
}

#endif

