/*
    Copyright (C) HWPORT.COM.
    All rights reserved.
    Author: JAEHYUK CHO <mailto:minzkn@minzkn.com>
*/

#if !defined(__def_mzftpd_header_mzftpd_h__)
# define __def_mzftpd_header_mzftpd_h__ "mzftpd.h"

#if !defined(_ISOC99_SOURCE)
# define _ISOC99_SOURCE (1L)
#endif

#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE (1L)
#endif

/* ---- */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <dirent.h>

#include <netinet/in.h>

#if defined(__GNUC__)
# include <pwd.h>
# define def_mzftpd_use_pwd (1L) /* pwd support */
#else
# define def_mzftpd_use_pwd (0L)
#endif

#if defined(__GNUC__)
# include <shadow.h>
# define def_mzftpd_use_shadow (1L) /* shadow support */
#else
# define def_mzftpd_use_shadow (0L)
#endif

#if defined(__GNUC__)
# include <grp.h>
# define def_mzftpd_use_grp (1L) /* grp support */
#else
# define def_mzftpd_use_grp (0L)
#endif

#if defined(__GNUC__)
# include <glob.h>
# define def_mzftpd_use_glob (1L) /* glob support */
#else
# define def_mzftpd_use_glob (0L)
#endif

#if defined(__GNUC__)
# include <crypt.h>
# define def_mzftpd_use_crypt (1L) /* crypt support */
#else
# define def_mzftpd_use_crypt (0L)
#endif

#if defined(__GNUC__)
# include <pthread.h>
# define def_mzftpd_use_pthread (1L) /* pthread support */
#else
# define def_mzftpd_use_pthread (0L)
#endif

/* ---- */

#define def_mzftpd_can_use_long_long (1L)

#if !defined(mzftpd_builtin_expect)
# if __GNUC__ < 3L
#  define mzftpd_builtin_expect(m_expression,m_value) m_expression
# else
#  define mzftpd_builtin_expect(m_expression,m_value) __builtin_expect((long)(m_expression),(long)(m_value))
# endif
#endif

#if defined(__GNUC__)
# define mzftpd_vsprintf_varg_check(m_format_index,m_varg_index) __attribute__((__format__(__printf__,m_format_index,m_varg_index)))
#else
# define mzftpd_vsprintf_varg_check(m_format_index,m_varg_index)
#endif

#if !defined(def_mzftpd_import)
# define def_mzftpd_import extern
#endif

#if !defined(def_mzftpd_import_c)
# if defined(__cplusplus)
#  define def_mzftpd_import_c extern "C"
# else
#  define def_mzftpd_import_c extern
# endif
#endif

#if !defined(def_mzftpd_export)
# if defined(__cplusplus)
#  define def_mzftpd_export
# else
#  define def_mzftpd_export
# endif
#endif

#if !defined(def_mzftpd_export_c)
# if defined(__cplusplus)
#  define def_mzftpd_export_c extern "C"
# else
#  define def_mzftpd_export_c
# endif
#endif

/* ---- */

#if !defined(mzftpd_path_node_t)
typedef struct mzftpd_path_node_ts {
    struct mzftpd_path_node_ts *m_prev;
    struct mzftpd_path_node_ts *m_next;
    unsigned int m_ignore;
    char *m_name;
}__mzftpd_path_node_t;
# define mzftpd_path_node_t __mzftpd_path_node_t
#endif

/* ---- */

#if defined(PF_INET) && defined(AF_INET)
# define def_mzftpd_can_use_ipv4 (1L)
#else
# define def_mzftpd_can_use_ipv4 (0L)
#endif

#if defined(PF_INET6) && defined(AF_INET6)
# define def_mzftpd_can_use_ipv6 (1L)
#else
# define def_mzftpd_can_use_ipv6 (0L)
#endif

/* ---- */

#if !defined(mzftpd_socket_t)
typedef int __mzftpd_socket_t;
# define mzftpd_socket_t __mzftpd_socket_t
#endif

