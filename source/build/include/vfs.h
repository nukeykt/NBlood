
#pragma once

#ifndef vfs_h_
#define vfs_h_

#include "compat.h"

#ifdef USE_PHYSFS

#include "physfs.h"

using buildvfs_FILE = PHYSFS_File *;
#define buildvfs_EOF (-1)
#define buildvfs_fread(p, s, n, fp) PHYSFS_readBytes((fp), (p), (s)*(n))
#define buildvfs_fwrite(p, s, n, fp) PHYSFS_writeBytes((fp), (p), (s)*(n))
#define buildvfs_fopen_read(fn) PHYSFS_openRead(fn)
#define buildvfs_fopen_write(fn) PHYSFS_openWrite(fn)
#define buildvfs_fopen_write_text(fn) PHYSFS_openWrite(fn)
#define buildvfs_fopen_append(fn) PHYSFS_openAppend(fn)
static inline int buildvfs_fgetc(buildvfs_FILE fp)
{
  unsigned char c;
  return buildvfs_fread(&c, 1, 1, fp) != 1 ? buildvfs_EOF : c;
}
static inline int buildvfs_fputc(char c, buildvfs_FILE fp)
{
    return PHYSFS_writeBytes(fp, &c, 1) != 1 ? buildvfs_EOF : c;
}
#define buildvfs_fclose(fp) PHYSFS_close(fp)
#define buildvfs_feof(fp) PHYSFS_eof(fp)
#define buildvfs_ftell(fp) PHYSFS_tell(fp)
#define buildvfs_fseek_abs(fp, o) PHYSFS_seek((fp), (o))
#define buildvfs_fseek_rel(fp, o) PHYSFS_seek((fp), PHYSFS_tell(fp) + (o))
#define buildvfs_rewind(fp) PHYSFS_seek((fp), 0)
#define buildvfs_fflush(fp) PHYSFS_flush(fp)

#define buildvfs_flength(fp) PHYSFS_fileLength(fp)

#define buildvfs_chdir(dir) (-1)
#define buildvfs_mkdir(dir, x) (!PHYSFS_mkdir(dir))
static inline char *buildvfs_getcwd(char *buf, size_t size)
{
    if (buf == nullptr || size == 0)
        return nullptr;

    buf[0] = '\0';
    return buf;
}

using buildvfs_fd = PHYSFS_File *;
#define buildvfs_fd_invalid (nullptr)
#define buildvfs_read(fd, p, s) PHYSFS_readBytes((fd), (p), (s))
#define buildvfs_write(fd, p, s) PHYSFS_writeBytes((fd), (p), (s))
#define buildvfs_open_read(fn) PHYSFS_openRead(fn)
#define buildvfs_open_write(fn) PHYSFS_openWrite(fn)
#define buildvfs_open_append(fn) PHYSFS_openAppend(fn)
#define buildvfs_close(fd) PHYSFS_close(fd)
#define buildvfs_tell(fd) PHYSFS_tell(fd)
static inline int64_t buildvfs_lseek_abs(buildvfs_fd fd, int64_t o)
{
    PHYSFS_seek(fd, o);
    return PHYSFS_tell(fd);
}
static inline int64_t buildvfs_lseek_rel(buildvfs_fd fd, int64_t o)
{
    PHYSFS_seek(fd, PHYSFS_tell(fd) + o);
    return PHYSFS_tell(fd);
}

#define buildvfs_length(fd) PHYSFS_fileLength(fd)
#define buildvfs_exists(fn) PHYSFS_exists(fn)
#define buildvfs_isdir(path) PHYSFS_isDirectory(path)
#define buildvfs_unlink(path) PHYSFS_delete(path)

#else

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
#endif

using buildvfs_FILE = FILE *;
#define buildvfs_EOF EOF
#define buildvfs_fread(p, s, n, fp) fread((p), (s), (n), (fp))
#define buildvfs_fwrite(p, s, n, fp) fwrite((p), (s), (n), (fp))
#define buildvfs_fopen_read(fn) fopen((fn), "rb")
#define buildvfs_fopen_write(fn) fopen((fn), "wb")
#define buildvfs_fopen_write_text(fn) fopen((fn), "w")
#define buildvfs_fopen_append(fn) fopen((fn), "ab+")
#define buildvfs_fgetc(fp) fgetc(fp)
#define buildvfs_fputc(c, fp) fputc((c), (fp))
#define buildvfs_fgets(str, size, fp) fgets((str), (size), (fp))
#define buildvfs_fclose(fp) fclose(fp)
#define buildvfs_feof(fp) feof(fp)
#define buildvfs_ftell(fp) ftell(fp)
#define buildvfs_fseek_abs(fp, o) fseek((fp), (o), SEEK_SET)
#define buildvfs_fseek_rel(fp, o) fseek((fp), (o), SEEK_CUR)
#define buildvfs_fseek_end(fp) fseek((fp), 0, SEEK_END)
#define buildvfs_rewind(fp) rewind(fp)
#define buildvfs_fflush(fp) fflush(fp)

