#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#define MAX_PATH_SIZE 4096
#define MAX_NAME_SIZE 255

typedef struct inode {
	uid_t 	uid;
	gid_t 	gid;
	mode_t	mode;
	time_t	ctime;
	time_t 	atime;
	time_t 	mtime;
	char	*data;
	off_t	size;
} Inode;

typedef struct dentry {
	char	*name;
	Inode	*node;
	struct dentry	*child;
	struct dentry	*next;
	struct dentry	*prev;
}Dentry;

//global variable
Dentry *root;

//about I-node function
void fillInNode(mode_t mode, uid_t uid, gid_t gid, time_t atime, time_t mtime, char *data, Inode* node)
{
	node->mode = mode;
	node->uid = uid;
	node->gid = gid;
	node->atime = atime;
	node->mtime = mtime;
	
	node->data = data;
	node->size = node->data ? strlen(node->data) : 0;
}

Inode *createInode(mode_t mode, uid_t uid, gid_t gid)
{
	Inode* Itemp = (Inode*)malloc(sizeof(Inode));
	
	fillInNode(mode, uid, gid, time(NULL), time(NULL), NULL, Itemp);
	Itemp->ctime = time(NULL);
	
	return Itemp;
}

void clearInode(Inode *node)
{
	node->uid = 0;
	node->gid = 0;
	node->mode = 0;
	node->ctime = 0;
	node->atime = 0;
	node->mtime = 0;
	free(node->data);
	node->size = 0;
}		

//about D.entry function
void insertDentry(Dentry *parent, Dentry *child)
{
	Dentry *childFirst;
	
	if(parent->child != NULL) {
		childFirst = parent->child;
		
		while(childFirst->next != NULL)
			childFirst = childFirst->next;
		childFirst->next= child;
		child->prev = childFirst;
	}
	else
		parent->child = child;
}

void eraseDentry(Dentry *parent, Dentry *child)
{
	Dentry *childFirst;
	
	childFirst = parent->child;

	if(childFirst == child)
	{
		if(child->next != NULL)
			parent->child = child->next;
		else
			parent->child = NULL;
	}
	else
	{
		if(child->next)
		{
			child->prev->next = child->next;
			child->next->prev = child->prev;
		}
		else
			child->prev->next = NULL;
	}
}

Dentry *createDentry(char *name, mode_t mode, uid_t uid, gid_t gid)
{
	Dentry *Dtemp = (Dentry*)malloc(sizeof(Dentry));
	Inode *Itemp = createInode(mode, uid, gid);
	
	Dtemp->name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(Dtemp->name,name);
	Dtemp->child = NULL;
	Dtemp->next = NULL;
	Dtemp->prev = NULL;
	
	Dtemp->node = Itemp;
	
	return Dtemp;
}

void clearDentry(Dentry *entry)
{
	clearInode(entry->node);
	if(entry->node != NULL)
		free(entry->node);
	
	entry->name = "";

	if(entry->prev)
	{
		if(entry->next)
			entry->prev->next = entry->next;
		else
			entry->prev->next = NULL;
	}
	if(entry->next)
	{
		if(entry->prev)
			entry->next->prev = entry->prev;
		else
			entry->next->prev = NULL;
	}
}

void removeDentry(Dentry *entry)
{
	clearDentry(entry);
	free(entry);	
}

Dentry *findDentry(const char *path)
{
	Dentry *current;
	char *temp = (char*)malloc(sizeof(char)*(strlen(path)+1));
	char *dirName;
	printf("***Debug findWithPath Path : %s\n", path);
	strcpy(temp, path);

	dirName = strtok(temp+1, "/");
	current = root;
	
	while(dirName != NULL && current != NULL)
	{
		if(current == root)
			current = current->child;
			
		while(current != NULL)
		{
			if(strcmp(dirName, current->name) == 0)
			{
				dirName = strtok(NULL, "/");
				if(dirName == NULL)
					break;

				current = current->child;
				
				
				break;	
			}	
			current = current->next;
		}	
	}
	free(temp);

	return current;
}
Dentry *findParentDentry(const char *path)
{
	Dentry *parent;
	char *temp = (char*)malloc(sizeof(char)*(strlen(path)+1));
	char *slice;
	
	strcpy(temp,path);
	
	slice = strrchr(temp, '/');
	*(slice+1) = '\0';
	
	parent = findDentry(temp);
	
	free(temp);
	
	return parent;
}

