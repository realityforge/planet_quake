#ifndef __SYSCALL_ARCH_H__
#define __SYSCALL_ARCH_H__

#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) 0, __SYSCALL_LL_E((x))

#ifdef __cplusplus
extern "C" {
#endif

/* Causes the final import in the wasm binary be named "env.sys_<name>" */
#define SYS_IMPORT(NAME) EM_IMPORT(__sys_##NAME)


long __syscall0(long n);
long __syscall1(long n, long a1);
long __syscall2(long n, long a1, long a2);
long __syscall3(long n, long a1, long a2, long a3);
long __syscall4(long n, long a1, long a2, long a3, long a4);
long __syscall5(long n, long a1, long a2, long a3, long a4, long a5);
long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6);


#ifdef __cplusplus
}
#endif

#endif