#if !defined(mzftpd_socklen_t)
typedef socklen_t __mzftpd_socklen_t;
# define mzftpd_socklen_t __mzftpd_socklen_t
#endif

#if !defined(mzftpd_sockdomain_t)
typedef int __mzftpd_sockdomain_t;
# define mzftpd_sockdomain_t __mzftpd_sockdomain_t
#endif

#if !defined(mzftpd_sockfamily_t)
typedef sa_family_t __mzftpd_sockfamily_t;
# define mzftpd_sockfamily_t __mzftpd_sockfamily_t
#endif

#if !defined(mzftpd_sockprotocol_t)
typedef int __mzftpd_sockprotocol_t;
# define mzftpd_sockprotocol_t __mzftpd_sockprotocol_t
#endif

#if !defined(mzftpd_sockaddr_t)
typedef struct sockaddr __mzftpd_sockaddr_t;
# define mzftpd_sockaddr_t __mzftpd_sockaddr_t
#endif

#if (def_mzftpd_can_use_ipv4 != 0L) && (!defined(mzftpd_in4_addr_t))
typedef struct in_addr __mzftpd_in4_addr_t;
# define mzftpd_in4_addr_t __mzftpd_in4_addr_t
#endif

#if (def_mzftpd_can_use_ipv6 != 0L) && (!defined(mzftpd_in6_addr_t))
typedef struct in6_addr __mzftpd_in6_addr_t;
# define mzftpd_in6_addr_t __mzftpd_in6_addr_t
#endif

#if (def_mzftpd_can_use_ipv4 != 0L) && (!defined(mzftpd_sockaddr_in4_t))
typedef struct sockaddr_in __mzftpd_sockaddr_in4_t;
# define mzftpd_sockaddr_in4_t __mzftpd_sockaddr_in4_t
#endif

#if (def_mzftpd_can_use_ipv6 != 0L) && (!defined(mzftpd_sockaddr_in6_t))
typedef struct sockaddr_in6 __mzftpd_sockaddr_in6_t;
# define mzftpd_sockaddr_in6_t __mzftpd_sockaddr_in6_t
#endif

#if !defined(mzftpd_sockaddr_storage_t)
typedef struct sockaddr_storage __mzftpd_sockaddr_storage_t;
# define mzftpd_sockaddr_storage_t __mzftpd_sockaddr_storage_t
#endif

#if !defined(mzftpd_sockaddr_all_t)
typedef union mzftpd_sockaddr_all_tu {
    unsigned char m_raw[ sizeof(mzftpd_sockaddr_storage_t) ];

    mzftpd_sockaddr_storage_t m_ss;
    mzftpd_sockaddr_t m_s;
# if def_mzftpd_can_use_ipv4 != 0L
    mzftpd_sockaddr_in4_t m_in4;
# endif
# if def_mzftpd_can_use_ipv6 != 0L
    mzftpd_sockaddr_in6_t m_in6;
# endif
}__mzftpd_sockaddr_all_t;
# define mzftpd_sockaddr_all_t __mzftpd_sockaddr_all_t
#endif


/* ---- */

#define def_mzftpd_company_name "HWPORT.COM."
#define def_mzftpd_server_name "HWPORT FTP Server"

#define def_mzftpd_worker_recv_timeout (-1)
#define def_mzftpd_worker_send_timeout (-1)

/* ---- */

#if !defined(mzftpd_t)
typedef void * __mzftpd_t;
# define mzftpd_t __mzftpd_t
#endif

