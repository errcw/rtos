#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <signal.h>

#include <errno.h>
#undef errno
extern int errno;

#include "lib.h"
#include "video.h"

void _exit (int status)
{
	__asm__ ("hlt");
}

int close(int file){
    return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int execve (const char *__path, char * const __argv[], char * const __envp[] )
{
  errno = ENOMEM;
  return -1;
}

int fork () 
{
  errno = EAGAIN;
  return -1;
}

int fstat (int file, struct stat *st) 
{
  st->st_mode = S_IFCHR;
  return 0;
}

int getpid () 
{
  return -1;
}

int isatty (int file)
{
   return 1;
}

int kill (pid_t pid, int sig)
{
  errno = EINVAL;
  return -1;
}

int link (const char *__path1, const char *__path2 )
{
  errno=EMLINK;
  return -1;
}

off_t lseek (int __fildes, off_t __offset, int __whence )
{
  return 0;
}

int open (const char *name, int flags, int mode)
{
  return -1;
}

_READ_WRITE_RETURN_TYPE read (int __fd, void *__buf, size_t __nbyte )
{
  return 0;
}

/*
 * Increase program data space. As malloc and related functions depend on 
 * this, it is useful to have a working implementation. The following 
 * suffices for a standalone system; it exploits the symbol end automatically 
 * defined by the GNU linker.
 */
void *sbrk (ptrdiff_t incr)
{
  extern char end;		/* Defined by the linker */
  static char *heap_end;
  char *prev_heap_end;
 
  if (heap_end == 0) {
    heap_end = &end;
  }
  prev_heap_end = heap_end;
  /*
  if (heap_end + incr > stack_ptr)
    {
      _write (1, "Heap and stack collisionn", 25);
      abort ();
    }
	*/

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int stat (const char *__path, struct stat *__sbuf)
{
  __sbuf->st_mode = S_IFCHR;
  return 0;
}

clock_t times (struct tms *buf)
{
  return -1;
}

int unlink (const char *__path )
{
  errno=ENOENT;
  return -1; 
}

int wait (int *status) 
{
  errno=ECHILD;
  return -1;
}

_READ_WRITE_RETURN_TYPE write (int __fd, const void *__buf, size_t __nbyte)
{
  char *cbuf = (char *)__buf;
  for (size_t i = 0; i < __nbyte; i++) {
    VideoPut(cbuf[i]);
  }
  return __nbyte;
}

int kprintf (const char *fmt, ...)
{
  int chars;
  va_list args;

  va_start(args, fmt);
  chars = vprintf(fmt, args);
  va_end(args);

  return chars;
}

