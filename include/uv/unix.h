/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef UV_UNIX_H
#define UV_UNIX_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>  /* MAXHOSTNAMELEN on Solaris */

#include <termios.h>
#include <pwd.h>

#if !defined(__MVS__)
#include <semaphore.h>
#include <sys/param.h> /* MAXHOSTNAMELEN on Linux and the BSDs */
#endif
#include <pthread.h>
#include <signal.h>

#include "uv/threadpool.h"

#if defined(__linux__)
# include "uv/linux.h"
#elif defined (__MVS__)
# include "uv/os390.h"
#elif defined(__PASE__)  /* __PASE__ and _AIX are both defined on IBM i */
# include "uv/posix.h"  /* IBM i needs uv/posix.h, not uv/aix.h */
#elif defined(_AIX)
# include "uv/aix.h"
#elif defined(__sun)
# include "uv/sunos.h"
#elif defined(__APPLE__)
# include "uv/darwin.h"
#elif defined(__DragonFly__)       || \
      defined(__FreeBSD__)         || \
      defined(__OpenBSD__)         || \
      defined(__NetBSD__)
# include "uv/bsd.h"
#elif defined(__CYGWIN__) || \
      defined(__MSYS__)   || \
      defined(__HAIKU__)  || \
      defined(__QNX__)    || \
      defined(__GNU__)
# include "uv/posix.h"
#endif

#ifndef NI_MAXHOST
# define NI_MAXHOST 1025
#endif

#ifndef NI_MAXSERV
# define NI_MAXSERV 32
#endif

#ifndef UV_IO_PRIVATE_PLATFORM_FIELDS
# define UV_IO_PRIVATE_PLATFORM_FIELDS /* empty */
#endif

struct uv__io_s;
struct uv_loop_s;

typedef void (*uv__io_cb)(struct uv_loop_s* loop,
                          struct uv__io_s* w,
                          unsigned int events);
typedef struct uv__io_s uv__io_t;

struct uv__io_s {
  uv__io_cb cb;
  struct uv__queue pending_queue;
  struct uv__queue watcher_queue;
  unsigned int pevents; /* Pending event mask i.e. mask at next tick. */
  unsigned int events;  /* Current event mask. */
  int fd;
  UV_IO_PRIVATE_PLATFORM_FIELDS
};

#ifndef UV_PLATFORM_SEM_T
# define UV_PLATFORM_SEM_T sem_t
#endif

#ifndef UV_PLATFORM_LOOP_FIELDS
# define UV_PLATFORM_LOOP_FIELDS /* empty */
#endif

#ifndef UV_PLATFORM_FS_EVENT_FIELDS
# define UV_PLATFORM_FS_EVENT_FIELDS /* empty */
#endif

#ifndef UV_STREAM_PRIVATE_PLATFORM_FIELDS
# define UV_STREAM_PRIVATE_PLATFORM_FIELDS /* empty */
#endif

/* Note: May be cast to struct iovec. See writev(2). */
typedef struct uv_buf_t {
  char* base;
  size_t len;
} uv_buf_t;

typedef int uv_file;
typedef int uv_os_sock_t;
typedef int uv_os_fd_t;
typedef pid_t uv_pid_t;

#define UV_ONCE_INIT PTHREAD_ONCE_INIT

typedef pthread_once_t uv_once_t;
typedef pthread_t uv_thread_t;
typedef pthread_mutex_t uv_mutex_t;
typedef pthread_rwlock_t uv_rwlock_t;
typedef UV_PLATFORM_SEM_T uv_sem_t;
typedef pthread_cond_t uv_cond_t;
typedef pthread_key_t uv_key_t;

/* Note: guard clauses should match uv_barrier_init's in src/unix/thread.c. */
#if defined(_AIX) || \
    defined(__OpenBSD__) || \
    !defined(PTHREAD_BARRIER_SERIAL_THREAD)
/* TODO(bnoordhuis) Merge into uv_barrier_t in v2. */
struct _uv_barrier {
  uv_mutex_t mutex;
  uv_cond_t cond;
  unsigned threshold;
  unsigned in;
  unsigned out;
};

typedef struct {
  struct _uv_barrier* b;
# if defined(PTHREAD_BARRIER_SERIAL_THREAD)
  /* TODO(bnoordhuis) Remove padding in v2. */
  char pad[sizeof(pthread_barrier_t) - sizeof(struct _uv_barrier*)];
# endif
} uv_barrier_t;
#else
typedef pthread_barrier_t uv_barrier_t;
#endif

