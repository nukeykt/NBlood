#ifndef VOIDWRAP_H_
#define VOIDWRAP_H_

#include <inttypes.h>
#include <stdbool.h>

#ifdef _WIN32
# include <windows.h>
# ifdef VOIDWRAP_ISEXPORTING
#  define VOIDWRAP_API __declspec(dllexport)
# else
#  define VOIDWRAP_API __declspec(dllimport)
# endif
typedef HINSTANCE VW_LIBHANDLE;
# define Voidwrap_LoadLibrary(lib) LoadLibrary((lib))
# define Voidwrap_GetSymbol(lib_handle, symbol) GetProcAddress((lib_handle), (symbol))
#else
# include <dlfcn.h>
# define VOIDWRAP_API
typedef void * VW_LIBHANDLE;
# define Voidwrap_LoadLibrary(lib) dlopen((lib), RTLD_NOW|RTLD_GLOBAL)
# define Voidwrap_GetSymbol(lib_handle, symbol) dlsym((lib_handle), (symbol))
#endif

// #define VWDEBUG

// Callback types
typedef void (*VW_CALLBACK_NOPARAM)(void);
typedef void (*VW_CALLBACK_INT32)(int32_t);
typedef void (*VW_CALLBACK_CHARPTR)(char const *);

// Callback setup function types
typedef void (*VW_SETCALLBACK_NOPARAM)(VW_CALLBACK_NOPARAM);
typedef void (*VW_SETCALLBACK_INT32)(VW_CALLBACK_INT32);
typedef void (*VW_SETCALLBACK_CHARPTR)(VW_CALLBACK_CHARPTR);

// Function types
typedef bool (*VW_BOOL)(void);
#ifdef VWSCREENSHOT
typedef bool (*VW_BOOL_SCREENSHOT)(char * filepath, int32_t width, int32_t height);
#endif
typedef void (*VW_VOID)(void);
typedef int32_t (*VW_INT32)(void);

#endif
