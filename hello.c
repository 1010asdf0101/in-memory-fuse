#define FUSE_USE_VERSION 26
#define HAVE_SETXATTR 0

#include <string.h>
#include <fcntl.h>
#include "myStruct.h"
#include <pwd.h>

DirTree *Root;
DirTree *hello;
FileDes Fd;

DirTree *findWithPath(const char *path){
	DirTree *cur;
	char *tmp;
	char *t_path=(char*)malloc(sizeof(char)*(strlen(path)+1));
	int done=0;
	if(path==NULL) return NULL;
	strcpy(t_path, path);

	tmp = strtok(t_path, "/");
	if(tmp==NULL) return Root;
	
	cur=Root->child;

	while(tmp != NULL && cur!=NULL)
	{
		while(cur != NULL){
			if(strcmp(cur->name, tmp)==0){
				done=1;
				break;
			}
			cur=cur->next;
		}
		if(done){
			tmp=strtok(NULL, "/");
			if(tmp==NULL) {
				free(t_path);
				return cur;
			}
			cur=cur->child;
		}
	}
	free(t_path);
	return NULL;
}

DirTree *findParent(const char *path){
	size_t len=strlen(path);
	char *last, *t_path;
	DirTree* res;

	if(len==0 || len==1) return NULL;
	t_path=(char*)malloc(sizeof(char)*(len+1));
	strncpy(t_path, path, len);
	last=strrchr(t_path, '/');
	*last='\0';
	res=findWithPath(t_path);
	free(t_path);
	return res;
}