static FORCE_INLINE int64_t buildvfs_length(int fd)
{
#ifdef _WIN32
    return filelength(fd);
#else
    struct stat st;
    return fstat(fd, &st) < 0 ? -1 : st.st_size;
#endif
}

#define buildvfs_chdir(dir) chdir(dir)
#define buildvfs_mkdir(dir, x) Bmkdir(dir, x)
#define buildvfs_getcwd(buf, size) getcwd((buf), (size))

using buildvfs_fd = int;
#define buildvfs_fd_invalid (-1)
#define buildvfs_read(fd, p, s) read((fd), (p), (s))
#define buildvfs_write(fd, p, s) write((fd), (p), (s))
#define buildvfs_open_read(fn) open((fn), O_RDONLY|O_BINARY)
#define buildvfs_open_write(fn) open((fn), O_BINARY|O_TRUNC|O_CREAT|O_WRONLY, S_IREAD|S_IWRITE)
// #define buildvfs_open_append(fn) todo(fn)
#define buildvfs_close(fd) close(fd)
#define buildvfs_tell(fd) lseek((fd), 0, SEEK_CUR)
#define buildvfs_lseek_abs(fd, o) lseek((fd), (o), SEEK_SET)
#define buildvfs_lseek_rel(fd, o) lseek((fd), (o), SEEK_CUR)

static FORCE_INLINE int64_t buildvfs_flength(FILE * f)
{
#ifdef _WIN32
    return filelength(_fileno(f));
#else
    return buildvfs_length(fileno(f));
#endif
}

static FORCE_INLINE int buildvfs_exists(char const* path)
{
#ifdef _WIN32
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct Bstat st;
    return !Bstat(path, &st);
#endif
}

static FORCE_INLINE int buildvfs_isdir(char const *path)
{
    struct Bstat st;
    return (Bstat(path, &st) ? 0 : (st.st_mode & S_IFDIR) == S_IFDIR);
}
#define buildvfs_unlink(path) unlink(path)

#endif

#define MAYBE_FCLOSE_AND_NULL(fileptr) do { \
    if (fileptr) { buildvfs_fclose(fileptr); fileptr = buildvfs_FILE{}; } \
} while (0)

static FORCE_INLINE void buildvfs_fputstrptr(buildvfs_FILE fp, char const * str)
{
    if (fp)
        buildvfs_fwrite(str, 1, strlen(str), fp);
}

static FORCE_INLINE void buildvfs_fputs(char const * str, buildvfs_FILE fp)
{
    if (fp)
        buildvfs_fwrite(str, 1, strlen(str), fp);
}

template <size_t N>
static FORCE_INLINE void buildvfs_fputstr(buildvfs_FILE fp, char const (&str)[N])
{
    if (fp)
        buildvfs_fwrite(&str, 1, N-1, fp);
}


