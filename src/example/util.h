

/*
	边角料 by 陈文清 2007
  modify by lijian2 2011-09-01
*/


#ifndef __PS_UTIL_INCLUDE__
#define __PS_UTIL_INCLUDE__

#include "type.h"
#include "log.h"

class Util
{
public:

	// (线程安全)取错误串
	static string Serr(int errnum)
	{
		char _err[256] = { 0 };
		char* pe = strerror_r(errnum, _err, sizeof(_err));
		return *_err ? _err : pe;
	}

	// 求beg-end的时间差，返回秒为单位的浮点数
	static float ExecTime(const struct timeval& beg, const struct timeval& end)
	{
		return ((float)(end.tv_sec - 1 - beg.tv_sec)) + ((float)(end.tv_usec + 1000000.0 - beg.tv_usec))/1000000.0;
	}

	// 求beg-end的时间差，返回秒为单位的浮点字串
	static string ExecTimeStr(const struct timeval& beg, const struct timeval& end)
	{
		return ToString(ExecTime(beg, end));
	}
	
	// 取日期
	static string GetShortDate()
	{
		time_t		nowtime;
		struct tm	res;
		char st[20];

		time(&nowtime);
		localtime_r(&nowtime, &res);
		sprintf(st, "%04d-%02d-%02d",
			res.tm_year+1900,
			res.tm_mon+1,
			res.tm_mday);
		return st;
	}

	// 取日期串
	static string GetDate()
	{
		time_t		nowtime;
		struct tm	res;
		char st[20];

		time(&nowtime);
		localtime_r(&nowtime, &res);
		sprintf(st, "%04d-%02d-%02d %02d:%02d:%02d",
			res.tm_year+1900,
			res.tm_mon+1,
			res.tm_mday,
			res.tm_hour,
			res.tm_min,
			res.tm_sec);
		return st;
	}

	static int GetFileSize(const string& filePath)
	{
		return GetFileSize(filePath.c_str());
	}

	//文件大小，原本stat返回-1表示出错，但调用此函数都不想多判断返回是否-1，故出错当文件为0
	static int GetFileSize(const char* filePath)
	{
		struct stat buf;
		return stat(filePath, &buf) ? 0 : buf.st_size;
	}
	
	//文件是否存在
	static bool FileExisted(const string& filePath)
	{
		return FileExisted(filePath.c_str());
	}

	static bool FileExisted(const char* filePath)
	{
		return (access(filePath, F_OK) == 0);
	}

	// 从文件路径名取文件名，最后的'/'字符替换成0，返回文件指针
	static char* GetFileName(const char *filePath)
	{
		char *p;
		char *p1 = (char *)filePath;

		while((p=strchr(p1, '/')) != NULL)	p1 = ++p;
		return p1;
	}
	
	// 从文件路径名检索文件名，复制到fileName中，超长截断
	static char* GetFileName(const char *filePath, char *fileName, int fileNameSize)
	{
		char *p;
		const char *p1 = filePath;
	
		while((p=strchr(p1, '/')) != NULL)	p1 = ++p;
		strncpy(fileName, p1, fileNameSize-1);
		*(fileName + fileNameSize - 1) = 0;
		return fileName;
	}
	
	// 从文件路径名取路径，最后'/'的下一字符替换成0，返回路径指针
	static char* GetPath(char *filePath)
	{
		char *p;
		char *p1 = (char *)filePath;

		while((p=strchr(p1, '/')) != NULL)	p1 = ++p;
		*p1 = 0;
		return filePath;
	}

	// 返回文件路径名的路径，含最后一个路径符'/'
	static string GetPath(const string& filePathName)
	{
		return filePathName.substr(0, filePathName.find_last_of("/") + 1);
	}

	// 返回文件路径名的文件名
	static string GetFileName(const string& filePathName)
	{
		return filePathName.substr(filePathName.find_last_of("/") + 1);
	}
	
	// 返回文件名的后缀
	static string GetFileNameSuffix(const string& filePathName)
	{
		return filePathName.substr(filePathName.find_last_of(".") + 1);
	}

	// 清除字串头尾的空格
	static string& StringTrim(string& s)
	{
		if( s.empty() )	return s;
		s.erase(0, s.find_first_not_of(" \t\r\n"));
		s.erase(s.find_last_not_of(" \t\r\n") + 1);
		return s;
	}
	
	//	清除字串头尾的非字符，返回新字串指针
	static char* StrTrim(char *str)
	{
		char *p, *pt;
	
		p = str;
		while(*p && isspace(*p))	p++;
		pt = p;
	
		p += strlen(pt) - 1;
		while(isspace(*p))	p--;
		*(++p) = '\0';
	
		return pt;
	}

	//	清除字串尾的非字符，返回新字串指针
	static char* StrTrimRight(char *str)
	{
		size_t len = strlen(str);
		char *ps = str + len - 1;
		while( len-- && (*ps == '\r' || *ps == '\n' || *ps == '\t' || *ps == ' ') )
			ps--;

		*(++ps) = '\0';
		return str;
	}

  static void StrTrimRight(string &src)
	{
		size_t len = src.size();
		const char *ps = src.c_str() + len - 1;
		while( len-- && (*ps == '\r' || *ps == '\n' || *ps == '\t' || *ps == ' ') )
			ps--;

	  if (len <= 0)
      src.clear();
    else
      src = src.substr(0, len+1);
	}

	//	打印以空格分开的字串
	static void ShowBin(const char *buf, int len)
	{
		while( len-- ) cout<<*buf++<<" ";
		cout<<endl;
	}

	static long long toInt64(const string& s)
	{
#ifdef _WIN32
		return _atoi64(s.c_str());
#else
		return atoll(s.c_str());
#endif
	}

