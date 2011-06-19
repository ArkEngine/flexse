#include <unistd.h>
#include "diskv.h"
#include "assert.h"

const char* const diskv :: FASTDI_DATA_FILE_FMT   = "%s/%s.dat.%d";

diskv::diskv(const char* mydi_path,const char* myidx_path,const char* mymodule,bool readonly,bool writesync)
    m_fileblock(mydi_path, mymodule, sizeof(diskv_idx_t))
{
	snprintf( m_di_path,  sizeof(m_di_path),  "%s", mydi_path );
	snprintf( m_idx_path, sizeof(m_idx_path), "%s", myidx_path);
	snprintf( m_module,   sizeof(m_module),   "%s", mymodule);
	m_iswritesync = writesync;
	m_isreadonly  = readonly;
	m_append_fd = -1;
	for( uint32_t i=0; i<FASTDI_FD_MAXNUM; i++ )
	{
		m_rw_fd[i] = -1;
	}

	m_done              = false;
	m_pdatabuf_idx      = NULL;
	m_pdatabuf_di       = NULL;
	m_pdatabuf_idx_size = 0;
	m_pdatabuf_di_size  = 0;
	m_idx_pfile         = NULL;
	m_di_pfile          = NULL;
	m_idx_curfno        = 0;
	m_di_curfno         = 0;
	m_iterator_id       = 0;
	memset (&m_curidx, 0, sizeof(diskv_idx_t));

	m_fnum = 1;
	m_data_num = 0;
	m_wdi_flen = 0;

	char filename[FASTDI_PATH_MAXSIZE];
	for( uint32_t i=0; i<FASTDI_FD_MAXNUM; i++ )
	{
		snprintf( filename, FASTDI_PATH_MAXSIZE, FASTDI_DATA_FILE_FMT, m_di_path, m_module, i );
		if( isfileexist( filename ) )
		{
			m_fnum = i+1;
			uint32_t filesize = getfilesize( filename );
			m_wdi_flen  = filesize;
		}
	}

	if (m_append_fd >= 0)
	{
		close (m_append_fd);
		m_append_fd = -1;
	}
	if(!m_isreadonly && (m_append_fd=openfile(m_fnum-1, false ))<=0 )
	{
		ALARM("opendi:%d failed. msg[%m]", m_fnum-1 );
		while(0 != raise(SIGKILL)){}
	}
	if( m_isreadonly )
	{
		m_append_fd = -1;
	}
	for( uint32_t i=0; i<m_fnum; i++ )
	{
		if (m_rw_fd[i] >= 0)
		{
			close (m_rw_fd[i]);
			m_rw_fd[i] = -1;
		}
		if( (m_rw_fd[i] = openfile(i, m_isreadonly ))<=0 )
		{
			ALARM("opendi:%d failed. msg[%m]", i);
			while(0 != raise(SIGKILL)){}
		}
	}
}

int diskv :: remove()
{
	char filename[FASTDI_PATH_MAXSIZE];
	if (m_append_fd>0) {
		close(m_append_fd);
		m_append_fd = -1;
	}
	for (uint32_t i=0; i<m_fnum; ++i) {
		if(m_rw_fd[i]>0) {
			close(m_rw_fd[i]);
			m_rw_fd[i] = -1;
		}
		snprintf( filename, FASTDI_PATH_MAXSIZE, FASTDI_DATA_FILE_FMT, m_di_path, m_module, i );
		remove(filename);
	}	

	if (m_fileblock.remove()<0 ) {
		ALARM( "%s idx remove failed.  %m", m_module );
		return -1;
	}
	return 0;
}

diskv :: ~diskv ()
{
	sync();
	if (m_append_fd>0) {
		close(m_append_fd);
		m_append_fd = -1;
	}
	for (uint32_t i=0; i<m_fnum; ++i) {
		if(m_rw_fd[i]>0) {
			close(m_rw_fd[i]);
			m_rw_fd[i] = -1;
		}
	}	

	free (m_pdatabuf_idx);
	m_pdatabuf_idx = NULL;
	free (m_pdatabuf_di);
	m_pdatabuf_di = NULL;
}

int diskv :: getdata(const diskv_idx_t* idx, char* data, const uint32_t datasize)
{
	if( datasize<=0 ||
			idx->fno >= m_fnum || 
			datasize < idx->dlen ||
			m_rw_fd[idx->fno] < 0  ||
			idx->dlen > FASTDI_ITEM_MAXSIZE || 
			(idx->off + idx->dlen) > FASTDI_FILE_MAXSIZE )
	{
		ALARM("%s err param. fno:%d off:%d dlen:%d datasize:%d rfd:%d"
                " DATAMAXLEN:%d FILE_MAXSIZE:%d fnum:%d",
				m_module, idx->fno, idx->off, idx->dlen, datasize, m_rw_fd[idx->fno],
                FASTDI_ITEM_MAXSIZE, FASTDI_FILE_MAXSIZE, m_fnum );
		return -1;
	}
	if ( 0 == idx->dlen){
		return 0;
	}

    // m_read[idx->fno] is OK?
	u_int ret = pread( m_rw_fd[idx->fno], data, idx->dlen, idx->off);
	if( ret!=idx->dlen )
	{
		ALARM("%s pread di data, failed. rfd[%d]:%d off:%u dlen:%u",
				m_module, idx->fno, m_rw_fd[idx->fno], idx->off, idx->dlen );
		return -1;
	}
	return idx->dlen;
}

