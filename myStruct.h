#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>

typedef struct stat status;
typedef struct DirectoryTree{
    status st;
    char *name, *data;
    struct DirectoryTree *child, *next, *parent;
} DirTree;

typedef struct fileDescriptor{
    DirTree* ptr_Tree[30];
    uint64_t idx;
} FileDes;



int pushChild(DirTree *pa_dt, DirTree *ch_dt){
    if(pa_dt->child==NULL){
        pa_dt->child=ch_dt;
        ch_dt->parent=pa_dt;
        return -1;
    }
    DirTree *cur=pa_dt->child;  
    while(cur->next!=NULL)
        cur=cur->next;
    cur->next = ch_dt;

    return 0;
}

int addFileDes(FileDes *fd, DirTree *ptr){
    if(fd->idx>=30) return -EMFILE;
    fd->ptr_Tree[fd->idx]=ptr;
    fd->idx=fd->idx+1;
    return (int)fd->idx-1;
}

DirTree* getFileDes(FileDes *fd, uint64_t idx){
    if(idx>fd->idx) return NULL;
    if(idx<1 || idx>30) return NULL;
    return fd->ptr_Tree[idx];
}

DirTree* popFileDes(FileDes *fd, uint64_t idx){
    DirTree *deleted=NULL;
    if(idx>fd->idx || idx<2 || idx>30) return NULL;
    deleted=fd->ptr_Tree[idx];
    for(int i=idx;i<fd->idx-1;++i){
        fd->ptr_Tree[i]=fd->ptr_Tree[i+1];
    }
    --fd->idx;
    return deleted;
}

void Debug_showDT(DirTree *dt){
    if(dt==NULL) puts("$$$DirTree is NULL ptr");
    printf("$$$name : %s\n", dt->name);
    printf("$$$child : %s, next : %s, parent : %s\n", 
        dt->child?dt->child->name:"null", dt->next?dt->next->name:"null", dt->parent?dt->parent->name:"null");
    printf("$$$Data : %s length : %ld\n", dt->data?dt->data:"null", dt->st.st_size);
}