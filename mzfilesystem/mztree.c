/*
  Copyright (C) JAEHYUK CHO
  All rights reserved.
  Code by JaeHyuk Cho <mailto:minzkn@minzkn.com>
*/

#if !defined(__def_mztree_source_mztree_c__)
# define __def_mztree_source_mztree_c__ "mztree.c"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mztree.h"

#define def_mztree_do_mode_free_all (0)
#define def_mztree_do_mode_free (1)
#define def_mztree_do_mode_pop_node (2)
#define def_mztree_do_mode_new (3)
#define def_mztree_do_mode_search (4)
#define def_mztree_do_mode_rename (5)

#if !defined(mztree_view_t)
# pragma pack(push,8)
typedef struct mztree_view_ts {
    unsigned int m_flags;

    mztree_t *m_tree;
    mztree_node_t *m_node;

    const char *m_pathname;

    size_t m_level;
    size_t m_max_level;
    size_t m_max_width;
    int m_with_next;
    size_t m_screen_width;

    unsigned char *m_keep_flags;
}__mztree_view_t;
# pragma pack(pop)
# define mztree_view_t __mztree_view_t
#endif

static __inline void *mztree_alloc(size_t s_size);
static __inline void *mztree_zalloc(size_t s_size);
static __inline void *mztree_realloc(void *s_ptr, size_t s_resize);
static __inline void *mztree_free(void *s_ptr);

static int mztree_is_space(int s_character);
static char * mztree_skip_space(const char *s_string);
static char *mztree_get_word_sep_alloc(int s_skip_space, const char *s_sep, const char **s_sep_string);

static mztree_node_t *mztree_do(mztree_node_t **s_node_ptr, const char *s_pathname, int s_do_mode, void *s_argument);

mztree_t *mztree_new(void);
mztree_t *mztree_delete(mztree_t *s_fs);

static void mztree_view_local(mztree_view_t *s_view, mztree_node_t *s_node);
int mztree_view(mztree_t *s_fs, const char *s_pathname, int s_with_next);

mztree_node_t *mztree_create(mztree_t *s_fs, const char *s_pathname, mode_t s_mode);
mztree_node_t *mztree_search(mztree_t *s_fs, const char *s_pathname);
mztree_node_t *mztree_remove(mztree_t *s_fs, const char *s_pathname);
mztree_node_t *mztree_rename(mztree_t *s_fs, const char *s_oldpathname, const char *s_newpathname);
ssize_t mztree_read(mztree_node_t *s_node, void *s_data, size_t s_size, off_t s_offset);
ssize_t mztree_write(mztree_node_t *s_node, const void *s_data, size_t s_size, off_t s_offset);

static __inline void *mztree_alloc(size_t s_size)
{
    return(malloc(s_size));
}

static __inline void *mztree_zalloc(size_t s_size)
{
    void *s_result;

    s_result = malloc(s_size);
    if(s_result == ((void *)0)) {
        /* errno = ENOMEM */
        return((void *)0);
    }

    return(memset(s_result, 0, s_size));
}

static __inline void *mztree_realloc(void *s_ptr, size_t s_resize)
{
    return(realloc(s_ptr, s_resize));    
}

static __inline void *mztree_free(void *s_ptr)
{
    if(s_ptr != ((void *)0)) {
        free(s_ptr);
    }

    return((void *)0);
}

static int mztree_is_space(int s_character)
{
    return(isspace(s_character));
}

static char * mztree_skip_space(const char *s_string)
{
    union {
        const char *m_const_string;
	const unsigned char *m_const_byte_ptr;
	char *m_string;
    }s_const_ptr;

    s_const_ptr.m_const_string = s_string;

    while(mztree_is_space((int)(*(s_const_ptr.m_const_byte_ptr))) != 0) {
        s_const_ptr.m_const_byte_ptr = (const unsigned char *)(&s_const_ptr.m_const_byte_ptr[1]);
    }

    return(s_const_ptr.m_string);
}

