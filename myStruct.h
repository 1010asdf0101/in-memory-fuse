#include <sys/types.h>
#include <errno.h>
#include <fuse.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct stat status;
typedef struct DirectoryTree{
    status st;
    char *name, *data;
    struct DirectoryTree *child, *next, *parent;
} DirTree;

typedef struct fileDescriptor{
    DirTree* ptr_Tree[30];
    int cnt;
} FileDes;


DirTree* newDirTree(DirTree *par, char *name, char *data, mode_t mod){
    DirTree* tmp= (DirTree* ) malloc( sizeof(DirTree) );
    int len=strlen(name)+1;
    struct fuse_context *fc=fuse_get_context();
    tmp->parent=par;
    tmp->next=tmp->child=NULL;

    tmp->name=(char*)malloc(len);
    strncpy(tmp->name, name, len-1);
    tmp->name[len-1]='\0';

    if(data==NULL) tmp->data=NULL;
    else{len=strlen(data)+1;
        tmp->data=(char*)malloc(len);   
        strncpy(tmp->data, data, len-1);
        tmp->data[len-1]='\0';
    }
    tmp->st.st_mode=mod;
    tmp->st.st_atime=tmp->st.st_ctime=tmp->st.st_mtime=time(NULL);
    tmp->st.st_size=len;
    tmp->st.st_uid=fc->uid;
    tmp->st.st_gid=fc->gid;
    return tmp;
}

int pushChild(DirTree *pa_dt, DirTree *ch_dt){
    if(pa_dt->child==NULL){
        pa_dt->child=ch_dt;
        ch_dt->parent=pa_dt;
        return 0;
    }
    DirTree *cur=pa_dt->child;  
    while(cur->next!=NULL)
        cur=cur->next;
    cur->next = ch_dt;
    ch_dt->parent=pa_dt;
    return 0;
}

int freeNode(DirTree *dt){
    DirTree *p_dt=dt->parent;
    DirTree *cur;
    if(p_dt->child==dt){
        p_dt->child=NULL;
        free(dt->name);
        free(dt->data);
        free(dt);
        return 0;
    }
    cur=p_dt->child;
    while(cur->next!=dt)
        cur=cur->next;
    cur->next=dt->next;
    free(dt->name);
    free(dt->data);
    free(dt);
    return 0;
}

uint64_t addFileDes(FileDes *fd, DirTree *ptr){
    int empty_idx;
    if(fd->cnt>=30) return -EMFILE;
    for(int i=0;i<30;i++) {
        if(fd->ptr_Tree[i]==NULL){
            empty_idx=i;
         break;
        }
    }
    fd->ptr_Tree[empty_idx]=ptr;
    ++fd->cnt;
    return empty_idx;
}

DirTree* getFileDes(FileDes *fd, uint64_t idx){
    if(idx<0 || idx>30) return NULL;
    return fd->ptr_Tree[idx];
}

DirTree* deleteFileDes(FileDes *fd, uint64_t idx){
    DirTree *deleted;
    if(idx>=30) return NULL;
    deleted=fd->ptr_Tree[idx];
    fd->ptr_Tree[idx]=NULL;
    --fd->cnt;
    return deleted;
}

void Debug_showDT(DirTree *dt){
    if(dt==NULL) {
        puts("$$$DirTree is NULL ptr");
        return;
    }
    printf("$$$name : %s\n", dt->name);
    printf("$$$child : %s, next : %s, parent : %s\n", 
        dt->child?dt->child->name:"null", dt->next?dt->next->name:"null", dt->parent?dt->parent->name:"null");
    printf("$$$Data : %s, Length : %ld, MODE : %o\n", dt->data?dt->data:"null", dt->st.st_size, dt->st.st_mode);

}