/* Platform-specific definitions for uv_spawn support. */
typedef gid_t uv_gid_t;
typedef uid_t uv_uid_t;

typedef struct dirent uv__dirent_t;

#define UV_DIR_PRIVATE_FIELDS \
  DIR* dir;

#if defined(DT_UNKNOWN)
# define HAVE_DIRENT_TYPES
# if defined(DT_REG)
#  define UV__DT_FILE DT_REG
# else
#  define UV__DT_FILE -1
# endif
# if defined(DT_DIR)
#  define UV__DT_DIR DT_DIR
# else
#  define UV__DT_DIR -2
# endif
# if defined(DT_LNK)
#  define UV__DT_LINK DT_LNK
# else
#  define UV__DT_LINK -3
# endif
# if defined(DT_FIFO)
#  define UV__DT_FIFO DT_FIFO
# else
#  define UV__DT_FIFO -4
# endif
# if defined(DT_SOCK)
#  define UV__DT_SOCKET DT_SOCK
# else
#  define UV__DT_SOCKET -5
# endif
# if defined(DT_CHR)
#  define UV__DT_CHAR DT_CHR
# else
#  define UV__DT_CHAR -6
# endif
# if defined(DT_BLK)
#  define UV__DT_BLOCK DT_BLK
# else
#  define UV__DT_BLOCK -7
# endif
#endif

/* Platform-specific definitions for uv_dlopen support. */
#define UV_DYNAMIC /* empty */

typedef struct {
  void* handle;
  char* errmsg;
} uv_lib_t;

#define UV_LOOP_PRIVATE_FIELDS                                                \
  unsigned long flags;                                                        /*标志位；Libuv 运行的一些标记，目前只有 UV_LOOP_BLOCK_SIGPROF，主要是用于 epoll_wait 的时候屏蔽 SIGPROF 信号，防止无效唤醒。*/  \
  int backend_fd;                                                             /*后端文件描述符；事件驱动模块的 fd，比如调用 epoll_create 返回的 fd*/  \
  struct uv__queue pending_queue;                                             /*待处理队列*/  \
  struct uv__queue watcher_queue;                                             /*观察者队列；指向需要在事件驱动模块中注册事件的 IO 观察者队列*/  \
  uv__io_t** watchers;                                                        /*观察者数组；watcher_queue 队列的节点 uv__io_t 中有一个 fd 字段，watchers 以 fd 为索引，记录 fd 所关联的 uv__io_t 结构体*/  \
  unsigned int nwatchers;                                                     /*观察者数量；watchers 相关的数量，在 maybe_resize 函数里设置*/  \
  unsigned int nfds;                                                          /*文件描述符数量；loop->watchers 中已使用的元素个数，一般为 watcher_queue 队列的节点数*/  \
  struct uv__queue wq;                                                        /*工作队列；线程池的子线程处理完任务后把对应的结构体插入到 wq 队列，由主线程在 Poll IO 阶段处理*/  \
  uv_mutex_t wq_mutex;                                                        /*工作队列互斥锁；控制 wq 队列互斥访问，否则多个子线程同时访问会有问题*/  \
  uv_async_t wq_async;                                                        /*工作队列异步处理；用于线程池的子线程和主线程通信*/  \
  uv_rwlock_t cloexec_lock;                                                   /*执行关闭操作的读写锁；用于设置 close-on-exec 时的锁，因为打开文件和设置 close-on-exec 不是原子操作（除非系统支持），所以需要一个锁控制这两个步骤是一个原子操作。*/  \
  uv_handle_t* closing_handles;                                               /*正在关闭的句柄；事件循环 close 阶段的队列，由 uv_close 产生*/  \
  struct uv__queue process_handles;                                           /*进程句柄队列；fork 出来的子进程队列*/  \
  struct uv__queue prepare_handles;                                           /*准备句柄队列；事件循环的 prepare 阶段对应的任务队列*/  \
  struct uv__queue check_handles;                                             /*检查句柄队列；事件循环的 check 阶段对应的任务队列*/  \
  struct uv__queue idle_handles;                                              /*空闲句柄队列；事件循环的 idle 阶段对应的任务队列*/  \
  struct uv__queue async_handles;                                             /*异步句柄队列；async_handles 队列，Poll IO 阶段执行 uv__async_io 遍历 async_handles 队列，处理里面 pending 为 1 的节点，然后执行它的回调*/  \
  void (*async_unused)(void);  /* TODO(bnoordhuis) Remove in libuv v2. */     /*未使用的异步函数，将在libuv v2中移除*/  \
  uv__io_t async_io_watcher;                                                  /*异步IO观察者；用于线程间通信 async handle 的 IO 观察者。用于监听是否有 async handle 任务需要处理*/  \
  int async_wfd;                                                              /*异步写文件描述符；用于保存子线程和主线程通信的写端 fd*/  \
  struct {                                                                    \
    void* min;                                                                /*最小值*/  \
    unsigned int nelts;                                                       /*元素数量*/  \
  } timer_heap;                                                               /*定时器堆；保存定时器二叉堆结构*/  \
  uint64_t timer_counter;                                                     /*定时器计数器；管理定时器节点的递增 id*/  \
  uint64_t time;                                                              /*时间；当前时间，Libuv 会在每次事件循环的开始和 Poll IO 阶段更新当前时间，然后在后续的各个阶段使用，减少系统调用次数*/  \
  int signal_pipefd[2];                                                       /*信号管道文件描述符；fork 出来的进程和主进程通信的管道，用于子进程收到信号的时候通知主进程，然后主进程执行子进程节点注册的回调*/  \
  uv__io_t signal_io_watcher;                                                 /*信号IO观察者；用于信号处理的 IO 观察者，类似 async_io_watcher，signal_io_watcher 保存了管道读端 fd 和回调，然后注册到事件驱动模块中，在子进程收到信号的时候，通过 write 写到管道，最后主进程在 Poll IO 阶段执行回调*/  \
  uv_signal_t child_watcher;                                                  /*子进程观察者；用于管理子进程退出信号的 handle*/  \
  int emfile_fd;                                                              /*备用的 fd; 当服务器处理连接因 fd 耗尽而失败时，可以使用 emfile_fd*/  \

  UV_PLATFORM_LOOP_FIELDS                                                     \