static char *mztree_get_word_sep_alloc(int s_skip_space, const char *s_sep, const char **s_sep_string)
{
    char *s_result;

    size_t s_token_size;
    const unsigned char *s_string;
    const unsigned char *s_left;
    const unsigned char *s_right;
    const unsigned char *s_sep_ptr;

    if(s_skip_space == 0) {
        s_right = s_left = s_string = (const unsigned char *)(*(s_sep_string));
        while(*(s_string) != 0u) {
            s_sep_ptr = (const unsigned char *)s_sep;
            while((*(s_string) != *(s_sep_ptr)) && (*(s_sep_ptr) != 0u)) {
                s_sep_ptr++;
            }
            if(*(s_string) == *(s_sep_ptr)) {
                break;
            }
            s_right = (++s_string);
        }
    }
    else {
        s_right = s_left = s_string = (const unsigned char *)mztree_skip_space(*(s_sep_string));
        while(*(s_string) != 0u) {
            s_sep_ptr = (const unsigned char *)s_sep;
            while((*(s_string) != *(s_sep_ptr)) && (*(s_sep_ptr) != 0u)) {
                s_sep_ptr++;
            }
            if(*(s_string) == *(s_sep_ptr)) {
                break;
            }
            if(mztree_is_space((int)(*(s_string))) != 0) {
                s_string++;
            }
            else {
                s_right = (++s_string);
            }
        }
    }
    
    s_token_size = (size_t)(s_right - s_left);
    s_result = (char *)mztree_alloc(s_token_size + ((size_t)1));
    if(s_result != ((char *)0)) {
        if(s_token_size > ((size_t)0)) {
            (void)memcpy((void *)s_result, (const void *)s_left, s_token_size);
        }
        *(((unsigned char *)s_result) + s_token_size) = 0u;
    }

    *(s_sep_string) = (const char *)s_string;

    return(s_result);
}

