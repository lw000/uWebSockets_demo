#include <string.h>
#include <stdlib.h>
#include "utils.h"

#if defined(WIN32) || defined(_WIN32)

std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
	std::wstring ret;
	if (!strUtf8.empty())
	{
		int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (nNum)
		{
			WCHAR* wideCharString = new WCHAR[nNum + 1];
			wideCharString[0] = 0;

			nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

			ret = wideCharString;
			delete[] wideCharString;
		}
		else
		{
			printf("Wrong convert to WideChar code:0x%x", GetLastError());
		}
	}
	return ret;
}

std::string StringWideCharToUtf8(const std::wstring& strWideChar)
{
	std::string ret;
	if (!strWideChar.empty())
	{
		int nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* utf8String = new char[nNum + 1];
			utf8String[0] = 0;

			nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, utf8String, nNum + 1, nullptr, FALSE);

			ret = utf8String;
			delete[] utf8String;
		}
		else
		{
			printf("Wrong convert to Utf8 code:0x%x", GetLastError());
		}
	}
	return ret;
}

std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
	std::string ret;
	if (!strUtf8.empty())
	{
		std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
		int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* ansiString = new char[nNum + 1];
			ansiString[0] = 0;

			nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

			ret = ansiString;
			delete[] ansiString;
		}
		else
		{
			printf("Wrong convert to Ansi code:0x%x", GetLastError());
		}
	}

	return ret;
}

// UTF-8 to GB2312
std::string utf8_to_gbk(const char* inbuf)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, inbuf, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, inbuf, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string r(str);
	if (str) delete[] str;
	return r;
}

// GB2312 to UTF-8
std::string gbk_to_utf8(const char* inbuf)
{
	int len = MultiByteToWideChar(CP_ACP, 0, inbuf, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, inbuf, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string r(str);
	if (str) delete[] str;
	return r;
}

static double getfreq(void)
{
	LARGE_INTEGER freq;
	if (!QueryPerformanceFrequency(&freq)) return 0.0;
	return (double)freq.QuadPart;
}

static double f = -1.0;

double gettime(void)
{
	LARGE_INTEGER t;
	if (f < 0.0) f = getfreq();
	if (f == 0.0) return (double)GetTickCount() / 1000.;
	else
	{
		QueryPerformanceCounter(&t);
		return (double)t.QuadPart / f;
	}
}
#else
static int code_convert(char *from_charset, char *to_charset, char *inbuf,
		int inlen, char *outbuf, int outlen) {
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, (size_t*) &inlen, pout, (size_t*) &outlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}

int utf8_to_gbk(char *inbuf, char *outbuf, int outlen) {
	return code_convert((char *) "utf-8", (char *) "gb2312", inbuf, strlen(inbuf),
			outbuf, outlen);
}

int gbk_to_utf8(char *inbuf, char *outbuf, size_t outlen) {
	return code_convert((char *) "gb2312", (char *) "utf-8", inbuf, strlen(inbuf),
			outbuf, outlen);
}

double gettime(void) {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) < 0)
		return 0.0;
	else
		return (double) tv.tv_sec + ((double) tv.tv_usec / 1000000.);
}
#endif