#define UV_REQ_TYPE_PRIVATE /* empty */

#define UV_REQ_PRIVATE_FIELDS  /* empty */

#define UV_PRIVATE_REQ_TYPES /* empty */

#define UV_WRITE_PRIVATE_FIELDS                                               \
  struct uv__queue queue;                                                     \
  unsigned int write_index;                                                   \
  uv_buf_t* bufs;                                                             \
  unsigned int nbufs;                                                         \
  int error;                                                                  \
  uv_buf_t bufsml[4];                                                         \

#define UV_CONNECT_PRIVATE_FIELDS                                             \
  struct uv__queue queue;                                                     \

#define UV_SHUTDOWN_PRIVATE_FIELDS /* empty */

#define UV_UDP_SEND_PRIVATE_FIELDS                                            \
  struct uv__queue queue;                                                     \
  struct sockaddr_storage addr;                                               \
  unsigned int nbufs;                                                         \
  uv_buf_t* bufs;                                                             \
  ssize_t status;                                                             \
  uv_udp_send_cb send_cb;                                                     \
  uv_buf_t bufsml[4];                                                         \

#define UV_HANDLE_PRIVATE_FIELDS                                              \
  uv_handle_t* next_closing;         /* 当 handle 在 close 队列时，该字段指向下一个 close 节点 */\
  unsigned int flags;                /* handle 的状态和标记 */                                         \

#define UV_STREAM_PRIVATE_FIELDS                                              \
  uv_connect_t *connect_req;                /* 连接请求；发起连接时，保存连接上下文的结构体 */ \
  uv_shutdown_t *shutdown_req;              /* 关闭请求；关闭写端时，保存上下文的结构体 */ \
  uv__io_t io_watcher;                      /* IO观察者；用于插入事件驱动模块的 IO 观察者，注册读写事件 */ \
  struct uv__queue write_queue;             /* 写入队列；待发送队列，记录了等待写操作的数据和元信息，和 write_queue_size 配合 */ \
  struct uv__queue write_completed_queue;   /* 已完成写入的队列；发送完成的队列，write_queue 的节点发生完毕后就会插入 write_completed_queue 队列，等待执行写结束回调 */ \
  uv_connection_cb connection_cb;           /* 连接回调；收到连接时执行的回调 */ \
  int delayed_error;                        /* 延迟错误；socket 操作失败的错误码，比如连接失败 */ \
  int accepted_fd;                          /* 已接受的文件描述符；accept 返回的 fd 或者 IPC 时收到的文件描述符 */ \
  void* queued_fds;                         /* 队列中的文件描述符；用于 IPC，accepted_fd 只能保存一个 fd，queued_fds 用于保存剩下的 */ \

  UV_STREAM_PRIVATE_PLATFORM_FIELDS                                           \