	static int toInt(const string& s)
	{
		return atoi(s.c_str());
	}

	static int toInt32(const string& s)
	{
		return toInt32(s.c_str());
	}

	static int toInt32(const char* c)
	{
		return atoi(c);
	}

	static string ToString(short val)
	{
		char buf[8];
		sprintf(buf, "%d", (int)val);
		return buf;
	}

	static string ToString(unsigned short val)
	{
		char buf[8];
		sprintf(buf, "%u", (unsigned int)val);
		return buf;
	}

	static string ToString(int val)
	{
		char buf[16];
		sprintf(buf, "%d", val);
		return buf;
	}

	static string ToString(unsigned int val)
	{
		char buf[16];
		sprintf(buf, "%u", val);
		return buf;
	}

	static string ToString(long val)
	{
		char buf[32];
		sprintf(buf, "%ld", val);
		return buf;
	}

	static string ToString(unsigned long val)
	{
		char buf[32];
		sprintf(buf, "%lu", val);
		return buf;
	}

	static string ToString(long long val)
	{
		char buf[32];
#ifdef _MSC_VER
		sprintf(buf, "%I64d", val);
#else
		sprintf(buf, "%lld", val);
#endif
		return buf;
	}

	static string ToString(unsigned long long val)
	{
		char buf[32];
#ifdef _MSC_VER
		sprintf(buf, "%I64u", val);
#else
		sprintf(buf, "%llu", val);
#endif
		return buf;
	}

	static string ToString(float val)
	{
		char buf[16];
		sprintf(buf, "%f", val);
		return buf;
	}

  static string ToString(double val)
	{
		char buf[32];
		sprintf(buf, "%f", val);
		return buf;
	}

  static string ToHexString(int val)
  {
	  char buf[16];
	  sprintf(buf, "%x", val);
	  return buf;
  }

  static char* H2A(const unsigned char* src, size_t srcLen, char* dst)
  {
    char *pd = dst;
    while( srcLen-- )
    {
       sprintf(pd, "%02x", *src++);
       pd += 2;
    }
    *pd = 0;
    return dst;
  }
  
   
  /**解析协议 将a=1&b=2&c=3转化为key->value值对*/
    static void Parse2List(vector<int>& dataList, const char* source, const char* split_pair){
        const char* data_left = source;     
        char* pair_split = NULL;
        do{
			int left_size = strlen(data_left);
			if(left_size<=0){
				break;
			}
            pair_split  = strchr(data_left, *split_pair);
            int len_pair_split = 0;        
            if(pair_split){
                len_pair_split = strlen(pair_split);
            }
            int keyvalue_len = strlen(data_left) - len_pair_split;
			if(keyvalue_len<=0){
				break;
			}
            char* keyvalue = new char[keyvalue_len + 1];
            memcpy(keyvalue, data_left, keyvalue_len);
            keyvalue[keyvalue_len] = '\0';
            dataList.push_back(toInt(keyvalue));
            delete[] keyvalue;
            if(pair_split){
                data_left = pair_split+1;
            }

        }while(pair_split && data_left);
    }
  
  /**解析协议 将a=1&b=2&c=3转化为key->value值对*/
    static void Parse2List(vector<string>& dataList, const char* source, const char* split_pair){
        const char* data_left = source;     
        char* pair_split = NULL;
        do{
			int left_size = strlen(data_left);
			if(left_size<=0){
				break;
			}
            pair_split  = strchr(data_left, *split_pair);
            int len_pair_split = 0;        
            if(pair_split){
                len_pair_split = strlen(pair_split);
            }
            int keyvalue_len = strlen(data_left) - len_pair_split;
			if(keyvalue_len<=0){
				break;
			}
            char* keyvalue = new char[keyvalue_len + 1];
            memcpy(keyvalue, data_left, keyvalue_len);
            keyvalue[keyvalue_len] = '\0';
            dataList.push_back(keyvalue);
            delete[] keyvalue;
            if(pair_split){
                data_left = pair_split+1;
            }

        }while(pair_split && data_left);
    }
	
	static void showHexData(const string& msg){
		string hexData = "";
		const char* msgChar = msg.c_str();
		int msgSize = msg.size();
		for(int i=0; i<msgSize; i++){
			char buf[16];
			unsigned char mchar = msgChar[i];
			sprintf(buf, "%02x", mchar);
			hexData.append(buf);
		}
		dlog4("msg:size:%d hexData:%s\n", hexData.size(), hexData.c_str());
	}
	
	static void IntArrayToString(vector<int>& source_list, const char* split_sign, string & target_str_list){
		if(source_list.size() <= 0){
			target_str_list = "";
		}
        for(size_t i=0; i<source_list.size();i++){
            target_str_list.append(ToString(source_list[i])).append(split_sign);
        }
	}
		
			// 取日期串
	static int GetDistSecondsNowToDate(){
		time_t		nowtime;
		struct tm	res;
		time(&nowtime);
		localtime_r(&nowtime, &res);
		int second_count = (23 - res.tm_hour) * 60 * 60 + (59 - res.tm_min) * 60 + (60 - res.tm_sec); 
		return second_count; 
	}


};


char *urlEncode(const char * asSrcUrl, int aiLen, int * aiNewLen);
void vperror(const char *fmt, ...);
void GetOpt(int argc, char** argv,
            bool& isPrint,
            bool& isFlush,
            int& debugLevel,
            std::string& setupFilePath,
            std::string& signame,
            const char *srv_name,
            const char* version);
bool PIDSaveToFile(const char* pidFilePath);
bool GetPidFromFile(const string& pidfile, int &pid);

#endif /*  __PS_UTIL_INCLUDE__ */