unsigned int checkPermission(unsigned int *id, unsigned int fileMode, unsigned int mode){
	struct fuse_context *fc=fuse_get_context();
	if(id[0]==fc->uid){
		if((mode&R_OK) && !(fileMode&S_IRUSR)) {
			return -EACCES;
		}
		if((mode&W_OK)&&!(fileMode&S_IWUSR)) {
			return -EACCES;
		}
		if((mode&X_OK) && !(fileMode&S_IXUSR)) {
			return -EACCES;
		}
	}
	else if(id[1]==fc->gid){
		if((mode&R_OK) && !(fileMode&S_IRGRP)) {
			return -EACCES;
		}
		if((mode&W_OK)&&!(fileMode&S_IWGRP)) {
			return -EACCES;
		}
		if((mode&X_OK) && !(fileMode&S_IXGRP)) {
			return -EACCES;
		}
	}
	else{
		if((mode&R_OK) && !(fileMode&S_IROTH)) {
			return -EACCES;
		}
		if((mode&W_OK)&& !(fileMode&S_IWOTH)) {
			return -EACCES;
		}
		if((mode&X_OK) && !(fileMode&S_IXOTH)) {
			return -EACCES;
		}
	}
	return 0;
}
static int hello_getattr(const char *path, struct stat *stbuf){
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

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	DirTree *cur, *tmp;

	cur=getFileDes(&Fd, fi->fh);
	if(cur==NULL) {
		return -EBADF;
	}
	if(cur->st.st_mode&__S_IFREG) {
		return -ENOTDIR;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	tmp=cur->child;
	while(tmp!=NULL){
		filler(buf, tmp->name, NULL, 0);
		tmp=tmp->next;
	}
	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi) {
	mode_t open_mode, file_mode;
	unsigned int id[2];
	int err=0;

	DirTree *cur=findWithPath(path);
	if(cur==NULL){
		return ENOENT;
	} 
	file_mode=cur->st.st_mode;
	if(file_mode&__S_IFDIR){
		return -EISDIR;
	}
	id[0]=cur->st.st_uid;
	id[1]=cur->st.st_uid;


	if((fi->flags&O_ACCMODE)==O_RDONLY) open_mode=R_OK;
	else if((fi->flags&O_ACCMODE)==O_WRONLY) open_mode=W_OK;
	else open_mode=W_OK | R_OK;

	err=checkPermission(id,file_mode, open_mode);
	if(err<0) return err;

	fi->fh=addFileDes(&Fd, cur);
	cur->st.st_atime=time(NULL);
	return 0;
}

static int hello_opendir(const char *path, struct fuse_file_info *fi){
	DirTree *cur;
	int res;
	cur=findWithPath(path);
	if(cur==NULL) {
		return -ENOENT;
	}
	if(cur->st.st_mode&__S_IFREG){
		return -ENOTDIR;
	}
	if(!(cur->st.st_mode&R_OK)){
		return -EACCES;
	}
	res=addFileDes(&Fd, cur);
	if(res<0) return res;
	fi->fh=res;
	cur->st.st_atime=time(NULL);
	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	DirTree *cur;
	size_t len;
	cur=getFileDes(&Fd, fi->fh);
	if(cur==NULL){	
		return -ENOENT;
	} 
	
	len=cur->st.st_size;
	if(offset < len) {
		if((offset+size) > len)
				size = len - offset;
		memcpy(buf, cur->data+offset, size);
	}
	else 
		size = 0;
	cur->st.st_atime=time(NULL);
	return size;
}

static int hello_release(const char *path, struct fuse_file_info *fi){
	deleteFileDes(&Fd, fi->fh);
	return 0;
}

static int hello_write(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	DirTree* cur;
	char *tmp;
	size_t diff;
	time_t t=time(NULL);

	cur=getFileDes(&Fd, fi->fh);
	if(cur==NULL){
		return -EBADF;
	}

	diff=size-cur->st.st_size;

	tmp=cur->data;

	cur->data=(char*)malloc(size);
	strncpy(cur->data+offset, buf, size);

	cur->st.st_size=size;
	updateParent(cur->parent, diff, t);
	cur->st.st_atime=cur->st.st_mtime=cur->st.st_ctime=t;
	free(tmp);
	return size;
}

static int hello_truncate(const char *path, off_t offset, struct fuse_file_info *fi){
	DirTree *cur;
	mode_t file_mode;
	size_t st_size, diff;
	time_t t=time(NULL);
	char *tmp;

	cur=findWithPath(path);
	if(cur==NULL) return -ENOENT;

	file_mode=cur->st.st_mode;
	if(file_mode&__S_IFDIR)	return -EISDIR;
	if(!(file_mode&W_OK)) return -EACCES;
	st_size=cur->st.st_size;
	diff=offset-st_size;
	tmp=(char*)malloc(offset);
	if(st_size>offset) strncpy(tmp, cur->data, offset);
	else{
		memset(tmp, 0, offset);
		strncpy(tmp, cur->data, st_size);
	}
	cur->data=tmp;
	cur->st.st_size=offset;
	updateParent(cur->parent, diff, t);
	cur->st.st_atime=cur->st.st_mtime=cur->st.st_ctime=t;
	return 0;
}

static int hello_releasedir(const char *path, struct fuse_file_info *fi){
	deleteFileDes(&Fd, fi->fh);
	return 0;
}

static int hello_mknod(const char *path, mode_t mode, dev_t dev){
	DirTree* cur, *par;
	mode_t p_mode;
	unsigned int id[2], err=0;
	cur=findWithPath(path);
	if(cur!=NULL) return -EEXIST;
	par=findParent(path);
	if(par==NULL) return -ENOENT;
	p_mode=par->st.st_mode;
	if(p_mode&__S_IFREG) return -ENOTDIR;

	id[0]=par->st.st_uid;
	id[1]=par->st.st_gid;
	err=checkPermission(id, p_mode, W_OK);
	if(err<0) return err;
	cur=newDirTree(par, strrchr(path, '/')+1, NULL, mode);
	pushChild(par, cur);
	return err;
}

static int hello_mkdir(const char *path, mode_t mode){
	DirTree* cur, *par;
	mode_t p_mode;
	unsigned int id[2], err=0;
	cur=findWithPath(path);
	if(cur!=NULL) return -EEXIST;
	par=findParent(path);
	if(par==NULL) return -ENOENT;
	p_mode=par->st.st_mode;
	if(p_mode&__S_IFREG) return -ENOTDIR;
	
	id[0]=par->st.st_uid;
	id[1]=par->st.st_gid;
	err=checkPermission(id, p_mode, W_OK);
	if(err<0) return err;
	cur=newDirTree(par, strrchr(path, '/')+1, NULL, __S_IFDIR|mode);
	pushChild(par, cur);
	return err;
}

static int hello_unlink(const char*path){
	DirTree *cur;
	mode_t mod;
	int err, id[2];
	cur=findWithPath(path);
	if(cur==NULL) return -ENOENT;

	for(int i=0;i<30;i++){
		if(Fd.ptr_Tree[i]==cur)
			return -EBUSY;
	}

	mod=cur->st.st_mode;
	if(mod & __S_IFDIR) return -EISDIR;
	id[0]=cur->st.st_uid;
	id[1]=cur->st.st_gid;
	err=checkPermission(id, mod, W_OK);
	if(err<0) return err;
	freeNode(cur);
	return 0;
}

static int hello_rmdir(const char *path){
	DirTree *cur;
	mode_t mod;
	int err, id[2];
	cur=findWithPath(path);
	if(cur==NULL) return -ENOENT;
	if(cur->next!=NULL) return -ENOTEMPTY;
	for(int i=0;i<30;i++){
		if(Fd.ptr_Tree[i]==cur)
			return -EBUSY;
	}

	mod=cur->st.st_mode;
	if(mod & __S_IFREG) return -ENOTDIR;
	id[0]=cur->st.st_uid;
	id[1]=cur->st.st_gid;
	err=checkPermission(id, mod, W_OK);
	if(err<0) return err;
	freeNode(cur);
	return 0;
}

static int hello_chmod(const char* path, mode_t mode, struct fuse_file_info *fi){
	DirTree *cur;
	int id[2];
	cur=findWithPath(path);
	if(cur==NULL) return -ENOENT;
	
	id[0]=cur->st.st_uid;
	id[1]=cur->st.st_gid;
	if(checkPermission(id, cur->st.st_mode, W_OK)>=0){
		mode_t mask=0777;
		mask=~mask;
		cur->st.st_mode=(cur->st.st_mode&mask)|mode;
		cur->st.st_atime=cur->st.st_ctime=time(NULL);
		return 0;
	}
	return -EACCES;
}

static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
	.readdir = hello_readdir,
	.open = hello_open,
	.read = hello_read,
	.opendir = hello_opendir,
	.release = hello_release,
	.write = hello_write,
	.truncate = hello_truncate,
	.releasedir = hello_releasedir,
	.mknod = hello_mknod,
	.mkdir = hello_mkdir,
	.unlink = hello_unlink,
	.rmdir = hello_rmdir,
	.chmod = hello_chmod,
};

