
#include "compat.h"
#include "baselayer.h"

#define SJSON_IMPLEMENT

#define sjson_malloc(user, size)       ((UNREFERENCED_PARAMETER(user)), Xmalloc(size))
#define sjson_free(user, ptr)          ((UNREFERENCED_PARAMETER(user)), Xfree(ptr))
#define sjson_realloc(user, ptr, size) ((UNREFERENCED_PARAMETER(user)), Xrealloc(ptr, size))

#define sjson_out_of_memory()		   do { sjson_assert(0 && "Out of memory"); Bexit(EXIT_FAILURE); } while(0)

#include "sjson.h"
