#include "filelinkblock.h"
#include "MyException.h"
#include "mylog.h"
#include "creat_sign.h"
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>

const char* const FileLinkBlock::BASE_DATA_PATH = "./data/";
const char* const FileLinkBlock::BASE_OFFSET_PATH = "./offset/";
FileLinkBlock::FileLinkBlock(const char* path, const char* name, bool readonly)
{
    MySuicideAssert(0 < strlen(path) && 0 < strlen(name));
    snprintf(m_flb_path, sizeof(m_flb_path), "%s/%s/", BASE_DATA_PATH, path);
    snprintf(m_flb_name, sizeof(m_flb_name), "%s", name);
    if (0 != access(BASE_DATA_PATH, F_OK))
    {
        MySuicideAssert(0 == mkdir(BASE_DATA_PATH, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IXOTH|S_IROTH));
    }
    if (0 != access(BASE_OFFSET_PATH, F_OK))
    {
        MySuicideAssert(0 == mkdir(BASE_OFFSET_PATH, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IXOTH|S_IROTH));
    }
    if (0 != access(m_flb_path, F_OK))
    {
        MySuicideAssert(0 == mkdir(m_flb_path, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IXOTH|S_IROTH));
    }

    DEBUG ("path[%s] name[%s]", path, name);
    pthread_mutex_init(&m_mutex, NULL);
    m_flb_w_fd      = -1;
    m_last_file_no  =  0;
    m_block_id      =  0;
    m_channel_name[0] =  0;
    m_read_file_name[0] =  0;

    if (!readonly)
    {
        check_and_repaire();
    }
}

FileLinkBlock::~FileLinkBlock()
{
    if (m_flb_w_fd > 0)
    {
        close(m_flb_w_fd);
        m_flb_w_fd = -1;
    }
    if (m_flb_r_fd > 0)
    {
        close(m_flb_r_fd);
        m_flb_r_fd = -1;
    }
}