void init_global(){
	Root=(DirTree*)malloc(sizeof(DirTree));
	hello=(DirTree*)malloc(sizeof(DirTree));
	char *tmp="Hello World!\n";
	char *name="hello";
	int len=strlen(tmp);
	hello->child=hello->next=NULL;

	hello->data=(char*)malloc(sizeof(char)*(len+1));
	strcpy(hello->data, tmp);
	
	hello->name=(char*)malloc(strlen(name)+1);
	strcpy(hello->name, name);

	hello->parent=Root;
	hello->st.st_atime=hello->st.st_ctime=hello->st.st_mtime=time(NULL);
	hello->st.st_uid=getuid();
	hello->st.st_gid=getgid();
	hello->st.st_mode=__S_IFREG|0777;
	hello->st.st_size=len+1;

	name="/";

	Root->name=(char*)malloc(strlen(name)+1);
	strcpy(Root->name, name);

	Root->data=NULL;
	Root->next=Root->parent=NULL;
	Root->child=hello;
	Root->st.st_uid=getuid();
	Root->st.st_gid=getgid();
	Root->st.st_mode=__S_IFDIR|0755;

	Root->st.st_atime=Root->st.st_ctime=Root->st.st_mtime=time(NULL);
	Root->st.st_size=0;

	Fd.cnt=0;
	for(int i=0;i<30;++i) Fd.ptr_Tree[i]=NULL;
}

int main(int argc, char *argv[])
{
	init_global();	
	printf("Hell FUSE file system started. \n");
	return fuse_main(argc, argv, &hello_oper, NULL);	
}