//about validation function
Dentry *hasChild(Dentry *entry)
{
	return entry->child;	
}

int getMode(struct fuse_file_info *fi)
{
	int mode;
	if((fi -> flags & O_ACCMODE) == O_WRONLY) 
		mode = W_OK;
	else if((fi -> flags & O_ACCMODE) == O_RDONLY) 
		mode = R_OK;
	else 
		mode = W_OK | R_OK;

	return mode;		
}

int checkEACCES(Inode *node, int mode)
{
	int res = 0;
	mode_t permission = node->mode;

	if(node->uid == fuse_get_context()->uid)
	{
		if(mode & R_OK) res |= (permission & S_IRUSR) ^ S_IRUSR;
        if(mode & W_OK) res |= (permission & S_IWUSR) ^ S_IWUSR;
        if(mode & X_OK) res |= (permission & S_IXUSR) ^ S_IXUSR;
	}
	else if(node->gid == fuse_get_context()->gid)
	{
		if(mode & R_OK) res |= (permission & S_IRGRP) ^ S_IRGRP;
        if(mode & W_OK) res |= (permission & S_IWGRP) ^ S_IWGRP;
        if(mode & X_OK) res |= (permission & S_IXGRP) ^ S_IXGRP;
	}
	else
	{
		if(mode & R_OK) res |= (permission & S_IROTH) ^ S_IROTH; 
        if(mode & W_OK) res |= (permission & S_IWOTH) ^ S_IWOTH;
        if(mode & X_OK) res |= (permission & S_IXOTH) ^ S_IXOTH;

	}
	
	if(res) 
        return 1;
    else
        return 0;
}

int checkEEXIST(const char *path)
{
	if(findDentry(path) != NULL)
		return 1;
	else
		return 0;
}

int checkENAMETOOLONG(const char *name)
{
	int len = strlen(name);
	if(strchr(name+1 , '/') != NULL)
	{
		if(len > MAX_PATH_SIZE)
			return 1;
		else
			return 0;			
	}
	else
	{
		if(len > MAX_NAME_SIZE)
			return 1;
		else
			return 0;
	}
}

int checkENOTDIR(const mode_t mode)
{
	if(!S_ISDIR(mode))
		return 1;
	else
		return 0;
}

int checkEISDIR(const mode_t mode)
{
	if(S_ISDIR(mode))
		return 1;
	else
		return 0;
}

int checkENOENT(const char *path, int isParent)
{
	if(isParent)
	{
		if(findParentDentry(path) == NULL)
			return 1;
		else
			return 0;
	}
	else
	{
		if(findDentry(path) == NULL)
			return 1;
		else
			return 0;
	}
}

int checkEPERM(Inode *node)
{
	uid_t userUid = fuse_get_context()->uid;

	if(node->uid == userUid)
		return 0;
	else
		return 1;
}

int checkENOTEMPTY(Dentry *entry)
{
	if(entry->child != NULL)
		return 1;
	else
		return 0;
}

static int kdm_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	Dentry* current;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

	current = findDentry(path);
	memset(stbuf,0,sizeof(struct stat));

	stbuf->st_mode = current->node->mode;
	stbuf->st_uid = current->node->uid;
	stbuf->st_gid = current->node->gid;


	if(S_ISDIR(stbuf->st_mode))
		stbuf->st_nlink	= 2;
	else
		stbuf->st_nlink = 1;
	stbuf->st_atime = current->node->atime;
	stbuf->st_mtime = current->node->mtime;
	stbuf->st_ctime = current->node->ctime;
	stbuf->st_size = current->node->size;

	return res;
}

