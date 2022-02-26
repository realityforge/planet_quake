//#include "../wasm/wasi/api.h"
//#include "../wasm/wasi/wasi-helpers.h"
#define fopen Sys_FOpen
#define INT_MIN  (-1-0x7fffffff)
#define INT_MAX  0x7fffffff
#define UINT_MAX 0xffffffffU
#define LONG_MIN (-LONG_MAX-1)
#define EINVAL ((short)28)
int *__errno_location(void);
#define errno (*__errno_location())
#define PATH_MAX 1024
#define ENOMEM 9971
#define SET_ERRNO() { errno = ENOMEM; }
#define FD_SETSIZE 1024
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)
#define a_crash Sys_Exit
#define abort Sys_Exit
#define UINT_MAX 0xffffffffU
#define RAND_MAX (0x7fffffff)
typedef qboolean bool;
typedef int off_t;
typedef unsigned long uintptr_t;
typedef long intptr_t;
typedef long ptrdiff_t;
typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;
typedef signed int   int64_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned int    uint64_t;
typedef signed int time_t;
struct _IO_FILE { char __x; };
typedef struct _IO_FILE FILE;
typedef __builtin_va_list va_list;
typedef __builtin_va_list __isoc_va_list;
typedef __SIZE_TYPE__ size_t;
typedef struct {
	unsigned long fds_bits[FD_SETSIZE / 8 / sizeof(long)];
} fd_set;
struct timeval { time_t tv_sec; time_t tv_usec; };

void srand (unsigned);
void *malloc (size_t);
void *calloc (size_t, size_t);
//void *realloc (void *, size_t);
void free (void *);

typedef int (*cmpfun)(const void *, const void *);
void qsort(void *base, size_t nel, size_t width, cmpfun cmp);
size_t strlen (const char *);
void *memmove (void *, const void *, size_t);
int memcmp (const void *, const void *, size_t);
void *memchr (const void *, int, size_t);
char *strcpy (char *, const char *);
char *strncpy (char *, const char *, size_t);
char *strcat (char *, const char *);
char *strncat (char *, const char *, size_t);
int strcmp (const char *, const char *);
int strncmp (const char *, const char *, size_t);
char *strchr (const char *, int);
char *strrchr (const char *, int);
char *strpbrk (const char *, const char *);
char *strstr (const char *, const char *);

#define Com_Memset memset
#define Com_Memcpy memcpy
#define Snd_Memset Com_Memset
void *memset (void *, int, size_t);
void *memcpy (void *, const void *, size_t);

// this only works because it's a single char?
int tolower( int c );
void Sys_Error( const char *error, ... );
void Sys_Error( const char *error, ... );

typedef void*jmp_buf;
#define setjmp(x) (qfalse)
#define longjmp(x, y)
//void longjmp( void *buf, int ret );
//int setjmp( void *buf );
int CL_Download( const char *cmd, const char *pakname, int autoDownload );


#define offsetof(type, member) __builtin_offsetof(type, member)
#define ftell Sys_FTell
#define fseek Sys_FSeek
#define fclose Sys_FClose
#define fwrite Sys_FWrite
#define fflush Sys_FFlush
#define fread Sys_FRead
#define rename Sys_Rename
#define remove Sys_Remove

long Sys_FTell(FILE *f);
int Sys_FSeek(FILE *f, long off, int whence);
int Sys_FClose(FILE *f);
size_t Sys_FWrite(const void *restrict src, size_t size, size_t nmemb, FILE *restrict f);
int Sys_FFlush(FILE *f);
size_t Sys_FRead(void *restrict destv, size_t size, size_t nmemb, FILE *restrict f);
size_t Sys_Remove(const char *);
size_t Sys_Rename(const char *, const char *);
uint16_t ntohs(uint16_t n);

int ED_vsprintf( char *str, const char *format, va_list ap );
int Q_sscanf( const char *buffer, const char *fmt, ... ) ;
#define vsnprintf(x, y, z, ...) ED_vsprintf(x, z, __VA_ARGS__)
#define vsprintf(x, y, ...) ED_vsprintf(x, y, __VA_ARGS__)
#define snprintf(x, y, z, ...) Com_sprintf(x, y, z, __VA_ARGS__)
#define sprintf(x, ...) Com_sprintf(x, 1024, __VA_ARGS__)
#define sscanf Q_sscanf
#define Q_vsnprintf vsnprintf

int assert_fail(const char *exprStr, const char *file, int line, const char *func);

#define assert(expr) (!(expr)?assert_fail(#expr, __FILE__, __LINE__, __func__):0)

int fprintf(FILE *f, const char *fmt, ...);

int atoi(const char *);

double strtod(const char *, char **);

float strtof(const char *, char **);

double atof(const char *);

long atol(const char *);
FILE *popen(const char *cmd, const char *mode);
FILE *popen(const char *cmd, const char *mode);

void Sys_SockaddrToString(char *dest, int destlen, const void *input);

int Sys_StringToSockaddr(const char *s, void *sadr, int sadr_len);
#define Sys_StringToSockaddr(x, y, z, w) Sys_StringToSockaddr(x, y, z)

void *gethostbyname (const char *);

void  Sys_Print(const char *s);

struct in_addr { uint32_t s_addr; };

struct sockaddr_in {
	int sin_family;
	uint16_t sin_port;
	struct in_addr sin_addr;
	uint8_t sin_zero[8];
};

double cos(double);
double sin(double);
double tan(double);
double acos(double);
double asin(double);
double atan(double);
double exp(double);
double log(double);
double sqrt(double);
double fabs(double);
double ceil(double);
double floor(double);

float cosf(float);
float sinf(float);
float tanf(float);
float acosf(float);
float asinf(float);
float atanf(float);
float expf(float);
float logf(float);
float sqrtf(float);

float fabsf(float);
long double fabsl(long double x);

float ceilf(float);
float floorf(float);

double atan2(double, double);
double pow(double, double);

float atan2f(float, float);
float powf(float, float);

int rand(void);
double round(double);
float roundf(float);
double rint(double);
float rintf(float);
int abs(int);
long labs(long);

struct tm {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
};

typedef struct tm qtime_t;
char *ctime(const time_t *timer);
struct tm *localtime(const time_t * t);
time_t time(time_t *t);
time_t mktime (struct tm *);

char *asctime (const struct tm *);

