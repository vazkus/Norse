/**************************************************************************//*****
 * @file     stdio.c
 * @brief    Implementation of newlib syscall
 ********************************************************************************/
 
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#undef errno
extern int errno;
extern int  _end;
 
caddr_t _sbrk ( int incr )
{
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;
 
  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;
 
  heap += incr;
 
  return (caddr_t) prev_heap;
}
 
int link(char *old, char *new) {
    (void)old;
    (void)new;
return -1;
}
 
int _close(int file)
{
  (void)file;
  return -1;
}
 
int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}
 
int _isatty(int file)
{
  (void)file;
  return 1;
}
 
int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}
 
int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}
 
int _write(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  return len;
}
 
 
/* --------------------------------- End Of File ------------------------------ */
