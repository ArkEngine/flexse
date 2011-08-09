#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mylog.h"
#include "bitlist.h"
#include "MyException.h"

bitlist :: bitlist (const char* dir, const char* file,
		const uint32_t cellsize, const uint32_t filesize)
{
	m_cellsize = cellsize;
	m_filesize = filesize;
	MyThrowAssert((0 == (m_filesize % m_cellsize))
			&& (0 == (m_cellsize % sizeof(uint32_t)))
			&& m_filesize > 0);
	m_cellcount = m_filesize / m_cellsize;

	char filename[MAX_FILENAME_LENGTH];
	snprintf(filename, sizeof(filename), "%s/%s", dir, file);

	mode_t amode = (0 == access(filename, F_OK)) ? O_RDWR : O_RDWR|O_CREAT ;
	m_fd = open(filename, amode);
	MyThrowAssert(m_fd != -1);
	MyThrowAssert (-1 != lseek(m_fd, filesize-1, SEEK_SET));
	MyThrowAssert ( 1 == write(m_fd, "", 1));
	MyThrowAssert (MAP_FAILED != (puint = (uint32_t*)mmap(0, m_filesize,
					PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0)));
}

bitlist :: ~bitlist()
{
	msync(puint, m_filesize, MS_SYNC);
	close(m_fd);
}
