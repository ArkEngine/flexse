#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "mylog.h"

const char* const mylog::strLogName = "./log/default.log";
const char* mylog::LevelTag[] = { "DEBUG", "ROUTN", "ALARM", "FATAL", };

mylog :: mylog(const uint32_t level, const uint32_t size, const char* logname)
{
	pthread_mutex_init(&m_lock, NULL);
	m_level = (level > (uint32_t)FATAL) ? (uint32_t)ROUTN : level;
	m_file_size = size > 1073741824 ? 1073741824 : size;
	const char* mylogname = (logname[0] != 0 ) ? logname : strLogName;
	snprintf(m_path, sizeof(m_path), "%s", mylogname);
	m_log_fd = -1;
	LogFileCheckOpen();
	fprintf(stderr, "mylog Level[%u] OL[%u] Path[%s] Size[%u] fd[%d]\n",
			m_level, level, m_path, m_file_size, m_log_fd);
}

void mylog :: WriteNByte(const int fd, const char* buff, const uint32_t size)
{
	int32_t  left   = size;
	uint32_t offset = size;
	while (left > 0)
	{
		int len = write(fd, buff+offset-left, left);
		if (len == -1 || len == 0)
		{
			return;
		}
		left -= len;
	}
}
void mylog :: WriteLog(const uint32_t mylevel, const char* file,
		const uint32_t line, const char* func, const char *format, ...)
{
	pthread_mutex_lock(&m_lock);
	LogFileCheckOpen();
	va_list args;
	va_start(args, format);
	char buff[LogContentMaxLen];
	uint32_t pos = 0;
	if (mylevel < m_level) {
		pthread_mutex_unlock(&m_lock);
		return;
	}
	uint32_t  level = (mylevel > (uint32_t)FATAL) ? (uint32_t)FATAL : mylevel;
	pos =  snprintf(&buff[pos], LogContentMaxLen-pos, "%s ", LevelTag[level]);
	pos += snprintf(&buff[pos], LogContentMaxLen-pos, "%s ", GetTimeNow());
	pos += snprintf(&buff[pos], LogContentMaxLen-pos, "%lu ", pthread_self());
	pos += snprintf(&buff[pos], LogContentMaxLen-pos, "file[%s:%u] ", file, line);
	pos += snprintf(&buff[pos], LogContentMaxLen-pos, "func[%s] ", func);
	vsnprintf(&buff[pos], LogContentMaxLen-pos-2, format, args);
	va_end(args);
	pos = strlen(buff);
	buff[pos] = '\n';
	buff[pos+1] = '\0';
	WriteNByte(m_log_fd, buff, pos+1);
	pthread_mutex_unlock(&m_lock);
}

void mylog :: LogFileCheckOpen()
{
	char  tmppath[LogNameMaxLen];
	char  curpath[LogNameMaxLen];
	if (NULL == getcwd(curpath, sizeof(curpath))) {
		fprintf(stderr, "Get Current Dir Fail.\n");
		return;
	}
	char* p = NULL;
	// 不存在就创建日志文件, 兼顾初始化和运行时的状态
	// 第一步，创建文件夹
	if (0 != access(m_path,F_OK)) {
		snprintf(tmppath, sizeof(tmppath), "%s", m_path);
		p = strrchr(tmppath, '/');
		if (p) {
			*p = '\0';
			Mkdirs(tmppath);
		}
		if (m_log_fd >= 0){
			close(m_log_fd);
			m_log_fd = -1;
		}
	}
	chdir(curpath);
	// 第二步，创建文件
	if (m_log_fd < 0) {
		m_log_fd = open(m_path, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
		if (m_log_fd < 0) {
			fprintf(stderr, "FILE[%s:%u] create logfile[%s] fail. msg[%m]\n",
					__FILE__, __LINE__, m_path);
			while(0 != raise(SIGKILL)){}
		}
	}
}

void mylog :: LogFileSizeCheck()
{
	struct stat st;
	fstat(m_log_fd, &st);
	if (st.st_size + LogContentMaxLen > m_file_size)
	{
		ftruncate(m_log_fd, 0);
	}
}

const char* mylog :: GetTimeNow()
{
	time_t now;
	struct tm mytm;

	m_timebuff[0] = 0;
	time(&now);
	localtime_r(&now, &mytm);
	mytm.tm_year += 1900;
	sprintf(m_timebuff, "%02d-%02d %02d:%02d:%02d",
			mytm.tm_mon+1, mytm.tm_mday, mytm.tm_hour, mytm.tm_min, mytm.tm_sec);

	return  m_timebuff;
}

void mylog :: Mkdirs(const char* dir)
{
	char tmp[1024];
	char *p = NULL;
	if (strlen(dir) == 0 || dir == NULL) {
		return;
	}
	memset(tmp, '\0', sizeof(tmp));
	strncpy(tmp, dir, strlen(dir));
	if (tmp[0] == '/') {
		p = strchr(tmp + 1, '/');
	} else {
		p = strchr(tmp, '/');
	}

	if (p) {
		*p = '\0';
		mkdir(tmp, 0777);
		chdir(tmp);
	} else {
		mkdir(tmp, 0777);
		chdir(tmp);
		return;
	}
	Mkdirs(p + 1);
}