static mztree_node_t *mztree_do(mztree_node_t **s_node_ptr, const char *s_pathname, int s_do_mode, void *s_argument)
{
    mztree_node_t *s_result, *s_node, *s_temp, *s_trace;
    mztree_data_node_t *s_data_node;
    const char *s_sep_string1;
    const char *s_sep_string2;
    char *s_node_name1, *s_node_name2;

    s_result = (mztree_node_t *)0;
    s_node = (s_node_ptr == ((mztree_node_t **)0)) ? ((mztree_node_t *)0) : (*s_node_ptr);

    switch(s_do_mode) {
        case def_mztree_do_mode_free_all:
	    while(s_node != ((mztree_node_t *)0)) {
	        s_temp = s_node;
		s_node = s_node->m_next;
		(void)mztree_do((mztree_node_t **)(&s_temp->m_sub), s_pathname, s_do_mode, s_argument);
		while(s_temp->m_data_node != ((mztree_data_node_t *)0)) {
		    s_data_node = s_temp->m_data_node;
		    s_temp->m_data_node = s_temp->m_data_node->m_next;
		    s_data_node = (mztree_data_node_t *)mztree_free((void *)s_data_node);
		}
		if(s_temp->m_name != ((char *)0)) {
		    s_temp->m_name = (char *)mztree_free((void *)s_temp->m_name);
		}
		(void)mztree_free((void *)s_temp);
	    }
	    break;
        case def_mztree_do_mode_free:
        case def_mztree_do_mode_pop_node:
        case def_mztree_do_mode_new:
        case def_mztree_do_mode_search:
            s_node_name1 = (char *)0;
            s_node_name2 = (char *)0;
            if(s_pathname != ((const char *)0)) {
		s_sep_string1 = s_pathname;
		do {
		    s_node_name1 = mztree_get_word_sep_alloc(0, "/\\", (const char **)(&s_sep_string1));
		    if(s_sep_string1[0] != '\0') {
		        s_sep_string1 = (const char *)(&s_sep_string1[1]); 
		    }
                    if(s_node_name1 == ((char *)0)) {
		        break;
		    }

                    if(strlen((const char *)s_node_name1) > ((size_t)0u)) {
		        s_sep_string2 = s_sep_string1;
			while(s_sep_string2[0] != '\0') {
		            s_node_name2 = mztree_get_word_sep_alloc(0, "/\\", (const char **)(&s_sep_string2));
		            if(s_sep_string2[0] != '\0') {
		                s_sep_string2 = (const char *)(&s_sep_string2[1]); 
		            }
			    if(s_node_name2 == ((char *)0)) {
			        break;
			    }
                    
		            if(strlen((const char *)s_node_name2) > ((size_t)0u)) {
			        break;
			    }

		            s_node_name2 = (char *)mztree_free((void *)s_node_name2);
			}
			break;
		    }

		    s_node_name1 = (char *)mztree_free((void *)s_node_name1);
		}while(s_sep_string1[0] != '\0');
	    }

	    s_trace = (mztree_node_t *)0;
	    s_temp = s_node;
	    while(s_temp != ((mztree_node_t *)0)) {
                if((s_node_name1 != ((char *)0)) && (s_temp->m_name != ((char *)0))) {
		    if(strcmp(s_node_name1, s_temp->m_name) == 0) {
		        if(s_trace == ((mztree_node_t *)0)) {
			    break;
			}
#if 0L /* BUG ! */			
			while((s_trace->m_next != ((mztree_node_t *)0)) && (s_temp != s_trace->m_next)) {
			    s_trace = s_trace->m_next;
			}
#elif 0L			
			while(s_trace->m_next != ((mztree_node_t *)0)) {
			    s_trace = s_trace->m_next;
			}
#endif			
		        break;
		    }
		}
	        s_trace = s_temp;
		s_temp = s_temp->m_next;
	    }

	    if((s_do_mode == def_mztree_do_mode_new) && (s_temp == ((mztree_node_t *)0)) && (s_node_name1 != ((char *)0))) {
	        s_temp = (mztree_node_t *)mztree_zalloc(sizeof(mztree_node_t));
		if(s_temp != ((mztree_node_t *)0)) {
		    s_temp->m_next = (mztree_node_t *)0;
		    s_temp->m_sub = (mztree_node_t *)0;
		    s_temp->m_name = s_node_name1;
		    s_temp->m_data_node = (mztree_data_node_t *)0;
		    if(s_argument != ((void *)0)) {
		        s_temp->m_statbuffer.st_mode = *((mode_t *)s_argument);
		    }
		    if(s_trace == ((mztree_node_t *)0)) {
		        s_node = s_temp;
		    }
		    else {
		        s_trace->m_next = s_temp;
		    }
		    s_node_name1 = (char *)0;
		}
	    }
	    else if(s_node_name1 == ((char *)0)){
                s_temp = s_node;
	    }

	    if((s_temp != ((mztree_node_t *)0)) && (s_node_name2 != ((char *)0))) {
	        s_result = mztree_do((mztree_node_t **)(&s_temp->m_sub), s_sep_string1, s_do_mode, s_argument);
	    }
	    else if(((s_do_mode == def_mztree_do_mode_free) || (s_do_mode == def_mztree_do_mode_pop_node)) && (s_temp != ((mztree_node_t *)0))) {
	        if(s_trace == ((mztree_node_t *)0)) {
		    s_node = s_temp->m_next;
		}
		else {
		    s_trace->m_next = s_temp->m_next;
		}
		s_temp->m_next = (mztree_node_t *)0;
		if(s_do_mode == def_mztree_do_mode_pop_node) {
		    s_result = s_temp;
		}
		else { /* def_mztree_do_mode_free */
		    s_result = mztree_do((mztree_node_t **)(&s_temp), (const char *)0, def_mztree_do_mode_free_all, s_argument);
		}
	    }
	    else {
	        s_result = s_temp;
	    }

	    s_node_name1 = (char *)mztree_free((void *)s_node_name1);
	    s_node_name2 = (char *)mztree_free((void *)s_node_name2);
	    break;
        case def_mztree_do_mode_rename:
            /* TODO */
	    break;
	default:
	    /* errno = EINVAL */
	    break;
    }

    if(s_node_ptr != ((mztree_node_t **)0)) {
        *s_node_ptr = s_node;
    }

    return(s_result);
}

