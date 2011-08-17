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
    // cellsize means size of the cell by uint32_t
	m_cellsize = cellsize;
	m_filesize = filesize;
	MySuicideAssert((0 == (m_filesize % (m_cellsize*sizeof(uint32_t))))
			&& m_filesize > 0 && m_cellsize > 0);
	m_cellcount = m_filesize / (m_cellsize*sizeof(uint32_t));

	char filename[MAX_FILENAME_LENGTH];
	snprintf(filename, sizeof(filename), "%s/%s", dir, file);

	mode_t amode = (0 == access(filename, F_OK)) ? O_RDWR : O_RDWR|O_CREAT ;
	m_fd = open(filename, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	MySuicideAssert(m_fd != -1);
	MySuicideAssert (-1 != lseek(m_fd, filesize-1, SEEK_SET));
	MySuicideAssert ( 1 == write(m_fd, "", 1));
	MySuicideAssert (MAP_FAILED != (puint = (uint32_t*)mmap(0, m_filesize,
					PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0)));
}

bitlist :: ~bitlist()
{
	msync(puint, m_filesize, MS_SYNC);
	close(m_fd);
}