int FileLinkBlock:: newfile (const char* strfile)
{
    mode_t amode = (0 == access(strfile, F_OK)) ? O_WRONLY|O_APPEND|O_TRUNC : O_WRONLY|O_CREAT|O_APPEND|O_TRUNC;
    m_flb_w_fd = open(strfile, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    DEBUG ("fd[%d] file[%s]", m_flb_w_fd, strfile);
    MySuicideAssert(m_flb_w_fd != -1);
    m_block_id = 0;
    return m_flb_w_fd;
}

void FileLinkBlock:: check_and_repaire()
{
    // ��Ҫ������ǰ�ļ��е����һ���ļ�
    // (1) ������մ�С�з֣�ɨ�赱ǰ�ļ��У��ҵ� name.n �������Ǹ��ļ�
    // (2) �������ʱ���з֣���ɨ��� name.YYYY-MM-DD-HH.n �������Ǹ��ļ���
    //     YYYY-MM-DD-HH��ʱ������ǵ�ǰʱ�䣬��������ļ�����׷��д��������ǣ����½���ǰʱ����ļ�
    // �죬C���Դ�����Щ�ַ�����ֱ��ج�Ρ���

    int max_file_no = detect_file();
    m_last_file_no = max_file_no < 0? 0 : max_file_no;

    char last_file_name[128];
    snprintf(last_file_name, sizeof(last_file_name), "%s/%s.%d", m_flb_path, m_flb_name, m_last_file_no);

    if (0 > max_file_no )
    {
        // ����ļ������ڻ���̫С�ˣ���Ҫ�½��ļ�
        m_flb_w_fd = newfile(last_file_name);
    }
    else
    {
        // ������һ���ļ����ؼ������ҵ����һ������д���Block
        // (1) ���ҵ�MAGIC_NUM������ҵ��� MAGIC_NUM����ʹ��ǩ���жϺ�����buff�Ƿ�����д��
        //     ����д������ļ�ִ��׷��д���ɣ�Ҫע��Ѻ���д���Ĳ��ָ�TRUNC����
        //     ���� goto(1) Ҫ������ǰѰ�� MAGIC_NUM;
        // (2) ���û�ҵ� MAGIC_NUM, ���ļ�����С�� BLOCK_MAX_SIZE �����½��ļ�
        //     ���򱨴��˳�����Ϊ������û�� MAGIC_NUM
        // ��ȡ 2*BLOCK_MAX_SIZE ��С��Ϊ�˿϶����ҵ�һ����ȷд��Ŀ�

        struct stat dstat;
        int ret = stat(last_file_name, &dstat);
        DEBUG("so[%s] size [%ld] ret[%d] %m", last_file_name, dstat.st_size, ret); 
        MySuicideAssert(0 == ret);
        if (dstat.st_size <= (int)sizeof(file_link_block_head))
        {
            // ���û�г��� head �Ĵ�С���϶���Ҫ�½��ļ���
            m_flb_w_fd = newfile(last_file_name);
            return;
        }

        // >>2<<2��Ϊ��ȥ���ļ�ĩβ�Ķ����ַ�(���ļ���С���ܱ�4����ʱ)
        // ����Ѱ�� MAGIC_NUM �Ϳ��Ժܷ����ˡ�
        uint32_t cutlen  = ((dstat.st_size >> 2) << 2);
        uint32_t readlen = cutlen >= 2*BLOCK_MAX_SIZE ?  2*BLOCK_MAX_SIZE : cutlen;
        uint32_t readoff = cutlen >= 2*BLOCK_MAX_SIZE ?  cutlen - 2*BLOCK_MAX_SIZE : 0;

        // ��ȡ�ļ�β����Ѱ�� magic_num
        uint32_t* tmpbuf = (uint32_t*) malloc(readlen);
        MySuicideAssert(tmpbuf != NULL);
        int flb_r_fd = open(last_file_name, O_RDONLY);
        MySuicideAssert(flb_r_fd > 0);
        MySuicideAssert(readoff == (uint32_t)lseek(flb_r_fd, readoff, SEEK_SET));
        // �� readoff λ�ö�ȡ����ֱ���ļ�ĩβ
        MySuicideAssert((int)readlen == read (flb_r_fd, tmpbuf, readlen));
        int step = readlen/sizeof(uint32_t);
        while (--step >= 0)
        {
            if (tmpbuf[step] == MAGIC_NUM)
            {
                // ���һ���Ƿ���ľ���head�����ų������������ú������magic_num
                // �������һ��ǩ����У��
                struct file_link_block_head* phead = (struct file_link_block_head*) &tmpbuf[step];
                MySuicideAssert(phead->magic_num == MAGIC_NUM);
                // m_block_id = phead->block_id + 1;
                DEBUG("block_id[%d] block_size[%d] log_id[%d] readlen[%d] readoff[%d] step[%d]",
                        phead->block_id, phead->block_size, phead->log_id, readlen, readoff, step);
                if ((phead->block_size + sizeof(file_link_block_head)) > BLOCK_MAX_SIZE)
                {
                    DEBUG("Maybe this is data zone. call me to buy lottery.");
                    continue;
                }
                if (phead->block_size + sizeof(file_link_block_head) > readlen-step*4 )
                {
                    DEBUG("Data zone too short.");
                    continue;
                }
                else
                {
                    uint32_t sum0, sum1;
                    creat_sign_64(phead->block_buff, phead->block_size, &sum0, &sum1);
                    if (phead->check_sum[0] == sum0 && phead->check_sum[1] == sum1)
                    {
                        // У��ͨ�����������һ����ȷ
                        m_block_id = phead->block_id + 1;
                        // ��д������� offset ��λ��׷��д��λ��
                        uint32_t woffset = readoff + step*4 +
                            sizeof(file_link_block_head) + (((phead->block_size+3) >> 2) << 2);
                        ROUTN("Find normal end block. woffset[%u] filesize[%lu] block_id[%d]",
                                woffset, dstat.st_size, phead->block_id);
                        MySuicideAssert(woffset <= (uint32_t)dstat.st_size);
                        m_flb_w_fd = open(last_file_name, O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                        MySuicideAssert(m_flb_w_fd > 0);
                        lseek(m_flb_w_fd, woffset, SEEK_SET);
                        break;
                    }
                    else
                    {
                        ALARM("Verify Failed. buffsize[%d] sum1[%u] sum2[%u] headsum1[%u] headsum2[%u]",
                                phead->block_size, sum0, sum1, phead->check_sum[0], phead->check_sum[1]);
                        continue;
                    }
                }
            }
        }

        if (readoff == 0 && step < 0)
        {
            DEBUG ("Can't find the magic number. size[%lu] BLOCK_MAX_SIZE[%d] readoff[%d]",
                    dstat.st_size, BLOCK_MAX_SIZE, readoff);
            m_flb_w_fd = newfile(last_file_name);
        }
        else if (readoff != 0 && step < 0)
        {
            FATAL ("Can't find the magic number. size[%lu] BLOCK_MAX_SIZE[%d] readoff[%d]",
                    dstat.st_size, BLOCK_MAX_SIZE, readoff);
            MySuicideAssert(0);
        }
        MySuicideAssert(m_flb_w_fd > 0);

        close(flb_r_fd);
        free (tmpbuf);
    }
    return;
}
int FileLinkBlock::detect_file( )
{
    DIR *dp;
    struct  dirent  *dirp;

    char prefix[128];
    snprintf(prefix, sizeof(prefix), "%s.", m_flb_name);

    if((dp = opendir(m_flb_path)) == NULL)
    {
        FATAL( "can't open %s! msg[%m] ", m_flb_path);
        MySuicideAssert(0);
    }

    int len = strlen(prefix);
    int max = -1;

    while((dirp = readdir(dp)) != NULL)
    {
        DEBUG( "%s", dirp->d_name);
        char* pn = strstr(dirp->d_name, prefix);
        if (pn != NULL)
        {
            const char* pp = &pn[len];
            const char* cc = pp;
            // ����׺�Ƿ�ȫ������
            while (isdigit(*cc)) { cc++; }
            if (*cc == '\0')
            {
                int n = atoi(&pn[len]);
//                DEBUG( "-- %d", n);
                if (n > max)
                {
                    max = n;
                }
            }
//            else
//            {
//                DEBUG( "^^ invalid file name : %s", dirp->d_name);
//            }
        }
    }

    closedir(dp);
    return max;
}

int FileLinkBlock::__write_message(const uint32_t log_id, const char* buff, const uint32_t buff_size)
{
    // (buff_size>>2)<<2 ��Ϊ�˲��벻��4���ֽڣ�����������magic_num�Ĳ��Ҹ��ӷ���
    uint32_t wlen = (((buff_size+3)>>2)<<2) + sizeof(file_link_block_head);
    if ( wlen > BLOCK_MAX_SIZE)
    {
        ALARM("buff_size[%d] too loooong. head[%u] wlen[%d]", buff_size, sizeof(file_link_block_head), wlen);
        return -1;
    }
    // ����Ƿ�Ӧ���½�һ���ļ���д
    uint32_t cur_offset = lseek(m_flb_w_fd, 0, SEEK_CUR);
    if (cur_offset + wlen > FILE_MAX_SIZE)
    {
        ROUTN("Need a New File. cur_file_no[%d] cur_offset[%d] wlen[%d]", m_last_file_no, cur_offset, wlen);
        close(m_flb_w_fd);
        m_flb_w_fd = -1;

        char last_file_name[128];
        m_last_file_no++;
        snprintf(last_file_name, sizeof(last_file_name), "%s/%s.%d", m_flb_path, m_flb_name, m_last_file_no);
        m_flb_w_fd = newfile(last_file_name);
        __write_message(log_id, buff, buff_size); // digui 
    }
    else
    {
        struct file_link_block_head myhead;
        memset (&myhead, 0, sizeof(file_link_block_head));
        myhead.magic_num  = MAGIC_NUM;
        myhead.block_id   = m_block_id;
        myhead.timestamp  = time(NULL);
        myhead.log_id     = log_id;
        myhead.block_size = buff_size;
        creat_sign_64 (buff, buff_size, &myhead.check_sum[0] , &myhead.check_sum[1]);

        char extra_buf[4];
        uint32_t  extra_len = (4 - (buff_size % 4 )) % 4;
        struct iovec wblocks[3];
        wblocks[0].iov_base = &myhead;
        wblocks[0].iov_len  = sizeof(file_link_block_head);
        wblocks[1].iov_base = (char*)buff;
        wblocks[1].iov_len  = buff_size;
        wblocks[2].iov_base = extra_buf;
        wblocks[2].iov_len  = extra_len;
        int wwlen = writev(m_flb_w_fd, wblocks, 3);
        DEBUG("wwlen[%d] wlen[%u] head[%u] bsize[%u] extrasize[%u]",
                wwlen, wlen, sizeof(file_link_block_head), buff_size, extra_len);
        MySuicideAssert(wwlen == (int)wlen);
        m_block_id ++;
    }
    return 0;
}
int FileLinkBlock::write_message(const uint32_t log_id, const char* buff, const uint32_t buff_size)
{
    assert (buff != NULL && buff_size > 0);
    pthread_mutex_lock(&m_mutex);
    int ret = __write_message(log_id, buff, buff_size);
    pthread_mutex_unlock(&m_mutex);
    return ret;
}
int FileLinkBlock::seek_message(const uint32_t file_no, const uint32_t file_offset, const uint32_t block_id)
{
    snprintf(m_read_file_name, sizeof(m_read_file_name), "%s/%s.%u", m_flb_path, m_flb_name, file_no);
    m_flb_r_fd = open(m_read_file_name, O_RDONLY);
    MySuicideAssert(m_flb_r_fd > 0);
    MySuicideAssert(file_offset == (uint32_t)lseek(m_flb_r_fd, file_offset, SEEK_SET));
    m_flb_read_offset   = file_offset;
    m_flb_read_block_id = block_id;
    m_flb_read_file_no  = file_no;
    DEBUG ("file[%s] file_no[%u] offset[%u] block_id[%u]", 
            m_read_file_name, file_no, file_offset, block_id);
    return 0;
}

int FileLinkBlock::seek_message(const uint32_t file_no, const uint32_t block_id)
{
    if (block_id == 0)
    {
        return seek_message(file_no, 0, block_id);
    }
    snprintf(m_read_file_name, sizeof(m_read_file_name), "%s/%s.%u", m_flb_path, m_flb_name, file_no);
    m_flb_r_fd = open(m_read_file_name, O_RDONLY);
    MySuicideAssert(m_flb_r_fd > 0);
    MySuicideAssert(0 == lseek(m_flb_r_fd, 0, SEEK_SET));
    struct file_link_block_head myhead;
    struct stat dstat;
    int ret = stat(m_read_file_name, &dstat);
    MySuicideAssert(ret == 0);

    while (1)
    {
        MySuicideAssert(sizeof(myhead) == readn(m_flb_r_fd, (char*)&myhead, sizeof(myhead)));
        MySuicideAssert(myhead.magic_num == MAGIC_NUM);
        MySuicideAssert(BLOCK_MAX_SIZE > myhead.block_size + sizeof(file_link_block_head));
        uint32_t stepsize = ((myhead.block_size + 3 ) >> 2 ) << 2;
        MySuicideAssert (-1 != lseek(m_flb_r_fd, stepsize, SEEK_CUR));
        m_flb_read_offset   = lseek(m_flb_r_fd, 0, SEEK_CUR);
        // У��block_id���ɣ��Ͳ��ü���check_num��
        if (block_id - 1 == myhead.block_id)
        {
            m_flb_read_block_id = block_id;
            m_flb_read_file_no  = file_no;
            DEBUG ("file[%s] blocksize[%u] file_no[%u] offset[%u] block_id[%u]", 
                    m_read_file_name, myhead.block_size, file_no, m_flb_read_offset, block_id);
            break;
        }
        if ((int)m_flb_read_offset == dstat.st_size)
        {
            ALARM("File no[%u] size[%lu] Block ID[%u] to biiiiiiiiiig.", file_no, dstat.st_size, block_id);
            MySuicideAssert(0);
        }
    }

    return 0;
}

int FileLinkBlock::read_message(uint32_t& log_id, uint32_t& block_id, char* buff, const uint32_t buff_size)
{
    assert (buff != NULL && buff_size > 0);
    struct file_link_block_head myhead;
    uint32_t try_count = 0;
    uint32_t read_size = 0;
    while(1)
    {
        // ����Ƿ��Ѿ��л����ļ���
        if (try_count > 2 && ( (int)m_flb_read_file_no < detect_file()))
        {
            // �Ѿ��л����ļ���
            close (m_flb_r_fd);
            m_flb_r_fd = -1;
            DEBUG("change to new file. file_no[%d]", m_flb_read_file_no+1); 
            seek_message(m_flb_read_file_no+1, 0, 0);
            try_count = 0;
            continue;
        }
        struct stat dstat;
        int ret = stat(m_read_file_name, &dstat);
        DEBUG("name[%s] size [%ld] ret[%d] try[%d] %m", m_read_file_name, dstat.st_size, ret, try_count); 
        MySuicideAssert(0 == ret && dstat.st_size >= (int)m_flb_read_offset);
        // ����Ƿ������ӵ�����
        if ( dstat.st_size == (int)m_flb_read_offset )
        {
            usleep(CHECK_INTERVAL);
            try_count++;
            continue;
        }
        else
        {
            // ��ʼ������
            MySuicideAssert(sizeof(myhead) == readn(m_flb_r_fd, (char*)&myhead, sizeof(myhead)));
            MySuicideAssert(myhead.magic_num == MAGIC_NUM);
            uint32_t stepsize = ((myhead.block_size + 3 ) >> 2 ) << 2;
            MySuicideAssert (stepsize < buff_size);
            MySuicideAssert(BLOCK_MAX_SIZE > myhead.block_size + sizeof(file_link_block_head));
            MySuicideAssert(stepsize == (uint32_t)readn(m_flb_r_fd, buff, stepsize));
            m_flb_read_offset = lseek(m_flb_r_fd, 0, SEEK_CUR);
            log_id            = myhead.log_id;
            block_id          = myhead.block_id;
            // У��block_id���ɣ��Ͳ��ü���check_num��
            MySuicideAssert(m_flb_read_block_id == myhead.block_id);
            DEBUG("get new data! name[%s] size [%u] block_id[%u] try[%u]",
                    m_read_file_name, myhead.block_size, myhead.block_id, try_count); 
            m_flb_read_block_id ++;
            read_size = myhead.block_size;
            break;
        }
    }
    return read_size;
}

void FileLinkBlock:: set_channel(const char* channel_name)
{
    // д���ļ�
    MySuicideAssert (channel_name != NULL && 0 < strlen(channel_name));
    snprintf (m_channel_name, sizeof(m_channel_name), "%s/%s", BASE_OFFSET_PATH, channel_name);
    return;
}

void FileLinkBlock:: load_offset(uint32_t &file_no, uint32_t& offset, uint32_t& block_id)
{
    // д���ļ�
    MySuicideAssert (0 < strlen(m_channel_name));
    FILE* fp = fopen(m_channel_name, "r");
    MySuicideAssert (fp != NULL);
    char rbuff[128];
    fgets(rbuff, sizeof(rbuff), fp);
    int ret = 0;
    ret = sscanf (rbuff, "file_no  : %u", &file_no);
    MySuicideAssert (1 == ret);
    fgets(rbuff, sizeof(rbuff), fp);
    ret = sscanf (rbuff, "offset   : %u", &offset);
    MySuicideAssert (1 == ret);
    fgets(rbuff, sizeof(rbuff), fp);
    ret = sscanf (rbuff, "block_id : %u", &block_id);
    MySuicideAssert (1 == ret);
    DEBUG("file_no : %u offset : %u block_id : %u", file_no, offset, block_id);
    fclose(fp);
}

void FileLinkBlock:: save_offset()
{
    // д���ļ�
    MySuicideAssert (0 < strlen(m_channel_name));
    int wfd = open(m_channel_name, O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    MySuicideAssert(wfd > 0);
    char wbuff[1024];
    int wlen = snprintf(wbuff, sizeof(wbuff),
            "file_no  : %u\n"
            "offset   : %u\n"
            "block_id : %u",
            m_flb_read_file_no, m_flb_read_offset, m_flb_read_block_id);
    MySuicideAssert(wlen == write(wfd, wbuff, wlen));
    close(wfd);
    return; 
}

int FileLinkBlock::readn(int fd, char* buf, const uint32_t size)
{
    int left = size;
    while (left > 0)
    {
        int len = read(fd, buf+size-left, left);
        if (len == -1 || len == 0)
        {

            ALARM( "readfile failed. ERROR[%m]" );
            continue;
        }
        left -= len;
    }
    return size;
}




















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
