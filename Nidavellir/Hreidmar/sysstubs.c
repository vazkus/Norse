#include <errno.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int _getpid()
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return(-1);
}
void abort()
{
    //disable_interrupts();
    // Break in the debugger
    asm volatile ("bkpt\n\t");
    while (1);
}

/*
void __assert_func(const char* file, int line, const char* func, const char* expr)
{
    abort();
}

unsigned long debug_lock()
{
    return disable_irqs();
}

void debug_unlock(unsigned long s)
{
    restore_interrupt_state(s);
}

static Mutex malloc_mutex;

void __malloc_lock(struct _reent* r)
{
    if (scheduler_is_active())
        task_mutex_lock(&malloc_mutex);
}

void __malloc_unlock(struct _reent* r)
{
    if (scheduler_is_active())
        task_mutex_unlock(&malloc_mutex);
}
*/

