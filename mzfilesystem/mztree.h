/*
  Copyright (C) JAEHYUK CHO
  All rights reserved.
  Code by JaeHyuk Cho <mailto:minzkn@minzkn.com>
*/

#if !defined(__def_mztree_header_mztree_h__)
# define __def_mztree_header_mztree_h__ "mztree.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#if !defined(mztree_data_node_t)
# define def_mztree_block_size (4 << 10)
typedef struct mztree_data_node_ts {
    struct mztree_data_node_ts *m_next;
    size_t m_alloc_size;
    
    void *m_data;
    size_t m_size;
}__mztree_data_node_t;
# define mztree_data_node_t __mztree_data_node_t
#endif

#if !defined(mztree_node_t)
typedef struct mztree_node_ts {
    struct mztree_node_ts *m_next;
    struct mztree_node_ts *m_sub;
    
    struct stat m_statbuffer;

    char *m_name;
    mztree_data_node_t *m_data_node;
}__mztree_node_t;
# define mztree_node_t __mztree_node_t
#endif

#if !defined(mztree_t)
typedef struct mztree_ts {
    mztree_node_t *m_node;

    void *m_ftpd_context;
}__mztree_t;
# define mztree_t __mztree_t
#endif

#if !defined(__def_mztree_source_mztree_c__)
extern mztree_t *mztree_new(void);
extern mztree_t *mztree_delete(mztree_t *s_fs);

extern int mztree_view(mztree_t *s_fs, const char *s_pathname, int s_with_next);

extern mztree_node_t *mztree_create(mztree_t *s_fs, const char *s_pathname, mode_t s_mode);
extern mztree_node_t *mztree_search(mztree_t *s_fs, const char *s_pathname);
extern mztree_node_t *mztree_remove(mztree_t *s_fs, const char *s_pathname);
extern mztree_node_t *mztree_rename(mztree_t *s_fs, const char *s_oldpathname, const char *s_newpathname);
extern ssize_t mztree_read(mztree_node_t *s_node, void *s_data, size_t s_size, off_t s_offset);
extern ssize_t mztree_write(mztree_node_t *s_node, const void *s_data, size_t s_size, off_t s_offset);
#endif

#endif

/* End of header */
