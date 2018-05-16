#ifndef __LW_UTIL_H__
#define __LW_UTIL_H__

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <uuid/uuid.h>
#include <sys/time.h>
#include <iconv.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>

#ifdef _WIN32
//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif

#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif
#elif __ANDROID__
// android
#elif __linux__
// linux
#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif

#if defined(WIN32) || defined(_WIN32)
#define lw_sleep(mseconds) SleepEx(mseconds, 1);
#else
#define lw_sleep(mseconds) usleep(mseconds*1000);
#endif

struct KV_T
{
	char *k;
	char *v;
};

class KVQueryUrlArgsValue
{
public:
	KVQueryUrlArgsValue();
	~KVQueryUrlArgsValue();

public:
	int parse(const char* data);
	char* find(const char* key);
	void each(std::function<void(KV_T*)> func);
	void reset();

private:
	std::list<KV_T*> _kv;
};

unsigned int lw_make_software_version(unsigned char major, unsigned char minor, unsigned short build);
void lw_software_version(unsigned int version, unsigned char &major, unsigned char &minor, unsigned short &build);
unsigned char lw_major_version(unsigned int version);
unsigned char lw_minor_version(unsigned int version);
unsigned short lw_build_version(unsigned int version);

unsigned long lw_hash_code(const char* c);

char * lw_strtok_r(char *s, const char *delim, char **state);

std::vector<std::string> lw_split(const char* str, const char* pattern);

std::string lw_make_uuidstring();

std::unordered_map<std::string, std::string> lw_split_url_pragma_data(const char* str);

void lw_trim(char* src, char* dest);
void lw_trim_l(char* src, char* dest);
void lw_trim_r(char* src, char* dest);

#define lw_min(a,b) ((a)<(b)?(a):(b))

#define lw_max(a,b) ((a)>(b)?(a):(b))

extern double gettime(void);

#if defined(WIN32) || defined(_WIN32)

//UTF-8 to GB2312
std::string utf8_to_gbk(const char* inbuf);
//GB2312 to UTF-8
std::string gbk_to_utf8(const char* inbuf);

std::wstring StringUtf8ToWideChar(const std::string& strUtf8);
std::string StringWideCharToUtf8(const std::wstring& strWideChar);
std::string UTF8StringToMultiByte(const std::string& strUtf8);

#ifndef __MINGW32__
#include <stdio.h>
#define snprintf(str, n, format, ...)  \
			_snprintf_s(str, n, _TRUNCATE, format, __VA_ARGS__)
#endif
#define strcasecmp stricmp
#define strncasecmp strnicmp

#define U2G(v)	utf8_to_gbk(v)
#define G2U(v)	gbk_to_utf8(v)

#else

#define U2G(v)
#define G2U(v)

#endif



#endif // !__LW_UTIL_H__