mztree_t *mztree_new(void)
{
    mztree_t *s_fs;

    s_fs = (mztree_t *)mztree_zalloc(sizeof(mztree_t));
    if(s_fs == ((mztree_t *)0)) {
        return((mztree_t *)0);
    }

    s_fs->m_node = (mztree_node_t *)0;

    s_fs->m_ftpd_context = (void *)0;

    return(s_fs);
}

mztree_t *mztree_delete(mztree_t *s_fs)
{
    (void)mztree_do((mztree_node_t **)(&s_fs->m_node), (const char *)0, def_mztree_do_mode_free_all, (void *)0);

    return((mztree_t *)mztree_free((void *)s_fs));
}

static void mztree_view_local(mztree_view_t *s_view, mztree_node_t *s_node)
{
    size_t s_count;
    int s_with_next;
    size_t s_x;
    int s_have_data;
    int s_scan_mode;
    size_t s_name_size;
    mztree_node_t *s_trace;

    s_with_next = s_view->m_with_next;
    s_view->m_with_next = 1;

    if((s_view->m_flags & (1u << 0)) == 0u) {
        s_scan_mode = 1;
        if(s_view->m_level > s_view->m_max_level) {
            s_view->m_max_level = s_view->m_level;
        }
    }
    else {
        s_scan_mode = 0;
    }
  
    for(s_trace = s_node;s_trace != ((mztree_node_t *)0);s_trace = s_trace->m_next) {
        s_name_size = strlen(s_trace->m_name);
        s_have_data = (s_trace->m_data_node == ((mztree_data_node_t *)0)) ? 0 : 1;
 
        s_x = ((size_t)2u) + (s_view->m_level * ((size_t)7u)) + ((size_t)1u) + ((size_t)6u) + ((size_t)1u) + s_name_size + ((size_t)1u) + ((size_t)1u);
        if(s_scan_mode == 0) {
            if(s_trace == s_view->m_node) {
                (void)fprintf(stdout, " \"/\" <- root (%s)\n", s_view->m_pathname);
            }
            (void)fputs("  ", stdout);
 
            s_view->m_keep_flags[s_view->m_level] = ((s_trace->m_next != ((mztree_node_t *)0)) && (s_with_next != 0)) ? ((unsigned char)1u) : ((unsigned char)2u);
            for(s_count = (size_t)0u;s_count <= s_view->m_level;s_count++) {
                if(s_count < s_view->m_level) {
                    switch(s_view->m_keep_flags[s_count]) {
                        case 0:  (void)fputs(" ", stdout); break;
                        case 1:  (void)fputs("|", stdout); break;
                        case 2:  (void)fputs("`", stdout); s_view->m_keep_flags[s_count] = (unsigned char)0u; break;
                        default: (void)fputs("!", stdout); break;
                    }
                    (void)fputs("      ", stdout); /* 6 char */
                }
                else {
                    switch(s_view->m_keep_flags[s_count]) {
                        case 0:  (void)fputs(" ", stdout); break;
                        case 1:  (void)fputs(")", stdout); break;
                        case 2:  (void)fputs("`", stdout); s_view->m_keep_flags[s_count] = (unsigned char)0u; break;
                        default: (void)fputs("!", stdout); break;
                    }
                    (void)fputs("------", stdout); /* 6 char */
                }
            }

            if(s_have_data == 0) {
                (void)fprintf(stdout, "\"%s\"", s_trace->m_name);
            }
            else {
                (void)fprintf(stdout, "[%s]", s_trace->m_name);
            }

            (void)fputs(" ", stdout);

            while(s_x < (s_view->m_max_width + ((size_t)4u))) {
                (void)fputs(((s_x % ((size_t)2u)) != ((size_t)0u)) ? "." : " ", stdout);
                ++s_x;
            }
            
            /* optional print */
            if((s_x + ((size_t)7u) + ((size_t)1u)) < s_view->m_screen_width) {
                mztree_data_node_t *s_data_node;
                size_t s_data_size;

                for(s_data_node = s_trace->m_data_node, s_data_size = (size_t)0u;s_data_node != ((mztree_data_node_t *)0);s_data_node = s_data_node->m_next) {
                    s_data_size += s_data_node->m_size;
                }

                (void)fprintf(stdout, "%3lu/%3lu", (unsigned long)s_name_size, (unsigned long)s_data_size);
                s_x += (size_t)7u;
            }

            (void)fputs("\n", stdout);
        }
        else {
            if(s_x > s_view->m_max_width) {
                s_view->m_max_width = s_x;
            }
        }

        if(s_trace->m_sub != ((mztree_node_t *)0)) {
            ++s_view->m_level;
            mztree_view_local(s_view, s_trace->m_sub);
            --s_view->m_level;
        }

        if(s_with_next == 0) {
            break;
        }
    }
}