#ifdef __cplusplus
extern "C" {
#endif

extern char toupperlookup[256];

extern char *kpzbuf;
extern int32_t kpzbufsiz;
extern int32_t kpzbufload(const char *);

#ifdef USE_PHYSFS
using buildvfs_kfd = PHYSFS_File *;
#define buildvfs_kfd_invalid (nullptr)

extern int32_t pathsearchmode;	// 0 = gamefs mode (default), 1 = localfs mode (editor's mode)

#define addsearchpath(a) addsearchpath_user(a, 0)
static inline int32_t addsearchpath_user(const char *p, int32_t)
{
    return PHYSFS_mount(p, NULL, 1) == 0 ? -1 : 0;
}

static inline int32_t removesearchpath(const char *p)
{
    return PHYSFS_unmount(p);
}
static inline void removesearchpaths_withuser(int32_t)
{
    // TODO
}


int32_t findfrompath(const char *fn, char **where);
buildvfs_kfd openfrompath(const char *fn, int32_t flags, int32_t mode);
buildvfs_FILE fopenfrompath(const char *fn, const char *mode);


extern int32_t numgroupfiles;
void uninitgroupfile(void);


static inline int initgroupfile(const char *filename)
{
    return PHYSFS_mount(filename, NULL, 1) == 0 ? -1 : 0;
}


#define kread(fd, p, s) PHYSFS_readBytes((fd), (p), (s))
#define kwrite(fd, p, s) PHYSFS_writeBytes((fd), (p), (s))
#define kopen4load(fn, searchfirst) PHYSFS_openRead(fn)
#define ktell(fd) PHYSFS_tell(fd)
#define kfilelength(fd) PHYSFS_fileLength(fd)


static inline void kclose(buildvfs_kfd handle)
{
    PHYSFS_close(handle);
}

#define kread_and_test(handle, buffer, leng) EDUKE32_PREDICT_FALSE(kread((handle), (buffer), (leng)) != (leng))
extern int32_t klseek(buildvfs_kfd handle, int32_t offset, int32_t whence);
#define klseek_and_test(handle, offset, whence) EDUKE32_PREDICT_FALSE(klseek((handle), (offset), (whence)) < 0)

static inline void krename(int32_t, int32_t, const char *)
{
}

#else
using buildvfs_kfd = int32_t;
#define buildvfs_kfd_invalid (-1)

extern int32_t pathsearchmode;	// 0 = gamefs mode (default), 1 = localfs mode (editor's mode)
char *listsearchpath(int32_t initp);
int32_t     addsearchpath_user(const char *p, int32_t user);
#define addsearchpath(a) addsearchpath_user(a, 0)
int32_t     removesearchpath(const char *p);
void     removesearchpaths_withuser(int32_t usermask);
int32_t		findfrompath(const char *fn, char **where);
buildvfs_kfd     openfrompath(const char *fn, int32_t flags, int32_t mode);
buildvfs_FILE fopenfrompath(const char *fn, const char *mode);

extern char g_modDir[BMAX_PATH];
extern int32_t numgroupfiles;
int initgroupfile(const char *filename);
void	uninitgroupfile(void);
buildvfs_kfd	kopen4load(const char *filename, char searchfirst);	// searchfirst: 0 = anywhere, 1 = first group, 2 = any group
buildvfs_kfd	kopen4loadfrommod(const char* filename, char searchfirst);
int32_t	kread(buildvfs_kfd handle, void *buffer, int32_t leng);
#define kread_and_test(handle, buffer, leng) EDUKE32_PREDICT_FALSE(kread((handle), (buffer), (leng)) != (int32_t)(leng))
int32_t	klseek(buildvfs_kfd handle, int32_t offset, int32_t whence);
#define klseek_and_test(handle, offset, whence) EDUKE32_PREDICT_FALSE(klseek((handle), (offset), (whence)) < 0)
int32_t	kfilelength(buildvfs_kfd handle);
int32_t	ktell(buildvfs_kfd handle);
void	kclose(buildvfs_kfd handle);

void krename(int32_t crcval, int32_t filenum, const char *newname);
char const * kfileparent(int32_t handle);
#endif

extern int32_t kpzbufloadfil(buildvfs_kfd);

#ifdef WITHKPLIB
int32_t cache1d_file_fromzip(buildvfs_kfd fil);
#endif

enum {
    BUILDVFS_FIND_FILE     = 1,
    BUILDVFS_FIND_DIR      = 2,
    BUILDVFS_FIND_DRIVE    = 4,
    BUILDVFS_FIND_NOCURDIR = 8,

    BUILDVFS_OPT_NOSTACK = 0x100,

    // the lower the number, the higher the priority
    BUILDVFS_SOURCE_DRIVE  = 0,
    BUILDVFS_SOURCE_CURDIR = 1,
    BUILDVFS_SOURCE_PATH   = 2,  // + path stack depth
    BUILDVFS_SOURCE_ZIP    = 0x7ffffffe,
    BUILDVFS_SOURCE_GRP    = 0x7fffffff,
};

typedef struct _BUILDVFS_FIND_REC {
    char *name;
    int32_t type, source;
    struct _BUILDVFS_FIND_REC *next, *prev, *usera, *userb;
} BUILDVFS_FIND_REC;
int32_t klistaddentry(BUILDVFS_FIND_REC **rec, const char *name, int32_t type, int32_t source);
void klistfree(BUILDVFS_FIND_REC *rec);
BUILDVFS_FIND_REC *klistpath(const char *path, const char *mask, int type);

extern int32_t lz4CompressionLevel;
int32_t     kdfread(void *buffer, int dasizeof, int count, buildvfs_kfd fil);
int32_t kdfread_LZ4(void *buffer, int dasizeof, int count, buildvfs_kfd fil);
void     dfwrite(const void *buffer, int dasizeof, int count, buildvfs_FILE fil);
void dfwrite_LZ4(const void *buffer, int dasizeof, int count, buildvfs_FILE fil);

#ifdef __cplusplus
}
#endif

#endif // vfs_h_
