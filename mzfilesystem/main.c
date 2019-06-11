/*
  Copyright (C) JAEHYUK CHO
  All rights reserved.
  Code by JaeHyuk Cho <mailto:minzkn@minzkn.com>
*/

#if !defined(FUSE_USE_VERSION)
# define FUSE_USE_VERSION 26
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <fuse.h>

#include "mztree.h"
#include "mzftpd.h"

static mztree_t *mzfuse_get_context(void);
static int mzfuse_log_puts(mztree_t *s_fs, const char *s_string);
static int mzfuse_log_vprintf(mztree_t *s_fs, const char *s_format, va_list s_var);
static int mzfuse_log_printf(mztree_t *s_fs, const char *s_format, ...);

static int mzfuse_getattr(const char *s_pathname, struct stat *s_statbuffer);
static int mzfuse_mkdir(const char *s_pathname, mode_t s_mode);
static int mzfuse_unlink(const char *s_pathname);
static int mzfuse_rmdir(const char *s_pathname);
static int mzfuse_rename(const char *s_oldpathname, const char *s_newpathname);
static int mzfuse_chmod(const char *s_pathname, mode_t s_mode);
static int mzfuse_open(const char *s_pathname, struct fuse_file_info *s_fileinfo);
static int mzfuse_read(const char *s_pathname, char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo);
static int mzfuse_write(const char *s_pathname, const char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo);
static int mzfuse_readdir(const char *s_pathname, void *s_buffer, fuse_fill_dir_t s_filler, off_t s_offset, struct fuse_file_info *s_fileinfo);
static void *mzfuse_init(struct fuse_conn_info *s_info);
static void mzfuse_destroy(void *s_context);
static int mzfuse_create(const char *s_pathname, mode_t s_mode, struct fuse_file_info *s_fileinfo);

int main(int s_argc, char **s_argv);

static struct fuse_operations g_mzfuse_operations = {
    /* int (*getattr) (const char *, struct stat *); */
    .getattr = mzfuse_getattr,
    /* int (*readlink) (const char *, char *, size_t); */
    /* int (*getdir) (const char *, fuse_dirh_t, fuse_dirfil_t); */
    /* int (*mknod) (const char *, mode_t, dev_t); */
    /* int (*mkdir) (const char *, mode_t); */
    .mkdir = mzfuse_mkdir,
    /* int (*unlink) (const char *); */
    .unlink = mzfuse_unlink,
    /* int (*rmdir) (const char *); */
    .rmdir = mzfuse_rmdir,
    /* int (*symlink) (const char *, const char *); */
    /* int (*rename) (const char *, const char *); */
    .rename = mzfuse_rename,
    /* int (*link) (const char *, const char *); */
    /* int (*chmod) (const char *, mode_t); */
    .chmod = mzfuse_chmod,
    /* int (*chown) (const char *, uid_t, gid_t); */
    /* int (*truncate) (const char *, off_t); */
    /* int (*utime) (const char *, struct utimbuf *); */
    /* int (*open) (const char *, struct fuse_file_info *); */
    .open = mzfuse_open,
    /* int (*read) (const char *, char *, size_t, off_t, struct fuse_file_info *); */
    .read = mzfuse_read,
    /* int (*write) (const char *, const char *, size_t, off_t, struct fuse_file_info *); */
    .write = mzfuse_write,
    /* int (*statfs) (const char *, struct statvfs *); */
    /* int (*flush) (const char *, struct fuse_file_info *); */
    /* int (*release) (const char *, struct fuse_file_info *); */
    /* int (*fsync) (const char *, int, struct fuse_file_info *); */
    /* int (*setxattr) (const char *, const char *, const char *, size_t, int); */
    /* int (*getxattr) (const char *, const char *, char *, size_t); */
    /* int (*listxattr) (const char *, char *, size_t); */
    /* int (*removexattr) (const char *, const char *); */
    /* int (*opendir) (const char *, struct fuse_file_info *); */
    /* int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *); */
    .readdir = mzfuse_readdir,
    /* int (*releasedir) (const char *, struct fuse_file_info *); */
    /* int (*fsyncdir) (const char *, int, struct fuse_file_info *); */
    /* void *(*init) (struct fuse_conn_info *conn); */
    .init = mzfuse_init,
    /* void (*destroy) (void *); */
    .destroy = mzfuse_destroy,
    /* int (*access) (const char *, int); */
    /* int (*create) (const char *, mode_t, struct fuse_file_info *); */
    .create = mzfuse_create,
    /* int (*ftruncate) (const char *, off_t, struct fuse_file_info *); */
    /* int (*fgetattr) (const char *, struct stat *, struct fuse_file_info *); */
    /* int (*lock) (const char *, struct fuse_file_info *, int cmd, struct flock *); */
    /* int (*utimens) (const char *, const struct timespec tv[2]); */
    /* int (*bmap) (const char *, size_t blocksize, uint64_t *idx); */
    /* unsigned int flag_nullpath_ok : 1; */
    /* unsigned int flag_reserved : 31; */
    /* int (*ioctl) (const char *, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data); */
    /* int (*poll) (const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp); */
};