int diskv :: append(diskv_idx_t* idx, const char* data, const uint32_t datasize)
{
	assert(idx != NULL && data != NULL );
	if (datasize <0 || datasize > FASTDI_ITEM_MAXSIZE || m_append_fd < 0  ) {
		ALARM("%s errdatasize:%d max: %u appendfd[%d]",
				m_module, datasize, FASTDI_ITEM_MAXSIZE, m_append_fd);
		return -1;
	}

	if( isneednewfile(datasize) )
	{
		int curnum=m_fnum;
		int wfd=0, rfd=0;

		if( (wfd = openfile(curnum, false))>0 && 
				(rfd = openfile(curnum, true))>0 )
		{
			fsync( m_append_fd );
			close( m_append_fd );
			m_append_fd = wfd;
			m_wdi_flen = 0;
			m_rw_fd[curnum] = rfd;
		}
		else
		{
			ALARM("%s opendi:%d msg[%m]", m_module, curnum );
			return -1;
		}
		m_fnum++;
	}

	if (0 == datasize) {
		return 0;
	}
	int ret = pwrite( m_append_fd, data, datasize, idx->off);
	if( (int)datasize == ret ){
		m_data_num ++;
		if( m_iswritesync )
        {
			fdatasync( m_append_fd );
        }
        idx->fno = m_fnum-1;
        idx->off = m_wdi_flen;
        m_wdi_flen += datasize;
        idx->dlen = datasize;
        return datasize;
    }
    else {
        ALARM("%s write di data, failed. fno:%d dsize:%d msg[%m]",
                m_module, idx->fno, datasize );
        return -1;
    }

    return -1;
}

int diskv :: append(const uint64_t id, const char* data, const uint32_t datasize)
{
    diskv_idx_t idx;
    memset (&idx, 0, sizeof(idx));
    idx.sign64 = id;
    int ret = 0;
    ret = append(&idx, data, datasize);
    if (ret != (int)datasize) {return ret;}
    // how to do sign64?
    return writeidx (id, &idx);
}

int diskv :: sync()
{
    fdatasync( m_append_fd );
    return 0;
}

int diskv :: readidx(const uint32_t idx_id, diskv_idx_t* idx)
{
    return m_fileblock.read(idx_id, idx, sizeof(diskv_idx_t) );
}

int diskv :: writeidx(uint32_t idx_id, const diskv_idx_t* idx)
{
    return m_fileblock.write(idx_id, idx);
}

bool diskv :: isneednewfile(const uint32_t appendsize)
{
    return (m_wdi_flen + appendsize) > FASTDI_FILE_MAXSIZE;
}

bool diskv :: isfileexist( const char* filename )
{
    return 0 == access( filename, X_OK);
}

uint32_t diskv :: getfilesize( const char* name )
{
    struct stat fs;
    if( 0!=stat( name, &fs ) )
        return 0;
    return fs.st_size;
}

uint32_t diskv :: getfilesize( FILE* fp )
{
    struct stat fs;
    if( 0!=fstat( fileno(fp), &fs ) )
        return 0;
    return fs.st_size;
}

int diskv :: openfile(const uint32_t fno, const bool readonly )
{
    if( fno >= FASTDI_FD_MAXNUM ) 
    {
        ALARM("%s fno:%d too large max: %u", m_module, fno, FASTDI_FD_MAXNUM);
        return -1;
    }
    char filename[FASTDI_PATH_MAXSIZE];
    snprintf( filename, FASTDI_PATH_MAXSIZE, FASTDI_DATA_FILE_FMT, m_di_path, m_module, fno );
    int fd=open( filename, readonly?O_RDONLY:(O_CREAT|O_RDWR), S_IRWXU|S_IRWXG|S_IROTH );
    if( fd<0 )
    {
        ALARM("%s open di file, failed. name:%s readonly:%d msg[%m]", 
                m_module, filename, readonly );
        while(0 != raise(SIGKILL)){}
    }
    return fd;
}

