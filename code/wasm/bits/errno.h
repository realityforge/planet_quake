extern int errno;
#define EMSCRIPTEN_NO_ERRNO 1
#define EINVAL ((short)28)
int *__errno_location(void);
#define errno (*__errno_location())
#define PATH_MAX 1024
#define ENOMEM          9971
#define EFAULT          14
#define EBADF            9