int mztree_view(mztree_t *s_fs, const char *s_pathname, int s_with_next)
{
    mztree_view_t s_view;
    size_t s_keep_flags_size;

    if(s_fs == ((mztree_t *)0)) {
        /* errno = EINVAL */
        return(-1);
    }

    s_view.m_flags = 0x00000000u;
    s_view.m_tree = s_fs;
    s_view.m_node = mztree_do((mztree_node_t **)(&s_fs->m_node), s_pathname, def_mztree_do_mode_search, (void *)0);
    if(s_view.m_node == ((mztree_node_t *)0)) {
        return(-1);
    }
    s_view.m_pathname = (s_pathname == ((const char *)0)) ? "/" : s_pathname;
    s_view.m_level = (size_t)0u;
    s_view.m_max_level = (size_t)0u;
    s_view.m_max_width = (size_t)0u;
    s_view.m_screen_width = (size_t)80u; /* TODO: terminal size */
    s_view.m_with_next = s_with_next;
    s_view.m_keep_flags = (unsigned char *)0;
    
    mztree_view_local((mztree_view_t *)(&s_view), s_view.m_node);

    s_keep_flags_size = sizeof(unsigned char) * s_view.m_max_level + ((size_t)1u);
    s_view.m_keep_flags = (unsigned char *)mztree_alloc(s_keep_flags_size);
    if(s_view.m_keep_flags == ((unsigned char *)0)) {
        /* errno = ENOMEM */
        return(-1);
    }
    (void)memset((void *)s_view.m_keep_flags, 0, s_keep_flags_size);
    s_view.m_flags |= 1u << 0;
    s_view.m_with_next = s_with_next;

    mztree_view_local((mztree_view_t *)(&s_view), s_view.m_node);

    (void)mztree_free((void *)s_view.m_keep_flags);

    return(0);
}

mztree_node_t *mztree_create(mztree_t *s_fs, const char *s_pathname, mode_t s_mode)
{
    return(mztree_do((mztree_node_t **)(&s_fs->m_node), s_pathname, def_mztree_do_mode_new, (void *)(&s_mode)));
}

mztree_node_t *mztree_search(mztree_t *s_fs, const char *s_pathname)
{
    return(mztree_do((mztree_node_t **)(&s_fs->m_node), s_pathname, def_mztree_do_mode_search, (void *)0));
}

mztree_node_t *mztree_remove(mztree_t *s_fs, const char *s_pathname)
{
    return(mztree_do((mztree_node_t **)(&s_fs->m_node), s_pathname, def_mztree_do_mode_free, (void *)0));
}

mztree_node_t *mztree_rename(mztree_t *s_fs, const char *s_oldpathname, const char *s_newpathname)
{
    return(mztree_do((mztree_node_t **)(&s_fs->m_node), s_oldpathname, def_mztree_do_mode_rename, (void *)s_newpathname));
}