std::string lw_make_uuidstring() {
#if defined(_WIN32) || defined(WIN32)
	char buffer[64] = {0};
	GUID guid;
	if (CoCreateGuid(&guid))
	{
		fprintf(stderr, "create guid error\n");
	}

	_snprintf(buffer, sizeof(buffer),
			"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
	return std::string(buffer);
#else
	uuid_t uu;
	uuid_generate(uu);
	char buf[64];
	int n = 0;
	for (int i = 0; i < 16; i++) {
		n += sprintf(&buf[n], "%02X-", uu[i]);
	}
	std::string str_uuid(buf, strlen(buf) - 1);
	return str_uuid;
#endif // _WIN32
}

unsigned int lw_make_software_version(unsigned char major, unsigned char minor,
		unsigned short build) {
	unsigned int version = major;
	version = (version << 8);
	version = (version) | minor;
	version = (version << 16);
	version = (version) | build;
	return version;
}

void lw_software_version(unsigned int version, unsigned char &major,
		unsigned char &minor, unsigned short &build) {
	build = (version & 0x0000FFFF);

	version = (version >> 16);
	minor = (version & 0x000000FF);

	version = (version >> 8);
	major = (version & 0x000000FF);
}

unsigned char lw_major_version(unsigned int version) {
	version = (version >> 24);
	return (version & 0x000000FF);
}

unsigned char lw_minor_version(unsigned int version) {
	version = (version >> 16);
	return (version & 0x000000FF);
}

unsigned short lw_build_version(unsigned int version) {
	return (version & 0x0000FFFF);
}

unsigned long lw_hash_code(const char* c) {
	unsigned long ret = 0;
	long n;
	unsigned long v;
	int r;

	if ((c == NULL) || (*c == '\0'))
		return (ret);
	/*
	 unsigned char b[16];
	 MD5(c,strlen(c),b);
	 return(b[0]|(b[1]<<8)|(b[2]<<16)|(b[3]<<24));
	 */

	n = 0x100;
	while (*c) {
		v = n | (*c);
		n += 0x100;
		r = (int) ((v >> 2) ^ v) & 0x0f;
		ret = (ret << r) | (ret >> (32 - r));
		ret &= 0xFFFFFFFFL;
		ret ^= v * v;
		c++;
	}
	return ((ret >> 16) ^ ret);
}

char * lw_strtok_r(char *s, const char *delim, char **state) {
	char *cp, *start;
	start = cp = s ? s : *state;
	if (!cp)
		return NULL;
	while (*cp && !strchr(delim, *cp))
		++cp;
	if (!*cp) {
		if (cp == start)
			return NULL;
		*state = NULL;
		return start;
	} else {
		*cp++ = '\0';
		*state = cp;
		return start;
	}
}

KVQueryUrlArgsValue::KVQueryUrlArgsValue() {
	_kv.clear();
}

KVQueryUrlArgsValue::~KVQueryUrlArgsValue() {
	this->reset();
}

int KVQueryUrlArgsValue::parse(const char* data) {
	if (data == NULL) {
		return -1;
	}

	if (strlen(data) == 0) {
		return -1;
	}

	this->reset();

	char *p = const_cast<char*>(data);
	char *p0 = NULL;
	char *p1 = NULL;
	p0 = lw_strtok_r(p, "&", &p1);
	if (p0 == NULL)
		return -1;
	while (p0 != NULL) {
		char *q = NULL;
		char *q1 = NULL;

		std::string k;
		std::string v;

		q = lw_strtok_r(const_cast<char*>(p0), "=", &q1);
		if (q == NULL)
			return -1;
		k = q;
		q = lw_strtok_r(NULL, "=", &q1);
		if (q == NULL)
			return -1;
		v = q;

		KV_T *_pkv = (KV_T*) malloc(sizeof(KV_T));
		_pkv->k = (char*) ::malloc(k.size() + 1);
		_pkv->v = (char*) ::malloc(v.size() + 1);
		strcpy(_pkv->k, k.c_str());
		strcpy(_pkv->v, v.c_str());

		_kv.push_back(_pkv);

		p0 = lw_strtok_r(NULL, "&", &p1);
	}
	return 0;
}

char* KVQueryUrlArgsValue::find(const char* key) {
	std::list<KV_T*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter) {
		KV_T *_pkv = *iter;
		if (strcmp(_pkv->k, key) == 0) {
			return _pkv->v;
			break;
		}
	}
	return NULL;
}

void KVQueryUrlArgsValue::each(std::function<void(KV_T*)> func) {
	std::list<KV_T*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter) {
		KV_T *_pkv = *iter;
		func(_pkv);
	}
}

void KVQueryUrlArgsValue::reset() {
	std::list<KV_T*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter) {
		KV_T *_pkv = *iter;
		free(_pkv->k);
		free(_pkv->v);
		free(_pkv);
	}
	_kv.clear();
}

std::vector<std::string> lw_split(const char* str, const char* pattern) {
	char *p = NULL;
	char *p1 = NULL;
	p = lw_strtok_r(const_cast<char*>(str), pattern, &p1);
	std::vector<std::string> s;
	while (p != NULL) {
		s.push_back(p);
		p = lw_strtok_r(NULL, pattern, &p1);
	}
	return s;
}

std::unordered_map<std::string, std::string> lw_split_url_pragma_data(
		const char* str) {
	char *p = const_cast<char*>(str);
	char *p0 = NULL;
	char *p1 = NULL;
	p0 = lw_strtok_r(p, "&", &p1);
	if (p0 == NULL) {
		return std::unordered_map<std::string, std::string>();
	}

	std::unordered_map<std::string, std::string> s;
	while (p0 != NULL) {
		{
			char *q = NULL;
			char *q1 = NULL;
			std::string k;
			std::string v;

			q = lw_strtok_r(const_cast<char*>(p0), "=", &q1);
			k = q;
			q = lw_strtok_r(NULL, "=", &q1);
			v = q;
			s[k] = v;
		}
		p0 = lw_strtok_r(NULL, "&", &p1);
	}
	return s;
}

void lw_trim(char* src, char* dest) {
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*ps == ' ') {
		++ps;
	}

	while (*pe == ' ') {
		--pe;
	}

	char* p = dest;
	while (ps <= pe) {
		*p++ = *ps++;
	}

	*p = '\0';
}

void lw_trim_l(char* src, char* dest) {
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*ps == ' ') {
		++ps;
	}

	char* p = dest;
	while (ps <= pe) {
		*p++ = *ps++;
	}

	*p = '\0';
}

void lw_trim_r(char* src, char* dest) {
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*pe == ' ') {
		--pe;
	}

	char* p = dest;
	while (ps <= pe) {
		*p++ = *ps++;
	}

	*p = '\0';
}