static int kdm_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
    (void) fi;
	Dentry *child, *current;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

		current = findDentry(path);
	if(checkENOTDIR(current->node->mode))
		return -ENOTDIR;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	if(current->child != NULL)
	{
		child = current->child;
		filler(buf, child->name, NULL, 0);

		while(child->next)
		{
			child = child->next;
			filler(buf, child->name, NULL, 0);
		}
	}

	return 0;
}

static int kdm_open(const char *path, struct fuse_file_info *fi)
{
	Dentry *entry;
	int mode;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

		entry = findDentry(path);

	if(checkEISDIR(entry->node->mode))
		return -EISDIR;

	mode = getMode(fi);
	if(checkEACCES(entry->node, mode) != 0)
		return -EACCES;

	return 0;
}

static int kdm_release(const char *path, struct fuse_file_info *fi)
{
	return 0;
}

static int kdm_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	Dentry *entry;
	size_t len;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

		entry = findDentry(path);

	len = entry->node->size;

	if(offset < len)
	{
		if((offset+size) > len)
				size = len - offset;

		memcpy(buf, entry->node->data+offset, size);
	}
	else
		size = 0;

	return size;
}

static int kdm_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) 
{
	Dentry *entry;
	size_t len;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

	entry = findDentry(path);

	entry->node->data = (char*) malloc(size);
	memcpy(entry->node->data+offset, buf, size);
	entry->node->size = size;
	
	return size;
}

static int kdm_mknod(const char *path, mode_t mode, dev_t dev)
{
	Dentry *parent, *entry;
	char *name;
	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 1))
		return -ENOENT;
	parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	if(checkEEXIST(path))
		return -EEXIST;
	
	name = strrchr(path, '/')+1;
	if(checkENAMETOOLONG(name))
		return -ENAMETOOLONG;

	entry = createDentry(name, mode, fuse_get_context()->uid, fuse_get_context()->gid);
	insertDentry(parent, entry);

	return 0;
}

static int kdm_mkdir(const char *path, mode_t mode)
{
	Dentry *parent, *entry;
	char *name;
	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 1))
		return -ENOENT;

		parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	if(checkEEXIST(path))
		return -EEXIST;

	name = strrchr(path, '/')+1;
	if(checkENAMETOOLONG(name))
		return -ENAMETOOLONG;

	entry = createDentry(name, __S_IFDIR | mode, fuse_get_context()->uid, fuse_get_context()->gid);
	insertDentry(parent, entry);

	return 0;
}

static int kdm_utime(const char *path, struct utimbuf *times)
{
	Dentry *entry;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;

	entry = findDentry(path);
	
	if(times == NULL)
	{
		if(checkEPERM(entry->node))
			return -EPERM;
		entry->node->atime = entry->node->mtime = time(NULL);
	}
	else
	{
		if(checkEACCES(entry->node, W_OK))
			return -EACCES;
		if(times->actime)
			entry->node->atime;
		if(times->modtime)
			entry->node->mtime;
	}

	return 0;
}

static int kdm_unlink(const char *path)
{
	Dentry *parent, *entry;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;	
	entry = findDentry(path);
	if(checkEISDIR(entry->node->mode))
		return -EISDIR;
	if(checkEACCES(entry->node, W_OK))
		return -EACCES;

	parent = findParentDentry(path);
	eraseDentry(parent, entry);
	removeDentry(entry);

	return 0;
}

static int kdm_rmdir(const char *path)
{
	Dentry *parent, *entry;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;	
	entry = findDentry(path);
	if(checkENOTDIR(entry->node->mode))
		return -ENOTDIR;
	if(checkEACCES(entry->node, W_OK))
		return -EACCES;
	if(checkENOTEMPTY(entry))
		return -ENOTEMPTY;

	parent = findParentDentry(path);
	eraseDentry(parent, entry);
	removeDentry(entry);

	return 0;
}