static mztree_t *mzfuse_get_context(void)
{
    struct fuse_context *s_fuse_context;

    s_fuse_context = fuse_get_context();
    if(s_fuse_context == ((struct fuse_context *)0)) {
        return((mztree_t *)0);
    }

    return((mztree_t *)s_fuse_context->private_data);
}

static int mzfuse_log_puts(mztree_t *s_fs, const char *s_string)
{
    mztree_node_t *s_node;

    s_node = mztree_create(s_fs, "/fs.log", S_IFREG | 0444);
    if(s_node == ((mztree_node_t *)0)) {
        return(-1);
    }

    return(mztree_write(s_node, (const void *)s_string, strlen(s_string), (off_t)s_node->m_statbuffer.st_size));
}

static int mzfuse_log_vprintf(mztree_t *s_fs, const char *s_format, va_list s_var)
{
    char s_buffer[ 1 << 10 ];

    vsprintf((char *)(&s_buffer[0]), s_format, s_var);

    return(mzfuse_log_puts(s_fs, (const char *)(&s_buffer[0])));
}

static int mzfuse_log_printf(mztree_t *s_fs, const char *s_format, ...)
{
    int s_result;
    va_list s_var;

    va_start(s_var, s_format);
    s_result = mzfuse_log_vprintf(s_fs, s_format, s_var);
    va_end(s_var);

    return(s_result);
}

static int mzfuse_getattr(const char *s_pathname, struct stat *s_statbuffer)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    
    if(strcmp(s_pathname, "/") == 0) {
        s_statbuffer->st_mode = S_IFDIR | 0755;
	s_statbuffer->st_nlink = 2;

	return(0);
    }

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        return(-ENOENT);
    }
    
    (void)memcpy((void *)s_statbuffer, (const void *)(&s_node->m_statbuffer), sizeof(struct stat));
    
    return(0);
}

static int mzfuse_mkdir(const char *s_pathname, mode_t s_mode)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_mkdir failed ! (pathname=\"%s\", mode=%08XH, -EEXIST)\n", s_pathname, (unsigned int)s_mode);
        return(-EEXIST);
    }
    
    s_node = mztree_create(s_fs, s_pathname, S_IFDIR | s_mode);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_mkdir failed ! (pathname=\"%s\", mode=%08XH, -EACCES)\n", s_pathname, (unsigned int)s_mode);
        return(-EACCES);
    }
    
    mzfuse_log_printf(s_fs, "* mzfuse_mkdir sucess. (pathname=\"%s\", mode=%08XH)\n", s_pathname, (unsigned int)s_mode);
    
    return(0);
}

static int mzfuse_unlink(const char *s_pathname)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    
    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_unlink failed ! (pathname=\"%s\", -ENOENT)\n", s_pathname);
        return(-ENOENT);
    }
    
    if(S_ISDIR(s_node->m_statbuffer.st_mode) != 0) {
        if(s_node->m_sub != ((mztree_node_t *)0)) {
            mzfuse_log_printf(s_fs, "* mzfuse_rmdir failed ! (pathname=\"%s\", -ENOTEMPTY)\n", s_pathname);
            return(-ENOTEMPTY);
        }
    }

    s_node = mztree_remove(s_fs, s_pathname);
    if(s_node != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_unlink failed ! (pathname=\"%s\", -EACCES)\n", s_pathname);
        return(-EACCES);
    }
    
    mzfuse_log_printf(s_fs, "* mzfuse_unlink sucess. (pathname=\"%s\")\n", s_pathname);

    return(0);
}

static int mzfuse_rmdir(const char *s_pathname)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    
    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rmdir failed ! (pathname=\"%s\", -ENOENT)\n", s_pathname);
        return(-ENOENT);
    }

    if(S_ISDIR(s_node->m_statbuffer.st_mode) == 0) {
        mzfuse_log_printf(s_fs, "* mzfuse_rmdir failed ! (pathname=\"%s\", -ENOTDIR)\n", s_pathname);
        return(-ENOTDIR);
    }

    if(s_node->m_sub != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rmdir failed ! (pathname=\"%s\", -ENOTEMPTY)\n", s_pathname);
        return(-ENOTEMPTY);
    }

    s_node = mztree_remove(s_fs, s_pathname);
    if(s_node != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rmdir failed ! (pathname=\"%s\", -EACCES)\n", s_pathname);
        return(-EACCES);
    }
    
    mzfuse_log_printf(s_fs, "* mzfuse_rmdir sucess. (pathname=\"%s\")\n", s_pathname);

    return(0);
}

