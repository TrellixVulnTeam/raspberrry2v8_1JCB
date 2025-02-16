#pragma once

// Use libc++ in Emscripten mode
#define __EMSCRIPTEN__

#include <stdlib.h>
#include <bits/alltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Add missing type
typedef struct __locale_struct* locale_t;
locale_t __cloc(void);

static inline locale_t duplocale(locale_t) {
  return NULL;
}

static inline void freelocale(locale_t) {
}

static inline locale_t newlocale(int, const char *, locale_t) {
  return NULL;
}

static inline locale_t uselocale(locale_t) {
  return NULL;
}

// Use linux as platform
#ifndef __linux__
#define __linux__
#endif

// Enforce no exceptions
#ifndef _LIBCPP_NO_EXCEPTIONS
#define _LIBCPP_NO_EXCEPTIONS
#endif

#define _LIBCPP_HAS_NO_THREADS
#define _LIBCPP_BUILD_STATIC
#define _LIBCPP_NO_RTTI

// These are quick-and-dirty hacks to make things pretend to work
// We should not use locale functions in kernel
static inline
long long strtoll_l(const char *__nptr, char **__endptr,
    int __base, locale_t __loc) {
  return strtoll(__nptr, __endptr, __base);
}
static inline
long strtol_l(const char *__nptr, char **__endptr,
    int __base, locale_t __loc) {
  return strtol(__nptr, __endptr, __base);
}

#if 0	//already inside stdlib.h
static inline
long double strtold_l(const char *__nptr, char **__endptr,
    locale_t __loc) {
  return strtold(__nptr, __endptr);
}
#endif
static inline
unsigned long long strtoull_l(const char *__nptr, char **__endptr,
    int __base, locale_t __loc) {
  return strtoull(__nptr, __endptr, __base);
}
static inline
unsigned long strtoul_l(const char *__nptr, char **__endptr,
    int __base, locale_t __loc) {
  return strtoul(__nptr, __endptr, __base);
}

#ifdef __cplusplus
}
#endif