#if !defined(mzftpd_account_t)
typedef struct mzftpd_account_ts {
    struct mzftpd_account_ts *m_prev;
    struct mzftpd_account_ts *m_next;

    unsigned int m_flags;

    char *m_username;
    char *m_plain_password;

    char *m_path_home;

    uid_t m_uid;
    gid_t m_gid;
}__mzftpd_account_t;
# define mzftpd_account_t __mzftpd_account_t
# define def_mzftpd_account_flag_none (0x00000000u)
# define def_mzftpd_account_flag_admin_user (0x00000001u)
# define def_mzftpd_account_flag_system_user (0x00000002u)
# define def_mzftpd_account_flag_guest_user (0x00000004u)
# define def_mzftpd_account_flag_allow_all_path (0x00000008u)
# define def_mzftpd_account_flag_encrypted_by_crypt (0x00010000u)
#endif

#if !defined(mzftpd_shadow_t)
typedef struct mzftpd_shadow_ts {
    mzftpd_socket_t m_listen_socket;
    mzftpd_sockaddr_all_t m_listen_addr;

    mzftpd_account_t *m_account_head;
    mzftpd_account_t *m_account_tail;
}__mzftpd_shadow_t;
# define mzftpd_shadow_t __mzftpd_shadow_t
#endif

#if !defined(mzftpd_session_t)
typedef struct mzftpd_session_ts {
    mzftpd_t m_handle;

    mzftpd_account_t *m_account_head;
    mzftpd_account_t *m_current_account;

    unsigned int m_flags;

    int m_send_timeout;
    int m_recv_timeout;

    /* command */
    mzftpd_socket_t m_command_socket;
    mzftpd_sockaddr_all_t m_command_sockaddr_all;
    mzftpd_socklen_t m_command_sockaddr_size;
    size_t m_command_buffer_size; 
    unsigned char *m_command_buffer;

    char *m_command;
    char *m_param;

    /* data */
    mzftpd_socket_t m_data_socket;
    mzftpd_sockaddr_all_t m_data_sockaddr_all;
    mzftpd_socklen_t m_data_sockaddr_size;
    size_t m_data_buffer_size; 
    unsigned char *m_data_buffer;
    off_t m_restart_position;

    /* file */
    int m_fd;

    /* current user info */
    char *m_username;
    unsigned int m_type;
   
    /* - */
    char *m_path_home;
    char *m_path_work;
    char *m_path_rename_from;
}__mzftpd_session_t;
# define mzftpd_session_t __mzftpd_session_t

# define def_mzftpd_session_flag_none (0x00000000u)
# define def_mzftpd_session_flag_user (0x00000001u)
# define def_mzftpd_session_flag_restart_position (0x00000002u)
# define def_mzftpd_session_flag_rename_from (0x00000004u)
# define def_mzftpd_session_flag_fork (0x00000008u)

# define def_mzftpd_session_type_none (0x00000000u)
# define def_mzftpd_session_type_A (0x00000001u)
# define def_mzftpd_session_type_I (0x00000002u)
# define def_mzftpd_session_type_L8 (0x00000003u)

# define def_mzftpd_list_option_none (0x00000000u)
# define def_mzftpd_list_option_a (0x00000001u)
# define def_mzftpd_list_option_l (0x00000002u)
# define def_mzftpd_list_option_f (0x00000004u)
# define def_mzftpd_list_option_r (0x00000008u)
# define def_mzftpd_list_option_unknown (0x80000000u)
#endif

/* ---- */

#if !defined(__def_mzftpd_source_mzftpd_c__)
def_mzftpd_import_c int mzftpd_isdigit(int s_character);
def_mzftpd_import_c int mzftpd_isspace(int s_character);
def_mzftpd_import_c int mzftpd_toupper(int s_character);

def_mzftpd_import_c size_t mzftpd_strnlen(const char *s_string, size_t s_max_size);
def_mzftpd_import_c size_t mzftpd_strlen(const char *s_string);

def_mzftpd_import_c char *mzftpd_strncpy(char *s_to, const char *s_from, size_t s_max_size);
def_mzftpd_import_c char *mzftpd_strcpy(char *s_to, const char *s_from);
def_mzftpd_import_c char *mzftpd_strncat(char *s_to, const char *s_from, size_t s_max_size);
def_mzftpd_import_c char *mzftpd_strcat(char *s_to, const char *s_from);