#define UV_TCP_PRIVATE_FIELDS /* empty */

#define UV_UDP_PRIVATE_FIELDS                                                 \
  uv_alloc_cb alloc_cb;                                                       \
  uv_udp_recv_cb recv_cb;                                                     \
  uv__io_t io_watcher;                                                        \
  struct uv__queue write_queue;                                               \
  struct uv__queue write_completed_queue;                                     \

#define UV_PIPE_PRIVATE_FIELDS                                                \
  const char* pipe_fname; /* NULL or strdup'ed */

#define UV_POLL_PRIVATE_FIELDS                                                \
  uv__io_t io_watcher;

#define UV_PREPARE_PRIVATE_FIELDS                                             \
  uv_prepare_cb prepare_cb;                                                   \
  struct uv__queue queue;                                                     \

#define UV_CHECK_PRIVATE_FIELDS                                               \
  uv_check_cb check_cb;                                                       \
  struct uv__queue queue;                                                     \

#define UV_IDLE_PRIVATE_FIELDS                                                \
  uv_idle_cb idle_cb;                                                         \
  struct uv__queue queue;                                                     \

#define UV_ASYNC_PRIVATE_FIELDS                                               \
  uv_async_cb async_cb;    /* 异步回调函数; 异步事件触发时执行的回调 */                                                      \
  struct uv__queue queue;  /* 队列结构体; 用于插入 async->handles 队列 */                                                     \
  int pending;             /* 标记是否有任务需要处理，为 1 说明需要执行回调 async_cb 处理任务 */                                                               \

#define UV_TIMER_PRIVATE_FIELDS                                               \
  uv_timer_cb timer_cb;                                                       \
  void* heap_node[3];                                                         \
  uint64_t timeout;                                                           \
  uint64_t repeat;                                                            \
  uint64_t start_id;

#define UV_GETADDRINFO_PRIVATE_FIELDS                                         \
  struct uv__work work_req;                                                   \
  uv_getaddrinfo_cb cb;                                                       \
  struct addrinfo* hints;                                                     \
  char* hostname;                                                             \
  char* service;                                                              \
  struct addrinfo* addrinfo;                                                  \
  int retcode;

#define UV_GETNAMEINFO_PRIVATE_FIELDS                                         \
  struct uv__work work_req;                                                   \
  uv_getnameinfo_cb getnameinfo_cb;                                           \
  struct sockaddr_storage storage;                                            \
  int flags;                                                                  \
  char host[NI_MAXHOST];                                                      \
  char service[NI_MAXSERV];                                                   \
  int retcode;

#define UV_PROCESS_PRIVATE_FIELDS                                             \
  struct uv__queue queue;                                                     \
  int status;                                                                 \

#define UV_FS_PRIVATE_FIELDS                                                  \
  const char *new_path;                                                       \
  uv_file file;                                                               \
  int flags;                                                                  \
  mode_t mode;                                                                \
  unsigned int nbufs;                                                         \
  uv_buf_t* bufs;                                                             \
  off_t off;                                                                  \
  uv_uid_t uid;                                                               \
  uv_gid_t gid;                                                               \
  double atime;                                                               \
  double mtime;                                                               \
  struct uv__work work_req;                                                   \
  uv_buf_t bufsml[4];                                                         \

#define UV_WORK_PRIVATE_FIELDS                                                \
  struct uv__work work_req;

#define UV_TTY_PRIVATE_FIELDS                                                 \
  struct termios orig_termios;                                                \
  int mode;

#define UV_SIGNAL_PRIVATE_FIELDS                                              \
  /* RB_ENTRY(uv_signal_s) tree_entry; */                                     \
  struct {                                                                    \
    struct uv_signal_s* rbe_left;                                             \
    struct uv_signal_s* rbe_right;                                            \
    struct uv_signal_s* rbe_parent;                                           \
    int rbe_color;                                                            \
  } tree_entry;                                                               \
  /* Use two counters here so we don have to fiddle with atomics. */          \
  unsigned int caught_signals;                                                \
  unsigned int dispatched_signals;