static int kdm_truncate(const char *path, off_t length)
{	
	void *newBuf;
	Dentry *parent, *entry;

	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	entry = findDentry(path);
	if(checkENOENT(path, 0))
		return -ENOENT;
	if(checkEISDIR(entry->node->mode))
		return -EISDIR;
	if(checkEACCES(entry->node, W_OK))
		return -EACCES;

	if(entry->node->size == length)
		return 0;

	newBuf = (char*)realloc(entry->node->data, length);
	if(!newBuf && length)
		return -ENOMEM;

	if(entry->node->size < length)
		memcpy(newBuf + entry->node->size, 0, length-entry->node->size);
	
	entry->node->data = newBuf;
	entry->node->size = length;
	
	return 0;
}

static int kdm_chmod(const char *path, mode_t mode)
{
	Dentry *parent, *entry;
	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	if(checkENOENT(path, 0))
		return -ENOENT;
	entry = findDentry(path);
	if(checkEPERM(entry->node))
		return -EPERM;
	if(checkEACCES(entry->node, W_OK))
		return -EACCES;

	entry->node->mode = mode;

	return 0;
}

static int kdm_chown(const char *path, uid_t owner, gid_t group)
{
	Dentry *parent, *entry;
	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	if(checkENOENT(path, 0))
		return -ENOENT;
	entry = findDentry(path);
	if(checkEPERM(entry->node))
		return -EPERM;
	if(checkEACCES(entry->node, W_OK))
		return -EACCES;

	if(owner != -1)
		entry->node->uid = owner;
	if(group != -1)
		entry->node->gid = group;

	return 0;
}

static int kdm_rename(const char *oldpath, const char *newpath)
{
	Dentry *oldEntry, *oldParent, *newParent;
	char *newName;

	oldParent = findParentDentry(oldpath);
	if(checkENOTDIR(oldParent->node->mode))
		return -ENOTDIR;
	if(checkENOENT(oldpath, 0))
		return -ENOENT;
	oldEntry = findDentry(oldpath);
	if(checkEACCES(oldEntry->node, W_OK))
		return -EACCES;

	if(checkENAMETOOLONG(newpath))
		return -ENAMETOOLONG;
	newName = strrchr(newpath,'/')+1;
	if(checkENAMETOOLONG(newName))
		return -ENAMETOOLONG;
	newParent = findParentDentry(newpath);
	if(checkENOTDIR(newParent->node->mode))
		return -ENOTDIR;

	if(oldParent != newParent)
	{
		eraseDentry(oldParent, oldEntry);
		insertDentry(newParent, oldEntry);
	}
	if(strcmp(oldEntry->name, newName) != 0)
	{
		oldEntry->name = (char*)realloc(oldEntry->name, sizeof(char)*(strlen(newName)+1));
		strcpy(oldEntry->name, newName);
	}
	
	return 0;
}

static int kdm_access(const char *path, int mode)
{
	Dentry *parent, *entry;

	parent = findParentDentry(path);
	if(checkENOTDIR(parent->node->mode))
		return -ENOTDIR;
	if(checkENAMETOOLONG(path))
		return -ENAMETOOLONG;
	if(checkENOENT(path, 0))
		return -ENOENT;
	entry = findDentry(path);

	if(mode == F_OK)
		return 0;

	return checkEACCES(entry->node, mode);
}

static struct fuse_operations kdm_oper = {
	.getattr		= kdm_getattr,
	.readdir 		= kdm_readdir,
	.open           = kdm_open,
	.release 		= kdm_release,
	.read 			= kdm_read,
	.write 			= kdm_write,
	.mknod 			= kdm_mknod,
	.mkdir			= kdm_mkdir,
	.utime 			= kdm_utime,
	.unlink 		= kdm_unlink,
	.rmdir          = kdm_rmdir,
	.truncate		= kdm_truncate,
	.chmod          = kdm_chmod,
	.chown 			= kdm_chown,
	.rename			= kdm_rename,
	.access 		= kdm_access
};

//main
int main(int argc, char *argv[])
{
	printf("Kim Dong Mins' FUSE file system started. \n");
	root = createDentry("/", __S_IFDIR | 0755, getuid(), getgid());

	return fuse_main(argc, argv, &kdm_oper, NULL);
}