def_mzftpd_import_c int mzftpd_strncmp(const char *s_left, const char *s_right, size_t s_max_size);
def_mzftpd_import_c int mzftpd_strcmp(const char *s_left, const char *s_right);
def_mzftpd_import_c int mzftpd_strncasecmp(const char *s_left, const char *s_right, size_t s_max_size);
def_mzftpd_import_c int mzftpd_strcasecmp(const char *s_left, const char *s_right);

def_mzftpd_import_c char *mzftpd_strpbrk(const char *s_string, const char *s_this);

def_mzftpd_import_c char *mzftpd_strstr(const char *s_string, const char *s_this);
def_mzftpd_import_c char *mzftpd_strcasestr(const char *s_string, const char *s_this);

def_mzftpd_import_c char *mzftpd_strndup(const char *s_string, size_t s_size);
def_mzftpd_import_c char *mzftpd_strdup(const char *s_string);

def_mzftpd_import_c size_t mzftpd_xtoa_limit(char *s_output, size_t s_max_output_size, unsigned int s_value, unsigned int s_radix, unsigned int s_width, const char *s_digits);
#if def_mzftpd_can_use_long_long != (0L)
def_mzftpd_import_c size_t mzftpd_llxtoa_limit(char *s_output, size_t s_max_output_size, unsigned long long s_value, unsigned int s_radix, unsigned int s_width, const char *s_digits);
#endif
def_mzftpd_import_c int mzftpd_atox(const char *s_string, int s_base);
def_mzftpd_import_c int mzftpd_atoi(const char *s_string);
#if def_mzftpd_can_use_long_long != (0L)
def_mzftpd_import_c long long mzftpd_atollx(const char *s_string, int s_base);
def_mzftpd_import_c long long mzftpd_atoll(const char *s_string);
#endif
def_mzftpd_import_c int mzftpd_vsnprintf(char *s_output, size_t s_max_output_size, const char *s_format, va_list s_var);
def_mzftpd_import_c int mzftpd_vsprintf(char *s_output, const char *s_format, va_list s_var);
def_mzftpd_import_c int mzftpd_snprintf(char *s_output, size_t s_max_output_size, const char *s_format, ...) mzftpd_vsprintf_varg_check(3,4);
def_mzftpd_import_c int mzftpd_sprintf(char *s_output, const char *s_format, ...) mzftpd_vsprintf_varg_check(2,3);

def_mzftpd_import_c char *mzftpd_alloc_vsprintf(const char *s_format, va_list s_var);
def_mzftpd_import_c char *mzftpd_alloc_sprintf(const char *s_format, ...) mzftpd_vsprintf_varg_check(1,2);

def_mzftpd_import_c char *mzftpd_get_word_sep(int s_skip_space, const char *s_sep, char **s_sep_string);
def_mzftpd_import_c char *mzftpd_get_word_sep_alloc(int s_skip_space, const char *s_sep, const char **s_sep_string);

def_mzftpd_import_c int mzftpd_check_pattern(const char *s_pattern, const char *s_string);

def_mzftpd_import_c mzftpd_path_node_t *mzftpd_free_path_node(mzftpd_path_node_t *s_node);
def_mzftpd_import_c mzftpd_path_node_t *mzftpd_path_to_node(const char *s_path);
def_mzftpd_import_c char *mzftpd_node_to_path(mzftpd_path_node_t *s_node, int s_strip);
def_mzftpd_import_c mzftpd_path_node_t *mzftpd_copy_path_node(mzftpd_path_node_t *s_node);
def_mzftpd_import_c mzftpd_path_node_t *mzftpd_append_path_node(mzftpd_path_node_t *s_head, mzftpd_path_node_t *s_node, int s_override);

def_mzftpd_import_c char *mzftpd_basename(char *s_pathname);