static int mzfuse_rename(const char *s_oldpathname, const char *s_newpathname)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    
    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_oldpathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rename failed ! (oldpathname=\"%s\", newpathname=\"%s\", -ENOENT)\n", s_oldpathname, s_newpathname);
        return(-ENOENT);
    }
    
    if(mztree_search(s_fs, s_oldpathname) != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rename failed ! (oldpathname=\"%s\", newpathname=\"%s\", -EEXIST)\n", s_oldpathname, s_newpathname);
        return(-EEXIST);
    }

    s_node = mztree_rename(s_fs, s_oldpathname, s_newpathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_rename failed ! (oldpathname=\"%s\", newpathname=\"%s\", -EACCES)\n", s_oldpathname, s_newpathname);
        return(-EACCES);
    }
        
    mzfuse_log_printf(s_fs, "* mzfuse_rename success. (oldpathname=\"%s\", newpathname=\"%s\")\n", s_oldpathname, s_newpathname);

    return(0);
}

static int mzfuse_chmod(const char *s_pathname, mode_t s_mode)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    
    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_chmod failed ! (pathname=\"%s\", mode=%08XH, -ENOENT)\n", s_pathname, (unsigned int)s_mode);
        return(-ENOENT);
    }

    s_node->m_statbuffer.st_mode = s_mode;

    mzfuse_log_printf(s_fs, "* mzfuse_chmod sucess. (pathname=\"%s\", mode=%08XH)\n", s_pathname, (unsigned int)s_mode);

    return(0);
}

static int mzfuse_open(const char *s_pathname, struct fuse_file_info *s_fileinfo)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_open failed ! (pathname=\"%s\", flags=%08XH, -ENOENT)\n", s_pathname, (unsigned int)s_fileinfo->flags);
        return(-ENOENT);
    }
    
    if((s_fileinfo->flags & O_RDONLY) != O_RDONLY) {
        mzfuse_log_printf(s_fs, "* mzfuse_open failed ! (pathname=\"%s\", flags=%08XH, -EACCES)\n", s_pathname, (unsigned int)s_fileinfo->flags);
        return(-EACCES);
    }
    
    mzfuse_log_printf(s_fs, "* mzfuse_open sucess. (pathname=\"%s\", flags=%08XH)\n", s_pathname, (unsigned int)s_fileinfo->flags);

    return(0);
}

static int mzfuse_read(const char *s_pathname, char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    ssize_t s_read_bytes;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        return(-ENOENT);
    }

    s_read_bytes = mztree_read(s_node, (void *)s_data, s_size, s_offset);

    return((int)s_read_bytes);
}

static int mzfuse_write(const char *s_pathname, const char *s_data, size_t s_size, off_t s_offset, struct fuse_file_info *s_fileinfo)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;
    ssize_t s_write_bytes;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        return(-ENOENT);
    }

    s_write_bytes = mztree_write(s_node, (const void *)s_data, s_size, s_offset);

    return((int)s_write_bytes);
}

static int mzfuse_readdir(const char *s_pathname, void *s_buffer, fuse_fill_dir_t s_filler, off_t s_offset, struct fuse_file_info *s_fileinfo)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
   
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_readdir failed ! (pathname=\"%s\", -ENOENT)\n", s_pathname);
        return(-ENOENT);
    }
    if((strcmp(s_pathname, "/") != 0) || (S_ISDIR(s_node->m_statbuffer.st_mode) != 0)) {
        s_node = s_node->m_sub;
    }

    s_filler(s_buffer, ".", (const struct stat *)0, (off_t)0);
    s_filler(s_buffer, "..", (const struct stat *)0, (off_t)0);

    while(s_node != ((mztree_node_t *)0)) {
        /* typedef int (*fuse_fill_dir_t) (void *buf, const char *name, const struct stat *stbuf, off_t off); */
        s_filler(s_buffer, (const char *)s_node->m_name, (const struct stat *)0, (off_t)0);

        s_node = s_node->m_next;
    }
	
    mzfuse_log_printf(s_fs, "* mzfuse_readdir sucess. (pathname=\"%s\")\n", s_pathname);

    return(0);
}