ssize_t mztree_read(mztree_node_t *s_node, void *s_data, size_t s_size, off_t s_offset)
{
    mztree_data_node_t *s_data_node;
    off_t s_temp_size;
    size_t s_read_size;

    if(s_offset >= ((off_t)s_node->m_statbuffer.st_size)) {
        return(0);
    }

    if(((off_t)s_node->m_statbuffer.st_size) < (s_offset + ((off_t)s_size))) {
        s_size = (size_t)(((off_t)s_node->m_statbuffer.st_size) - s_offset);
    }

    s_read_size = (size_t)0u;
    s_data_node = s_node->m_data_node;
    
    if((s_read_size < s_size) && (s_data_node != ((mztree_data_node_t *)0))) {
        s_temp_size = s_size;
        if(s_temp_size > (s_data_node->m_size - s_offset)) {
            s_temp_size = s_data_node->m_size - s_offset;
        }
        if(s_temp_size > ((off_t)0)) {
            if(s_data_node->m_data == ((void *)0)) { /* zero hole */
                (void)memset((void *)(((unsigned char *)s_data) + s_read_size), 0, (size_t)s_temp_size);
	    }
            else {
                (void)memcpy((void *)(((unsigned char *)s_data) + s_read_size), (const void *)(((unsigned char *)s_data_node->m_data) + s_offset), (size_t)s_temp_size);
	    }

            s_read_size += (size_t)s_temp_size;
        }

    }

    if(s_read_size < s_size) { /* zero hole */
        (void)memset((void *)(((unsigned char *)s_data) + s_read_size), 0, s_size - s_read_size);
	s_read_size += s_size - s_read_size;
    }

    return((ssize_t)s_read_size);
}

ssize_t mztree_write(mztree_node_t *s_node, const void *s_data, size_t s_size, off_t s_offset)
{
    mztree_data_node_t *s_data_node;
    size_t s_prev_data_node_size;
    size_t s_new_data_node_size;

    if(s_node->m_data_node == ((mztree_data_node_t *)0)) {
        s_prev_data_node_size = (size_t)0;
    }
    else {
        s_prev_data_node_size = s_node->m_data_node->m_alloc_size;
    }

    s_new_data_node_size = s_prev_data_node_size;
    if(s_prev_data_node_size < (((size_t)s_offset) + s_size)) {
       s_new_data_node_size = ((size_t)s_offset) + s_size; 
       s_new_data_node_size = ((s_new_data_node_size + (def_mztree_block_size - 1)) / def_mztree_block_size) * def_mztree_block_size;
    }
   
    s_data_node = s_node->m_data_node;
    if((s_prev_data_node_size != s_new_data_node_size) || (s_data_node == ((mztree_data_node_t *)0))) {
        s_data_node = (mztree_data_node_t *)mztree_realloc((void *)s_node->m_data_node, sizeof(mztree_data_node_t) + s_new_data_node_size);
        if(s_data_node == ((mztree_data_node_t *)0)) {
            return((ssize_t)(-1));
        }
	
	if(s_node->m_data_node == ((mztree_data_node_t *)0)) {
	    (void)memset((void *)s_data_node, 0, sizeof(mztree_data_node_t));
	}
	
	s_data_node->m_alloc_size = s_new_data_node_size;
        s_data_node->m_data = (void *)(&s_data_node[1]);
        
	if(s_new_data_node_size > s_prev_data_node_size) { /* zero hole */
            (void)memset((void *)(((unsigned char *)s_data_node->m_data) + ((size_t)s_prev_data_node_size)), 0, s_new_data_node_size - s_prev_data_node_size);
        }
    }
       
    if(s_data_node->m_size < (((size_t)s_offset) + s_size)) {
        s_data_node->m_size = ((size_t)s_offset) + s_size; 
    }

    (void)memcpy((void *)(((unsigned char *)s_data_node->m_data) + ((size_t)s_offset)), s_data, s_size);

    s_node->m_data_node = s_data_node;
    s_node->m_statbuffer.st_size = s_data_node->m_size;

    return((ssize_t)s_size);
}

#endif

/* End of source */
