/*
 * Candi implementations of replacements for Microsoft banned functions.
 *
 */


#ifndef __candi_s_h
#define __candi_s_h

#ifdef __cplusplus

#include <cstdio>
#include <cstring>

extern "C" {
#endif  

#include <stddef.h>
#include <stdarg.h>

    /* these macro wrappers are used to pass file and line number for error reporting */
    
#define memcpy_s(dest, dest_n, src, src_n)                     \
    _memcpy_s(dest, dest_n, src, src_n, __FILE__, __LINE__)
    
#define strcpy_s(dest, dest_n, src)                            \
    _strcpy_s(dest, dest_n, src, __FILE__, __LINE__)
    
#define strcat_s(dest, dest_n, src)                            \
    _strcat_s(dest, dest_n, src, __FILE__, __LINE__)
    
#define sprintf_s(dest, dest_n, fmt, ...)                      \
    _sprintf_s(dest, dest_n, fmt, __FILE__, __LINE__, __VA_ARGS__)

#define vsprintf_s(dest, dest_n, fmt, ap)                      \
    _vsprintf_s(dest, dest_n, fmt, ap, __FILE__, __LINE__)

#define gets_s(dest, dest_n) \
    _gets_s(dest, dest_n, __FILE__, __LINE__)


    void _candi_s_abort(const char *msg, const char *file, const char *line);

    void candi_s_abort_func_set(void (*_candi_s_abort)(const char *msg, const char *file, const char *line));

    void * _memcpy_s(void *dest,
                     size_t dest_n,
                     const void *src,
                     size_t src_n,
                     const char *file,
                     const int line);

    char * _strcpy_s(char *dest,
                     size_t dest_n,
                     const char *src,
                     const char *file,
                     const int line);

    char * _strcat_s(char *dest,
                     size_t dest_n,
                     const char *src,
                     const char *file,
                     const int line);

    int _sprintf_s(char *dest,
                   size_t dest_n,
                   const char *fmt,
                   const char *file,
                   const int line,
                   ...);

    int _vsprintf_s(char *dest,
                    size_t dest_n,
                    const char *fmt,
                    va_list ap,
                    const char *file,
                    const int line);


#ifndef PHP_OPENZWAVE_VERSION
#ifndef PHP_NATIVE_DRIVER_VERSION

#ifndef __APPLE__
#pragma GCC poison memcpy
//#pragma GCC poison strncat strcat
//#pragma GCC poison strncpy strcpy
#pragma GCC poison sprintf
#endif

#endif
#endif

#ifdef __cplusplus    
}
#endif

#endif