def_mzftpd_import_c mzftpd_sockprotocol_t mzftpd_get_protocol_by_name(const char *s_protocol_name);

def_mzftpd_import_c mzftpd_socket_t mzftpd_socket_open(mzftpd_sockdomain_t s_domain, mzftpd_sockfamily_t s_type, mzftpd_sockprotocol_t s_protocol);
def_mzftpd_import_c mzftpd_socket_t mzftpd_socket_close(mzftpd_socket_t s_socket);

def_mzftpd_import_c int mzftpd_bind(mzftpd_socket_t s_socket, const void *s_sockaddr_ptr, mzftpd_socklen_t s_sockaddr_size);
def_mzftpd_import_c int mzftpd_listen(mzftpd_socket_t s_socket, int s_backlog);
def_mzftpd_import_c mzftpd_socket_t mzftpd_accept(mzftpd_socket_t s_listen_socket, void *s_sockaddr_ptr, mzftpd_socklen_t *s_sockaddr_size_ptr, int s_msec);
def_mzftpd_import_c int mzftpd_connect(mzftpd_socket_t s_socket, const void *s_sockaddr_ptr, mzftpd_socklen_t s_sockaddr_size, int s_msec);
def_mzftpd_import_c ssize_t mzftpd_recv(mzftpd_socket_t s_socket, void *s_data, size_t s_size, int s_msec);
def_mzftpd_import_c ssize_t mzftpd_send(mzftpd_socket_t s_socket, const void *s_data, size_t s_size, int s_msec);
def_mzftpd_import_c ssize_t mzftpd_send_message(mzftpd_socket_t s_socket, int s_msec, const char *s_format, ...) mzftpd_vsprintf_varg_check(3,4);

def_mzftpd_import_c const char *mzftpd_inet_ntop(mzftpd_sockfamily_t s_family, const void *s_inX_addr_ptr, char *s_address, mzftpd_socklen_t s_address_size);
def_mzftpd_import_c const char *mzftpd_inet_stop(const mzftpd_sockaddr_all_t *s_sockaddr_all, char *s_address, mzftpd_socklen_t s_address_size);
def_mzftpd_import_c int mzftpd_inet_pton(mzftpd_sockfamily_t s_family, const char *s_address, void *s_inX_addr_ptr);

def_mzftpd_import_c mzftpd_t mzftpd_open(void);
def_mzftpd_import_c mzftpd_t mzftpd_open_with_port(int s_bind_port);
def_mzftpd_import_c mzftpd_t mzftpd_close(mzftpd_t s_handle);
def_mzftpd_import_c int mzftpd_do(mzftpd_t s_handle, int s_msec);

def_mzftpd_import_c mzftpd_account_t *mzftpd_new_account(const char *s_username, unsigned int s_flags);
def_mzftpd_import_c mzftpd_account_t *mzftpd_free_account(mzftpd_account_t *s_account);
def_mzftpd_import_c int mzftpd_account_set_plain_password(mzftpd_account_t *s_account, const char *s_plain_password);
def_mzftpd_import_c int mzftpd_add_account(mzftpd_t s_handle, mzftpd_account_t *s_account);
def_mzftpd_import_c int mzftpd_account_set_path_home(mzftpd_account_t *s_account, const char *s_path_home);
def_mzftpd_import_c int mzftpd_add_user(mzftpd_t s_handle, mzftpd_account_t **s_account_ptr, unsigned int s_flags, const char *s_username, const char *s_plain_password, const char *s_path_home);
def_mzftpd_import_c mzftpd_account_t *mzftpd_account_check_login(mzftpd_account_t *s_account_head, const char *s_username, const char *s_plain_password);

def_mzftpd_import_c int mzftpd_data_open(mzftpd_session_t *s_session);
def_mzftpd_import_c int mzftpd_data_close(mzftpd_session_t *s_session);
#endif

#endif

/* vim: set expandtab: */
/* End of header */