static void *mzfuse_init(struct fuse_conn_info *s_info)
{
    mztree_t *s_fs;
    mzftpd_t s_ftpd;
   
    (void)s_info;
    
    s_fs = mztree_new();
    if(s_fs == ((mztree_t *)0)) {
        return((void *)0);
    }
  
    mzfuse_log_printf(s_fs, "* mztree initialized. [%s %s]\n", __DATE__, __TIME__);

    mztree_create(s_fs, "/SUBDIR", S_IFDIR | 0755);
    mztree_create(s_fs, "/SUBDIR/SUB_00", S_IFDIR | 0755);
    mztree_create(s_fs, "/SUBDIR/SUB_01", S_IFDIR | 0755);
    mztree_create(s_fs, "/SUBDIR/SUB_02", S_IFDIR | 0755);
    mztree_create(s_fs, "/SUBDIR/SUB_03", S_IFDIR | 0755);
    mztree_create(s_fs, "/SUBDIR/TEST FILE 00", S_IFREG | 0644);
    mztree_create(s_fs, "/SUBDIR/TEST FILE 01", S_IFREG | 0644);
    
    s_ftpd = mzftpd_open_with_port(21);
    if(s_ftpd == ((mzftpd_t)0)) {
        s_ftpd = mzftpd_open_with_port(2211);
        if(s_ftpd == ((mzftpd_t)0)) {
            s_ftpd = mzftpd_open_with_port(2121);
            if(s_ftpd == ((mzftpd_t)0)) {
                mzfuse_log_printf(s_fs, "* mzftpd_open failed !\n");
	    }
	    else {
	        mzfuse_log_printf(s_fs, "* mzftpd_open sucess. (port=%d)\n", 2121);
	    }
	}
	else {
	    mzfuse_log_printf(s_fs, "* mzftpd_open sucess. (port=%d)\n", 2211);
	}
    }
    else {
	mzfuse_log_printf(s_fs, "* mzftpd_open sucess. (port=%d)\n", 21);
    }
    if(s_ftpd != ((mzftpd_t)0)) {
        (void)mzftpd_add_user(s_ftpd, (mzftpd_account_t **)0, def_mzftpd_account_flag_none | def_mzftpd_account_flag_guest_user, "ftp", (const char *)0, (const char *)0 /* "/home/ftp" */);
        (void)mzftpd_add_user(s_ftpd, (mzftpd_account_t **)0, def_mzftpd_account_flag_none | def_mzftpd_account_flag_guest_user, "anonymous", (const char *)0, (const char *)0 /* "/home/ftp" */);

        s_fs->m_ftpd_context = (void *)s_ftpd;
    }

    
    return((void *)s_fs);
}

static void mzfuse_destroy(void *s_context)
{
    mztree_t *s_fs = (mztree_t *)s_fs;
 
    if(s_fs != ((mztree_t *)0)) {
        mzftpd_t s_ftpd = s_fs->m_ftpd_context;

        if(s_ftpd != ((mzftpd_t)0)) {
	    s_ftpd = mzftpd_close(s_ftpd);
	    s_fs->m_ftpd_context = (void *)s_ftpd;
	}

        s_fs = mztree_delete(s_fs);
    }
}
    
static int mzfuse_create(const char *s_pathname, mode_t s_mode, struct fuse_file_info *s_fileinfo)
{
    mztree_t *s_fs;
    mztree_node_t *s_node;

    s_fs = mzfuse_get_context();
    if(s_fs == ((mztree_t *)0)) {
        return(-ENOENT);
    }
    
    s_node = mztree_search(s_fs, s_pathname);
    if(s_node != ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_create failed ! (pathname=\"%s\", mode=%08XH, flags=%08XH, -EEXIST)\n", s_pathname, (unsigned int)s_mode, (unsigned int)s_fileinfo->flags);
        return(-EEXIST);
    }
    
    s_node = mztree_create(s_fs, s_pathname, s_mode);
    if(s_node == ((mztree_node_t *)0)) {
        mzfuse_log_printf(s_fs, "* mzfuse_create failed ! (pathname=\"%s\", mode=%08XH, flags=%08XH, -EACCES)\n", s_pathname, (unsigned int)s_mode, (unsigned int)s_fileinfo->flags);
        return(-EACCES);
    }
    
    mzfuse_log_printf(s_fs, "* mzfuse_create sucess. (pathname=\"%s\", mode=%08XH, flags=%08XH)\n", s_pathname, (unsigned int)s_mode, (unsigned int)s_fileinfo->flags);
    
    return(0);
}

int main(int s_argc, char **s_argv)
{
    /* int fuse_main(int argc, char *argv[], const struct fuse_operations *op, void *user_data); */
    return(fuse_main(s_argc, s_argv, (const struct fuse_operations *)(&g_mzfuse_operations), (void *)0));
}

/* End of source */