void diskv :: begin()
{
    m_done = false;
    m_iterator_id = 0;
    m_pdatabuf_idx = (char*)malloc(FASTDI_DATABUF_SIZE);
    if (NULL == m_pdatabuf_idx)
    {
        ALARM("module[%s] malloc iobuffer failed. size[%u] msg[%m]",
                m_module, FASTDI_DATABUF_SIZE);
        m_pdatabuf_idx_size = FASTDI_DATABUF_SIZE;
    }
    else
    {
        m_pdatabuf_idx_size = 0;
    }
    m_pdatabuf_di = (char*)malloc(FASTDI_DATABUF_SIZE);
    if (NULL == m_pdatabuf_di)
    {
        ALARM("module[%s] malloc iobuffer failed. size[%u] msg[%m]",
                m_module, FASTDI_DATABUF_SIZE);
        m_pdatabuf_di_size = FASTDI_DATABUF_SIZE;
    }
    else
    {
        m_pdatabuf_di_size = 0;
    }

    char namebuf[FASTDI_PATH_MAXSIZE];
    for( int i=0; i<=m_idx_handle.mfiles._cur_logicfno; i++ )
    {
        snprintf( namebuf, sizeof(namebuf), DATAFILE_NAME_FMT, m_idx_handle.mfiles._filepath, i );
        m_idx_pfile = fopen( namebuf, "r");
        if( NULL == m_idx_pfile )
        {
            ALARM("module[%s] [%s:%u]open idx file failed. path:%s %m",
                    m_module, __FILE__, __LINE__, namebuf);
            continue;
        }
        else
        {
            m_idx_curfno = i;
            m_iterator_id = m_idx_curfno * FBLOCK_PERBLOCK_IDX_SIZE;
            setvbuf( m_idx_pfile, m_pdatabuf_idx, _IOFBF, m_pdatabuf_idx_size);
            DEBUG( "module[%s] [%s:%u]open idx file. path:%s max[%u]",
                    m_module, __func__, __LINE__, namebuf, m_idx_handle.mfiles._cur_logicfno);
            return;
        } 
    }
    m_done = true;
    return;
}

int diskv :: next(uint32_t& id, diskv_idx_t& idx, void* buff, const uint32_t buffsize)
{
    if (m_done) {
        return 0; 
    }

    char namebuf[FASTDI_PATH_MAXSIZE]; 
    while (m_idx_pfile && fread( &m_curidx, sizeof(diskv_idx_t), 1, m_idx_pfile )!=1)
    {
        if (NULL != m_idx_pfile)
        {
            fclose( m_idx_pfile );
            m_idx_pfile = NULL;
        }

        for( int i=m_idx_curfno+1; i<=m_idx_handle.mfiles._cur_logicfno; i++ )
        {
            snprintf( namebuf, sizeof(namebuf), DATAFILE_NAME_FMT, m_idx_handle.mfiles._filepath, i );
            m_idx_pfile = fopen( namebuf, "r");
            if( !m_idx_pfile )
            {
                ALARM("module[%s] [%s:%u]open idx file failed. path:%s %m",
                        m_module, __FILE__, __LINE__, namebuf);
                continue;
            }
            else
            {
                m_idx_curfno = i;
                m_iterator_id = m_idx_curfno * FBLOCK_PERBLOCK_IDX_SIZE;
                setvbuf( m_idx_pfile, m_pdatabuf_idx, _IOFBF, m_pdatabuf_idx_size);
                DEBUG( "module[%s] [%s:%u]open idx file. path:%s max[%u]",
                        m_module, __func__, __LINE__, namebuf, m_idx_handle.mfiles._cur_logicfno);
                break;
            }
        }
    }

    if (NULL == m_idx_pfile)
    {
        m_done = true;
        return 0;
    }

    idx = m_curidx;
    id = m_iterator_id++;
    return iterator_read(buff, buffsize);
}

int diskv :: iterator_read(void* buff, const uint32_t buffsize)
{
    if (m_curidx.dlen > buffsize)
    {
        return -1;
    }
    if (m_curidx.dlen == 0)
    {
        return 0;
    }
    if(m_di_curfno != m_curidx.fno || m_di_pfile == NULL)
    {
        if (NULL != m_di_pfile)
        {
            fclose(m_di_pfile);
            m_di_pfile = NULL;
        }
        m_di_curfno = m_curidx.fno;
        char namebuf[FASTDI_PATH_MAXSIZE];
        snprintf( namebuf, FASTDI_PATH_MAXSIZE, FASTDI_DATA_FILE_FMT, m_di_path, m_module, m_di_curfno );
        if(!isfileexist( namebuf ) )
        {
            ALARM("module[%s] file does not exist.path:%s %m", m_module, namebuf);
            return -1;
        }
        m_di_pfile = fopen(namebuf,"r");
        if (!m_di_pfile)
        {
            ALARM("module[%s] open di file failed.path:%s %m", m_module, namebuf);
            return -1;
        }
        setvbuf( m_di_pfile, m_pdatabuf_di, _IOFBF, m_pdatabuf_di_size);
    }

    assert(m_di_pfile);
    if ( 0 != fseek(m_di_pfile,m_curidx.off,SEEK_SET) ) //
    {
        ALARM("fseek err.%m");
        return -1;
    }
    int ret = fread( buff, m_curidx.dlen,1, m_di_pfile  );
    if( ret != 1 )
    {
        ALARM( "%s pread di data, failed. fno:%d off:%u dlen:%u ret:%u",
                m_module, m_curidx.fno,  m_curidx.off, m_curidx.dlen,ret);
        return -1;
    }
    return m_curidx.dlen;
}

bool diskv :: end()
{
    if (((int)m_idx_curfno == m_idx_handle.mfiles._cur_logicfno) &&
            ((m_idx_pfile != NULL) && (ftell(m_idx_pfile) == (int)getfilesize(m_idx_pfile))))
    {
        m_done = true;
    }

    return m_done;
}
