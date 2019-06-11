#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <fcntl.h>
#include "myStruct.h"
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

DirTree *Root;
DirTree *hello;
FileDes Fd;

DirTree *findWithPath(const char *path)
{
	DirTree *cur;
	char *tmp;
	char *restrict t_path=(char*restrict)malloc(sizeof(char)*(strlen(path)+1));
	if(path==NULL) {
		return NULL;
	}
	strcpy(t_path, path);
	tmp = strtok(t_path+1, "/");

	if(tmp==NULL) {
		return Root;
	}

	cur=Root->child;

	while(cur != NULL)
	{
		while(cur != NULL){
			if(strcmp(cur->name, tmp)==0) break;
			cur=cur->next;
		}
		if(cur==NULL){
			free(t_path);
			return NULL;
		}
		tmp=strtok(NULL, "/");
		if(tmp==NULL){
			free(t_path);
			return cur;
		}
		cur=cur->child;
	}
	free(t_path);
	return cur;
}

unsigned int checkPermission(unsigned int id, unsigned int fileMode, unsigned int mode){
	printf("***Debug checkPermission : %d %d\n", fileMode, mode);
	struct fuse_context *fc=fuse_get_context();
	if(id==fc->uid){
		if(!((mode&R_OK) && (fileMode&S_IRUSR))) {
			puts("***check Permission EACCEnd");
			return -EACCES;
		}
		if(!((mode&W_OK)&&(fileMode&S_IWUSR))) {
			puts("***check Permission eacc End");
			return -EACCES;
		}
		if(!((mode&X_OK) && (fileMode&S_IXUSR))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
	}
	else if(id==fc->gid){
		if(!((mode&R_OK) && (fileMode&S_IRGRP))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
		if(!((mode&W_OK)&&(fileMode&S_IWGRP))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
		if(!((mode&X_OK) && (fileMode&S_IXGRP))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
	}
	else{
		if(!((mode&R_OK) && (fileMode&S_IROTH))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
		if(!((mode&W_OK)&&(fileMode&S_IWOTH))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
		if(!((mode&X_OK) && (fileMode&S_IXOTH))) {
			puts("***check Permission EACC End");
			return -EACCES;
		}
	}
	puts("***check Permission End");
	return 0;
}
static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res;
	DirTree *cur;

	cur = findWithPath(path);
	if(cur==NULL) {
		return -ENOENT;
	}

	memset(stbuf,0,sizeof(struct stat));

	stbuf->st_mode = cur->st.st_mode;
	stbuf->st_uid = cur->st.st_uid;
	stbuf->st_gid = cur->st.st_gid;


	if(stbuf->st_mode&__S_IFDIR)
		stbuf->st_nlink	= 2;
	else
		stbuf->st_nlink = 1;
	stbuf->st_atime = cur->st.st_atime;
	stbuf->st_mtime = cur->st.st_mtime;
	stbuf->st_ctime = cur->st.st_ctime;
	stbuf->st_size = cur->st.st_size;
	return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("***Debug readdir path : %s, fi : %ld\n", path, fi->fh);
	(void) offset;
    (void) fi;
	DirTree *cur, *tmp;

	cur=getFileDes(&Fd, fi->fh);
	if(cur==NULL) {
		puts("***readdir noent end");
		return -EBADF;
	}

	Debug_showDT(cur);

	if(cur->st.st_mode&__S_IFREG) {
		puts("***readdir  not dir end");
		return -ENOTDIR;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	tmp=cur->child;
	while(tmp!=NULL){
		filler(buf, tmp->name, NULL, 0);
		tmp=tmp->next;
	}

	puts("***readdir End");
	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	printf("***Debug Open : %s, %ld\n", path, fi->fh);
	int open_mode;
	int file_mode;
	struct fuse_context *tmp=fuse_get_context();

	DirTree *cur=findWithPath(path);
	if(cur==NULL){
		puts("***open ENOENT");
		return ENOENT;
	} 
	file_mode=cur->st.st_mode;
	if(file_mode&__S_IFDIR){
		puts("***open not reg edd");
		return -EISDIR;
	}

	if((fi->flags&O_ACCMODE)==O_RDONLY) open_mode=R_OK;
	else if((fi->flags&O_ACCMODE)==O_WRONLY) open_mode=W_OK;
	else open_mode=W_OK | R_OK;

	fi->fh=addFileDes(&Fd, cur);
	puts("***open end");
	return 0;
}

static int hello_opendir(const char *path, struct fuse_file_info *fi){
	DirTree *cur;
	
	cur=findWithPath(path);
	if(cur==NULL) {
		return -ENOENT;
	}
	Debug_showDT(cur);
	if(cur->st.st_mode&__S_IFREG){
		return -ENOTDIR;
	}
	if(!(cur->st.st_mode&R_OK)){
		return -EACCES;
	}

	fi->fh=addFileDes(&Fd, cur);
	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("***Debug Read : %s, size : %ld, FD : %ld\n", path, size, fi->fh);
	DirTree *cur;
	size_t len;
	cur=getFileDes(&Fd, fi->fh);
	if(cur==NULL){	
		puts("*** read end");
		return -ENOENT;
	} 

	Debug_showDT(cur);

	len=cur->st.st_size;
	printf("***len : %ld, offset : %ld\n", len, offset);
	if(offset < len) {
		if((offset+size) > len)
				size = len - offset;
		memcpy(buf, cur->data+offset, size);
	}
	else 
		size = 0;
	puts("*** read end");
	return size;
}

static struct fuse_operations hello_oper = {
	.getattr		= hello_getattr,
	.readdir 		= hello_readdir,
	.open           = hello_open,
	.read 			= hello_read,
	.opendir       =hello_opendir,
};

void init_global(){
	Root=(DirTree*)malloc(sizeof(DirTree));
	hello=(DirTree*)malloc(sizeof(DirTree));
	char *tmp="Hello World!\n";
	int len=strlen(tmp);
	hello->child=hello->next=NULL;
	hello->data=(char*)malloc(sizeof(char)*(len+1));
	strcpy(hello->data, tmp);
	hello->name="hello";
	hello->parent=Root;
	hello->st.st_atime=hello->st.st_ctime=hello->st.st_mtime=time(NULL);
	hello->st.st_uid=getuid();
	hello->st.st_gid=getgid();
	hello->st.st_mode=__S_IFREG|0777;
	hello->st.st_size=len;

	Root->name="/";
	Root->data=NULL;
	Root->next=Root->parent=NULL;
	Root->child=hello;
	Root->st.st_uid=getuid();
	Root->st.st_gid=getgid();
	Root->st.st_mode=__S_IFDIR|0755;

	Root->st.st_atime=Root->st.st_ctime=Root->st.st_mtime=time(NULL);
	Root->st.st_size=0;

	Fd.idx=0;
}

int main(int argc, char *argv[])
{
	init_global();	
	printf("Hell FUSE file system started. \n");
	return fuse_main(argc, argv, &hello_oper, NULL);	
}