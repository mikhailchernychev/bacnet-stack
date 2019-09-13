/*
 * Candi implementations of replacements for Microsoft banned functions.
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>


/*
 * set the global abort() function pointer
 * this allows test code to over-ride abort() or for a 
 */
void _candi_s_abort(const char *msg, const char *file, const int line) {
  (void)msg;
  syslog(LOG_ERR, "%s:%d memcpy_s() *dest is NULL", file, line);
  abort();
}

/* store the abort() function in a global pointer
   this allows us to over-ride this pointer for testing */
void (*_candi_s_abort_func)(const char *msg, const char *file, const int line) = _candi_s_abort;

/* accessor to allow the calling application to override the abort() function */
void candi_s_abort_func_set(void (*_abort_func)(const char *msg, const char *file, const int line)) {
  _candi_s_abort_func = _abort_func;
}


/*
 * check that dest and src are not null
 * check that dest_n >= src_n
 *
 * file and line are used for error reporting and are set by the memcpy_s macro
 *
 * returns a pointer to dest, just like memcpy()
 */
void *_memcpy_s(void *dest,
               const size_t dest_n,
               const void *src,
               const size_t src_n,
               const char *file,
               const int line)
{
    if (! dest) {
        _candi_s_abort_func("%s:%d memcpy_s() *dest is NULL", file, line);
        return dest;
    }
    if (! src) {
        memset(dest, 0, dest_n);
        _candi_s_abort_func("%s:%d memcpy_s() *src is NULL", file, line);
        return dest;
    }
    if (dest_n < src_n) { 
        memset(dest, 0, dest_n);       
        _candi_s_abort_func("%s:%d memcpy_s() dest_n < src_n", file, line);
        return dest;
    }
    memcpy(dest, src, src_n);

    return dest;
}

/*
 * check that dest and src are not null
 * make sure dest string is null terminated
 *
 * file and line are used for error reporting and are set by the strcpy_s macro
 *
 * returns a pointer to dest, just like strcpy()
 */
char * _strcpy_s(char *dest,
                 size_t dest_n,
                 const char *src,
                 const char *file,
                 const int line)
{
    if (! dest) {
        _candi_s_abort_func("%s:%d strcpy_s() *dest is NULL", file, line);
        return dest;
    }
    if (! src) {
        memset(dest, 0, dest_n);
        _candi_s_abort_func("%s:%d strcpy_s() *src is NULL", file, line);
        return dest;
    }

    strncpy(dest, src, dest_n);
    /* make sure the dest is null-terminated
       (this differs from the microsoft behavior, which doesn't copy the string) */
    dest[dest_n - 1] = '\0';

    return dest;
}

/*
 * check that dest and src are not null
 * make sure dest string is null terminated
 *
 * file and line are used for error reporting and are set by the strcat_s macro
 *
 * returns a pointer to dest, just like strcat()
 */
char * _strcat_s(char *dest,
                 size_t dest_n,
                 const char *src,
                 const char *file,
                 const int line)
{
    if (! dest) {
        _candi_s_abort_func("%s:%d strcat_s() *dest is NULL", file, line);
        return dest;
    }
    if (! src) {
        memset(dest, 0, dest_n);
        _candi_s_abort_func("%s:%d strcat_s() *src is NULL", file, line);
        return dest;
    }

    strncat(dest, src, dest_n);
    /* make sure the dest is null-terminated
       (this differs from the microsoft behavior, which doesn't copy the string) */
    dest[dest_n] = '\0';

    return dest;
}

/*
 * check that dest is not null
 * make sure dest string is null terminated
 *
 * file and line are used for error reporting and are set by the sprintf_s macro
 *
 * returns the number of characters written or would have been written, like snprintf()
 */
int _sprintf_s(char *dest,
               size_t dest_n,
               const char *fmt,
               const char *file,
               const int line,
               ...)
{
    va_list ap;
    int n;

    if (! dest) {
        _candi_s_abort_func("%s:%d sprintf_s() *dest is NULL", file, line);
        return -1;
    }    

    va_start(ap, line);
    n = vsnprintf(dest, dest_n, fmt, ap);
    va_end(ap);

    // make sure string is null terminated
    dest[dest_n - 1] = '\0';

    return n;
}

/*
 * check that dest is not null
 * make sure dest string is null terminated
 *
 * file and line are used for error reporting and are set by the vsprintf_s macro
 *
 * returns the number of characters written or would have been written, like snprintf()
 */
int _vsprintf_s(char *dest,
                size_t dest_n,
                const char *fmt,
                va_list ap,
                const char *file,
                const int line)
{
    int n;

    if (! dest) {
        _candi_s_abort_func("%s:%d vsprintf_s() *dest is NULL", file, line);
        return -1;
    }    

    n = vsnprintf(dest, dest_n, fmt, ap);

    // make sure string is null terminated
    dest[dest_n - 1] = '\0';

    return n;
}


/*
 * safe replacement for gets
 * check that dest is not null
 * make sure dest string is null terminated
 *
 * file and line are used for error reporting and are set by the strcat_s macro
 *
 * returns a pointer to dest or NULL on error, just like gets()
 */
char * _gets_s(char *dest,
               size_t dest_n,
               const char *file,
               const int line)
{
    char *rc;

    if (! dest) {
        _candi_s_abort_func("%s:%d gets_s() *dest is NULL", file, line);
        return NULL;
    }    

    rc = fgets(dest, dest_n, STDIN_FILENO);
    if (rc == NULL) {
        /* make sure dest is zero'd out */
        memset(dest, 0, dest_n);

        /* don't error out, let user check errno */
        return NULL;
    }

    /* make sure string is null terminated */
    dest[dest_n - 1] = '\0';

    return rc;
}

/**
* For debugging on a mac call and alternative to clock_gettime()
*/

#ifdef __APPLE__
#include <mach/mach_time.h>
#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)

//static double orwl_timebase = 0.0;
//static uint64_t orwl_timestart = 0;

int clock_monotonic(struct timespec *res) {
    // be more careful in a multithreaded environement
//  if (!orwl_timestart) {
//    mach_timebase_info_data_t tb = { 0 };
//    mach_timebase_info(&tb);
//    orwl_timebase = tb.numer;
//    orwl_timebase /= tb.denom;
//    orwl_timestart = mach_absolute_time();
//  }
//  double diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
//  res->tv_sec = diff * ORWL_NANO;
//  res->tv_nsec = diff - (res->tv_sec * ORWL_GIGA);
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv)  {
        return rv;
    }
    res->tv_sec  = now.tv_sec;
    res->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#else
int clock_monotonic(struct timespec *res) {
  return clock_gettime(CLOCK_MONOTONIC, res);
}
#endif