#define UV_FS_EVENT_PRIVATE_FIELDS                                            \
  uv_fs_event_cb cb;                                                          \
  UV_PLATFORM_FS_EVENT_FIELDS                                                 \

/* fs open() flags supported on this platform: */
#if defined(O_APPEND)
# define UV_FS_O_APPEND       O_APPEND
#else
# define UV_FS_O_APPEND       0
#endif
#if defined(O_CREAT)
# define UV_FS_O_CREAT        O_CREAT
#else
# define UV_FS_O_CREAT        0
#endif

#if defined(__linux__) && defined(__arm__)
# define UV_FS_O_DIRECT       0x10000
#elif defined(__linux__) && defined(__m68k__)
# define UV_FS_O_DIRECT       0x10000
#elif defined(__linux__) && defined(__mips__)
# define UV_FS_O_DIRECT       0x08000
#elif defined(__linux__) && defined(__powerpc__)
# define UV_FS_O_DIRECT       0x20000
#elif defined(__linux__) && defined(__s390x__)
# define UV_FS_O_DIRECT       0x04000
#elif defined(__linux__) && defined(__x86_64__)
# define UV_FS_O_DIRECT       0x04000
#elif defined(__linux__) && defined(__loongarch__)
# define UV_FS_O_DIRECT       0x04000
#elif defined(O_DIRECT)
# define UV_FS_O_DIRECT       O_DIRECT
#else
# define UV_FS_O_DIRECT       0
#endif

#if defined(O_DIRECTORY)
# define UV_FS_O_DIRECTORY    O_DIRECTORY
#else
# define UV_FS_O_DIRECTORY    0
#endif
#if defined(O_DSYNC)
# define UV_FS_O_DSYNC        O_DSYNC
#else
# define UV_FS_O_DSYNC        0
#endif
#if defined(O_EXCL)
# define UV_FS_O_EXCL         O_EXCL
#else
# define UV_FS_O_EXCL         0
#endif
#if defined(O_EXLOCK)
# define UV_FS_O_EXLOCK       O_EXLOCK
#else
# define UV_FS_O_EXLOCK       0
#endif
#if defined(O_NOATIME)
# define UV_FS_O_NOATIME      O_NOATIME
#else
# define UV_FS_O_NOATIME      0
#endif
#if defined(O_NOCTTY)
# define UV_FS_O_NOCTTY       O_NOCTTY
#else
# define UV_FS_O_NOCTTY       0
#endif
#if defined(O_NOFOLLOW)
# define UV_FS_O_NOFOLLOW     O_NOFOLLOW
#else
# define UV_FS_O_NOFOLLOW     0
#endif
#if defined(O_NONBLOCK)
# define UV_FS_O_NONBLOCK     O_NONBLOCK
#else
# define UV_FS_O_NONBLOCK     0
#endif
#if defined(O_RDONLY)
# define UV_FS_O_RDONLY       O_RDONLY
#else
# define UV_FS_O_RDONLY       0
#endif
#if defined(O_RDWR)
# define UV_FS_O_RDWR         O_RDWR
#else
# define UV_FS_O_RDWR         0
#endif
#if defined(O_SYMLINK)
# define UV_FS_O_SYMLINK      O_SYMLINK
#else
# define UV_FS_O_SYMLINK      0
#endif
#if defined(O_SYNC)
# define UV_FS_O_SYNC         O_SYNC
#else
# define UV_FS_O_SYNC         0
#endif
#if defined(O_TRUNC)
# define UV_FS_O_TRUNC        O_TRUNC
#else
# define UV_FS_O_TRUNC        0
#endif
#if defined(O_WRONLY)
# define UV_FS_O_WRONLY       O_WRONLY
#else
# define UV_FS_O_WRONLY       0
#endif

/* fs open() flags supported on other platforms: */
#define UV_FS_O_FILEMAP       0
#define UV_FS_O_RANDOM        0
#define UV_FS_O_SHORT_LIVED   0
#define UV_FS_O_SEQUENTIAL    0
#define UV_FS_O_TEMPORARY     0

#endif /* UV_UNIX_H */
