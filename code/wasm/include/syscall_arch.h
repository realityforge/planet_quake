#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

#pragma once
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// You can use these functions by passing format string to arg_sigs.
// Note that `code` requires you to provide a const C string known at compile time,
// otherwise the "unable to find data for ASM/EM_JS const" error will be thrown.
// https://github.com/WebAssembly/binaryen/blob/51c8f2469f8fd05197b7694c65041b1567f2c6b5/src/wasm/wasm-emscripten.cpp#L183

// C++ needs the nothrow attribute so -O0 doesn't lower these calls as invokes.
__attribute__((nothrow))
int emscripten_asm_const_int(const char* code, const char* arg_sigs, ...);
__attribute__((nothrow))
double emscripten_asm_const_double(const char* code, const char* arg_sigs, ...);

__attribute__((nothrow))
int emscripten_asm_const_int_sync_on_main_thread(
  const char* code, const char* arg_sigs, ...);
__attribute__((nothrow))
double emscripten_asm_const_double_sync_on_main_thread(
  const char* code, const char* arg_sigs, ...);

__attribute__((nothrow))
void emscripten_asm_const_async_on_main_thread(
  const char* code, const char* arg_sigs, ...);

#ifdef __cplusplus
#define _EM_JS_CPP_BEGIN extern "C" {
#define _EM_JS_CPP_END   }
#else // __cplusplus
#define _EM_JS_CPP_BEGIN
#define _EM_JS_CPP_END
#endif // __cplusplus

#define EM_JS(ret, name, params, ...)                                                              \
  _EM_JS_CPP_BEGIN                                                                                 \
  ret name params EM_IMPORT(name);                                                                 \
  EMSCRIPTEN_KEEPALIVE                                                                             \
  __attribute__((section("em_js"), aligned(1))) char __em_js__##name[] =                           \
    #params "<::>" #__VA_ARGS__;                                                                   \
  _EM_JS_CPP_END

#define CALL_JS_1(cname, jsname, type, casttype) \
  EM_JS(type, JS_##cname, (type x), { return jsname(x) }); \
  type cname(type x) { return JS_##cname((casttype)x); }

#define CALL_JS_1_TRIPLE(cname, jsname) \
  CALL_JS_1(cname, jsname, double, double) \
  CALL_JS_1(cname##f, jsname, float, float)
  
#define CALL_JS_2(cname, jsname, type, casttype) \
  EM_JS(type, JS_##cname, (type x, type y), { return jsname(x, y) }); \
  type cname(type x, type y) { return JS_##cname((casttype)x, (casttype)y); }

#define CALL_JS_2_TRIPLE(cname, jsname) \
  CALL_JS_2(cname, jsname, double, double) \
  CALL_JS_2(cname##f, jsname, float, float)

#define CALL_JS_1_IMPL(cname, type, casttype, impl) \
  EM_JS(type, JS_##cname, (type x), impl); \
  type cname(type x) { return JS_##cname((casttype)x); }

#define CALL_JS_1_IMPL_TRIPLE(cname, impl) \
  CALL_JS_1_IMPL(cname, double, double, impl) \
  CALL_JS_1_IMPL(cname##f, float, float, impl)

// In wasm backend, we need to call the emscripten_asm_const_* functions with
// the C vararg calling convention, because we will call it with a variety of
// arguments, but need to generate a coherent import for the wasm module before
// binaryen can run over it to fix up any calls to emscripten_asm_const_*.  In
// order to read from a vararg buffer, we need to know the signatures to read.
// We can use compile-time trickery to generate a format string, and read that
// in JS in order to correctly handle the vararg buffer.

#ifndef __cplusplus

// We can use the generic selection C11 feature (that clang supports pre-C11
// as an extension) to emulate function overloading in C.
// All pointer types should go through the default case.
#define _EM_ASM_SIG_CHAR(x) _Generic((x), \
    float: 'f', \
    double: 'd', \
    int: 'i', \
    unsigned: 'i', \
    default: 'i')

// This indirection is needed to allow us to concatenate computed results, e.g.
//   #define BAR(N) _EM_ASM_CONCATENATE(FOO_, N)
//   BAR(3) // rewritten to BAR_3
// whereas using ## or _EM_ASM_CONCATENATE_ directly would result in BAR_N
#define _EM_ASM_CONCATENATE(a, b) _EM_ASM_CONCATENATE_(a, b)
#define _EM_ASM_CONCATENATE_(a, b) a##b

// Counts arguments. We use $$ as a sentinel value to enable using ##__VA_ARGS__
// which omits a comma in the event that we have 0 arguments passed, which is
// necessary to keep the count correct.
#define _EM_ASM_COUNT_ARGS_EXP(_$,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,n,...) n
#define _EM_ASM_COUNT_ARGS(...) \
    _EM_ASM_COUNT_ARGS_EXP($$,##__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

// Find the corresponding char for each argument.
#define _EM_ASM_ARG_SIGS_0(x, ...)
#define _EM_ASM_ARG_SIGS_1(x, ...) _EM_ASM_SIG_CHAR(x),
#define _EM_ASM_ARG_SIGS_2(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_1(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_3(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_2(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_4(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_3(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_5(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_4(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_6(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_5(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_7(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_6(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_8(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_7(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_9(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_8(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_10(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_9(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_11(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_10(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_12(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_11(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_13(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_12(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_14(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_13(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_15(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_14(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_16(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_15(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_(N, ...) \
    ((char[]){ _EM_ASM_CONCATENATE(_EM_ASM_ARG_SIGS_,N)(__VA_ARGS__) '\0' })

#define _EM_ASM_ARG_SIGS(...) \
    _EM_ASM_ARG_SIGS_(_EM_ASM_COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__)

// We lead with commas to avoid adding an extra comma in the 0-argument case.
#define _EM_ASM_PREP_ARGS(...) , _EM_ASM_ARG_SIGS(__VA_ARGS__), ##__VA_ARGS__

#else // __cplusplus

// C++ needs to support vararg template parameter packs, e.g. like in
// tests/core/test_em_asm_parameter_pack.cpp. Because of that, a macro-only
// approach doesn't work (a macro applied to a parameter pack would expand
// incorrectly). So we can use a template class instead to build a temporary
// buffer of characters.

// As emscripten is require to build successfully with -std=c++03, we cannot
// use std::tuple or std::integral_constant. Using C++11 features is only a
// warning in modern Clang, which are ignored in system headers.
template<typename, typename = void> struct __em_asm_sig {};
template<> struct __em_asm_sig<float> { static const char value = 'd'; };
template<> struct __em_asm_sig<double> { static const char value = 'd'; };
template<> struct __em_asm_sig<char> { static const char value = 'i'; };
template<> struct __em_asm_sig<signed char> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned char> { static const char value = 'i'; };
template<> struct __em_asm_sig<short> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned short> { static const char value = 'i'; };
template<> struct __em_asm_sig<int> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned int> { static const char value = 'i'; };
template<> struct __em_asm_sig<long> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned long> { static const char value = 'i'; };
template<> struct __em_asm_sig<bool> { static const char value = 'i'; };
template<> struct __em_asm_sig<wchar_t> { static const char value = 'i'; };
template<typename T> struct __em_asm_sig<T*> { static const char value = 'i'; };

// Explicit support for enums, they're passed as int via variadic arguments.
template<bool> struct __em_asm_if { };
template<> struct __em_asm_if<true> { typedef void type; };
template<typename T> struct __em_asm_sig<T, typename __em_asm_if<__is_enum(T)>::type> {
    static const char value = 'i';
};

// Instead of std::tuple
template<typename... Args>
struct __em_asm_type_tuple {};

// Instead of std::make_tuple
template<typename... Args>
__em_asm_type_tuple<Args...> __em_asm_make_type_tuple(Args... args) {
    return {};
}

template<typename>
struct __em_asm_sig_builder {};

template<typename... Args>
struct __em_asm_sig_builder<__em_asm_type_tuple<Args...> > {
  static const char buffer[sizeof...(Args) + 1];
};

template<typename... Args>
const char __em_asm_sig_builder<__em_asm_type_tuple<Args...> >::buffer[] = { __em_asm_sig<Args>::value..., 0 };

// We move to type level with decltype(make_tuple(...)) to avoid double
// evaluation of arguments. Use __typeof__ instead of decltype, though,
// because the header should be able to compile with clang's -std=c++03.
#define _EM_ASM_PREP_ARGS(...) \
    , __em_asm_sig_builder<__typeof__(__em_asm_make_type_tuple(__VA_ARGS__))>::buffer, ##__VA_ARGS__
#endif // __cplusplus

// Note: If the code block in the EM_ASM() family of functions below contains a comma,
// then wrap the whole code block inside parentheses (). See tests/core/test_em_asm_2.cpp
// for example code snippets.

#define CODE_EXPR(code) (__extension__({           \
    __attribute__((section("em_asm"), aligned(1))) \
    static const char x[] = code;                  \
    x;                                             \
  }))

// Runs the given JavaScript code on the calling thread (synchronously), and returns no value back.
#define EM_ASM0(code) ((void)emscripten_asm_const_int(CODE_EXPR(#code), 'v'))
#define EM_ASM(code, ...) ((void)emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code on the calling thread (synchronously), and returns an integer back.
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code on the calling thread (synchronously), and returns a double back.
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns no value back.
// Call this function for example to access DOM elements in a pthread when building with -s USE_PTHREADS=1.
// Avoid calling this function in performance sensitive code, because this will effectively sleep the
// calling thread until the main browser thread is able to service the proxied function call. If you have
// multiple MAIN_THREAD_EM_ASM() code blocks to call in succession, it will likely be much faster to
// coalesce all the calls to a single MAIN_THREAD_EM_ASM() block. If you do not need synchronization nor
// a return value back, consider using the function MAIN_THREAD_ASYNC_EM_ASM() instead, which will not block.
// In single-threaded builds (including proxy-to-worker), MAIN_THREAD_EM_ASM*()
// functions are direct aliases to the corresponding EM_ASM*() family of functions.
#define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code synchronously on the main browser thread, and returns an integer back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_INT(code, ...) emscripten_asm_const_int_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns a double back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Asynchronously dispatches the given JavaScript code to be run on the main browser thread.
// If the calling thread is the main browser thread, then the specified JavaScript code is executed
// synchronously. Otherwise an event will be queued on the main browser thread to execute the call
// later (think postMessage()), and this call will immediately return without waiting. Be sure to guard any accesses to shared memory on the heap inside the JavaScript code with appropriate locking.
#define MAIN_THREAD_ASYNC_EM_ASM(code, ...) ((void)emscripten_asm_const_async_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Old forms for compatibility, no need to use these.
// Replace EM_ASM_, EM_ASM_ARGS and EM_ASM_INT_V with EM_ASM_INT,
// and EM_ASM_DOUBLE_V with EM_ASM_DOUBLE.
#define EM_ASM_(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_INT_V(code) EM_ASM_INT(code)
#define EM_ASM_DOUBLE_V(code) EM_ASM_DOUBLE(code)

#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

#define EM_IMPORT(NAME) __attribute__((import_module("env"), import_name(#NAME)))

#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) 0, __SYSCALL_LL_E((x))

/* Causes the final import in the wasm binary be named "env.sys_<name>" */
#define SYS_IMPORT(NAME) EM_IMPORT(__sys_##NAME)


static inline long __syscall0(long n)
{
  return 0;
}
static inline long __syscall1(long n, long a1)
{
  return 0;
}
static inline long __syscall2(long n, long a1, long a2)
{
  return 0;
}
static inline long __syscall3(long n, long a1, long a2, long a3)
{
  return 0;
}
static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
  return 0;
}
static inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
  return 0;
}
static inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
  return 0;
}

long SYS_IMPORT(exit) __sys_exit(long exit_code) {
  return __syscall(SYS_exit, long exit_code);
}
long SYS_IMPORT(open) __sys_open(long path, long flags, ...) {
  return __syscall(SYS_open, long path, long flags, ...);
} // mode is optional
long SYS_IMPORT(link) __sys_link(long oldpath, long newpath) {
  return __syscall(SYS_link, long oldpath, long newpath);
}
long SYS_IMPORT(unlink) __sys_unlink(long path) {
  return __syscall(SYS_unlink, long path);
}
long SYS_IMPORT(chdir) __sys_chdir(long path) {
  return __syscall(SYS_chdir, long path);
}
long SYS_IMPORT(mknod) __sys_mknod(long path, long mode, long dev) {
  return __syscall(SYS_mknod, long path, long mode, long dev);
}
long SYS_IMPORT(chmod) __sys_chmod(long path, long mode) {
  return __syscall(SYS_chmod, long path, long mode);
}
long SYS_IMPORT(getpid) __sys_getpid(void) {
  return __syscall(SYS_getpid, void);
}
long SYS_IMPORT(pause) __sys_pause(void) {
  return __syscall(SYS_pause, void);
}
long SYS_IMPORT(access) __sys_access(long path, long amode) {
  return __syscall(SYS_access, long path, long amode);
}
long SYS_IMPORT(nice) __sys_nice(long inc) {
  return __syscall(SYS_nice, long inc);
}
long SYS_IMPORT(sync) __sys_sync(void) {
  return __syscall(SYS_sync, void);
}
long SYS_IMPORT(rename) __sys_rename(long old_path, long new_path) {
  return __syscall(SYS_rename, long old_path, long new_path);
}
long SYS_IMPORT(mkdir) __sys_mkdir(long path, long mode) {
  return __syscall(SYS_mkdir, long path, long mode);
}
long SYS_IMPORT(rmdir) __sys_rmdir(long path) {
  return __syscall(SYS_rmdir, long path);
}
long SYS_IMPORT(dup) __sys_dup(long fd) {
  return __syscall(SYS_dup, long fd);
}
long SYS_IMPORT(pipe) __sys_pipe(long fd) {
  return __syscall(SYS_pipe, long fd);
}
long SYS_IMPORT(acct) __sys_acct(long filename) {
  return __syscall(SYS_acct, long filename);
}
long SYS_IMPORT(ioctl) __sys_ioctl(long fd, long request, ...) {
  return __syscall(SYS_ioctl, long fd, long request, ...);
}
long SYS_IMPORT(setpgid) __sys_setpgid(long pid, long gpid) {
  return __syscall(SYS_setpgid, long pid, long gpid);
}
long SYS_IMPORT(umask) __sys_umask(long mask) {
  return __syscall(SYS_umask, long mask);
}
long SYS_IMPORT(dup2) __sys_dup2(long oldfd, long newfd) {
  return __syscall(SYS_dup2, long oldfd, long newfd);
}
long SYS_IMPORT(getppid) __sys_getppid(void) {
  return __syscall(SYS_getppid, void);
}
long SYS_IMPORT(getpgrp) __sys_getpgrp(void) {
  return __syscall(SYS_getpgrp, void);
}
long SYS_IMPORT(setsid) __sys_setsid(void) {
  return __syscall(SYS_setsid, void);
}
long SYS_IMPORT(setrlimit) __sys_setrlimit(long resource, long limit) {
  return __syscall(SYS_setrlimit, long resource, long limit);
}
long SYS_IMPORT(getrusage) __sys_getrusage(long who, long usage) {
  return __syscall(SYS_getrusage, long who, long usage);
}
long SYS_IMPORT(symlink) __sys_symlink(long target, long linkpath) {
  return __syscall(SYS_symlink, long target, long linkpath);
}
long SYS_IMPORT(readlink) __sys_readlink(long path, long buf, long bufsize) {
  return __syscall(SYS_readlink, long path, long buf, long bufsize);
}
long SYS_IMPORT(munmap) __sys_munmap(long addr, long len) {
  return __syscall(SYS_munmap, long addr, long len);
}
long SYS_IMPORT(fchmod) __sys_fchmod(long fd, long mode) {
  return __syscall(SYS_fchmod, long fd, long mode);
}
long SYS_IMPORT(getpriority) __sys_getpriority(long which, long who) {
  return __syscall(SYS_getpriority, long which, long who);
}
long SYS_IMPORT(setpriority) __sys_setpriority(long which, long who, long prio) {
  return __syscall(SYS_setpriority, long which, long who, long prio);
}
long SYS_IMPORT(socketcall) __sys_socketcall(long call, long args) {
  return __syscall(SYS_socketcall, long call, long args);
}
long SYS_IMPORT(setitimer) __sys_setitimer(long which, long new_value, long old_value) {
  return __syscall(SYS_setitimer, long which, long new_value, long old_value);
}
long SYS_IMPORT(wait4) __sys_wait4(long pid, long wstatus, long options, long rusage) {
  return __syscall(SYS_wait4, long pid, long wstatus, long options, long rusage);
}
long SYS_IMPORT(setdomainname) __sys_setdomainname(long name, long size) {
  return __syscall(SYS_setdomainname, long name, long size);
}
long SYS_IMPORT(uname) __sys_uname(long buf) {
  return __syscall(SYS_uname, long buf);
}
long SYS_IMPORT(mprotect) __sys_mprotect(long addr, long len, long size) {
  return __syscall(SYS_mprotect, long addr, long len, long size);
}
long SYS_IMPORT(getpgid) __sys_getpgid(long pid) {
  return __syscall(SYS_getpgid, long pid);
}
long SYS_IMPORT(fchdir) __sys_fchdir(long fd) {
  return __syscall(SYS_fchdir, long fd);
}
long SYS_IMPORT(_newselect) __sys__newselect(long nfds, long readfds, long writefds, long exceptfds, long timeout) {
  return __syscall(SYS__newselect, long nfds, long readfds, long writefds, long exceptfds, long timeout);
}
long SYS_IMPORT(msync) __sys_msync(long addr, long len, long flags) {
  return __syscall(SYS_msync, long addr, long len, long flags);
}
long SYS_IMPORT(getsid) __sys_getsid(long pid) {
  return __syscall(SYS_getsid, long pid);
}
long SYS_IMPORT(fdatasync) __sys_fdatasync(long fd) {
  return __syscall(SYS_fdatasync, long fd);
}
long SYS_IMPORT(mlock) __sys_mlock(long addr, long len) {
  return __syscall(SYS_mlock, long addr, long len);
}
long SYS_IMPORT(munlock) __sys_munlock(long addr, long len) {
  return __syscall(SYS_munlock, long addr, long len);
}
long SYS_IMPORT(mlockall) __sys_mlockall(long flags) {
  return __syscall(SYS_mlockall, long flags);
}
long SYS_IMPORT(munlockall) __sys_munlockall(void) {
  return __syscall(SYS_munlockall, void);
}
long SYS_IMPORT(mremap) __sys_mremap(long old_addr, long old_size, long new_size, long flags, long new_addr) {
  return __syscall(SYS_mremap, long old_addr, long old_size, long new_size, long flags, long new_addr);
}
long SYS_IMPORT(poll) __sys_poll(long fds, long nfds, long timeout) {
  return __syscall(SYS_poll, long fds, long nfds, long timeout);
}
long SYS_IMPORT(rt_sigqueueinfo) __sys_rt_sigqueueinfo(long tgid, long sig, long uinfo) {
  return __syscall(SYS_rt_sigqueueinfo, long tgid, long sig, long uinfo);
}
long SYS_IMPORT(getcwd) __sys_getcwd(long buf, long size) {
  return __syscall(SYS_getcwd, long buf, long size);
}
long SYS_IMPORT(ugetrlimit) __sys_ugetrlimit(long resource, long rlim) {
  return __syscall(SYS_ugetrlimit, long resource, long rlim);
}
long SYS_IMPORT(mmap2) __sys_mmap2(long addr, long len, long prot, long flags, long fd, long off) {
  return __syscall(SYS_mmap2, long addr, long len, long prot, long flags, long fd, long off);
}
long SYS_IMPORT(truncate64) __sys_truncate64(long path, long zero, long low, long high) {
  return __syscall(SYS_truncate64, long path, long zero, long low, long high);
}
long SYS_IMPORT(ftruncate64) __sys_ftruncate64(long fd, long zero, long low, long high) {
  return __syscall(SYS_ftruncate64, long fd, long zero, long low, long high);
}
long SYS_IMPORT(stat64) __sys_stat64(long path, long buf) {
  return __syscall(SYS_stat64, long path, long buf);
}
long SYS_IMPORT(lstat64) __sys_lstat64(long path, long buf) {
  return __syscall(SYS_lstat64, long path, long buf);
}
long SYS_IMPORT(fstat64) __sys_fstat64(long fd, long buf) {
  return __syscall(SYS_fstat64, long fd, long buf);
}
long SYS_IMPORT(lchown32) __sys_lchown32(long path, long owner, long group) {
  return __syscall(SYS_lchown32, long path, long owner, long group);
}
long SYS_IMPORT(getuid32) __sys_getuid32(void) {
  return __syscall(SYS_getuid32, void);
}
long SYS_IMPORT(getgid32) __sys_getgid32(void) {
  return __syscall(SYS_getgid32, void);
}
long SYS_IMPORT(geteuid32) __sys_geteuid32(void) {
  return __syscall(SYS_geteuid32, void);
}
long SYS_IMPORT(getegid32) __sys_getegid32(void) {
  return __syscall(SYS_getegid32, void);
}
long SYS_IMPORT(setreuid32) __sys_setreuid32(long ruid, long euid) {
  return __syscall(SYS_setreuid32, long ruid, long euid);
}
long SYS_IMPORT(setregid32) __sys_setregid32(long rgid, long egid) {
  return __syscall(SYS_setregid32, long rgid, long egid);
}
long SYS_IMPORT(getgroups32) __sys_getgroups32(long size, long list) {
  return __syscall(SYS_getgroups32, long size, long list);
}
long SYS_IMPORT(fchown32) __sys_fchown32(long fd, long owner, long group) {
  return __syscall(SYS_fchown32, long fd, long owner, long group);
}
long SYS_IMPORT(setresuid32) __sys_setresuid32(long ruid, long euid, long suid) {
  return __syscall(SYS_setresuid32, long ruid, long euid, long suid);
}
long SYS_IMPORT(getresuid32) __sys_getresuid32(long ruid, long euid, long suid) {
  return __syscall(SYS_getresuid32, long ruid, long euid, long suid);
}
long SYS_IMPORT(setresgid32) __sys_setresgid32(long rgid, long egid, long sgid) {
  return __syscall(SYS_setresgid32, long rgid, long egid, long sgid);
}
long SYS_IMPORT(getresgid32) __sys_getresgid32(long rgid, long egid, long sgid) {
  return __syscall(SYS_getresgid32, long rgid, long egid, long sgid);
}
long SYS_IMPORT(chown32) __sys_chown32(long path, long owner, long group) {
  return __syscall(SYS_chown32, long path, long owner, long group);
}
long SYS_IMPORT(setuid32) __sys_setuid32(long uid) {
  return __syscall(SYS_setuid32, long uid);
}
long SYS_IMPORT(setgid32) __sys_setgid32(long uid) {
  return __syscall(SYS_setgid32, long uid);
}
long SYS_IMPORT(mincore) __sys_mincore(long addr, long length, long vec) {
  return __syscall(SYS_mincore, long addr, long length, long vec);
}
long SYS_IMPORT(madvise1) __sys_madvise1(long addr, long length, long advice) {
  return __syscall(SYS_madvise1, long addr, long length, long advice);
}
long SYS_IMPORT(getdents64) __sys_getdents64(long fd, long dirp, long count) {
  return __syscall(SYS_getdents64, long fd, long dirp, long count);
}
long SYS_IMPORT(fcntl64) __sys_fcntl64(long fd, long cmd, ...) {
  return __syscall(SYS_fcntl64, long fd, long cmd, ...);
}
long SYS_IMPORT(exit_group) __sys_exit_group(long status) {
  return __syscall(SYS_exit_group, long status);
}
long SYS_IMPORT(statfs64) __sys_statfs64(long path, long size, long buf) {
  return __syscall(SYS_statfs64, long path, long size, long buf);
}
long SYS_IMPORT(fstatfs64) __sys_fstatfs64(long fd, long size, long buf) {
  return __syscall(SYS_fstatfs64, long fd, long size, long buf);
}
long SYS_IMPORT(fadvise64_64) __sys_fadvise64_64(long fd, long zero, long low, long high, long low2, long high2, long advice) {
  return __syscall(SYS_fadvise64_64, long fd, long zero, long low, long high, long low2, long high2, long advice);
}
long SYS_IMPORT(openat) __sys_openat(long dirfd, long path, long flags, ...) {
  return __syscall(SYS_openat, long dirfd, long path, long flags, ...);
}
long SYS_IMPORT(mkdirat) __sys_mkdirat(long dirfd, long path, long mode) {
  return __syscall(SYS_mkdirat, long dirfd, long path, long mode);
}
long SYS_IMPORT(mknodat) __sys_mknodat(long dirfd, long path, long mode, long dev) {
  return __syscall(SYS_mknodat, long dirfd, long path, long mode, long dev);
}
long SYS_IMPORT(fchownat) __sys_fchownat(long dirfd, long path, long owner, long group, long flags) {
  return __syscall(SYS_fchownat, long dirfd, long path, long owner, long group, long flags);
}
long SYS_IMPORT(fstatat64) __sys_fstatat64(long dirfd, long path, long buf, long flags) {
  return __syscall(SYS_fstatat64, long dirfd, long path, long buf, long flags);
}
long SYS_IMPORT(unlinkat) __sys_unlinkat(long dirfd, long path, long flags) {
  return __syscall(SYS_unlinkat, long dirfd, long path, long flags);
}
long SYS_IMPORT(renameat) __sys_renameat(long olddirfd, long oldpath, long newdirfd, long newpath) {
  return __syscall(SYS_renameat, long olddirfd, long oldpath, long newdirfd, long newpath);
}
long SYS_IMPORT(linkat) __sys_linkat(long olddirfd, long oldpath, long newdirfd, long newpath, long flags) {
  return __syscall(SYS_linkat, long olddirfd, long oldpath, long newdirfd, long newpath, long flags);
}
long SYS_IMPORT(symlinkat) __sys_symlinkat(long target, long newdirfd, long linkpath) {
  return __syscall(SYS_symlinkat, long target, long newdirfd, long linkpath);
}
long SYS_IMPORT(readlinkat) __sys_readlinkat(long dirfd, long path, long bug, long bufsize) {
  return __syscall(SYS_readlinkat, long dirfd, long path, long bug, long bufsize);
}
long SYS_IMPORT(fchmodat) __sys_fchmodat(long dirfd, long path, long mode, ...) {
  return __syscall(SYS_fchmodat, long dirfd, long path, long mode, ...);
}
long SYS_IMPORT(faccessat) __sys_faccessat(long dirfd, long path, long amode, long flags) {
  return __syscall(SYS_faccessat, long dirfd, long path, long amode, long flags);
}
long SYS_IMPORT(pselect6) __sys_pselect6(long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks) {
  return __syscall(SYS_pselect6, long nfds, long readfds, long writefds, long exceptfds, long timeout, long sigmaks);
}
long SYS_IMPORT(utimensat) __sys_utimensat(long dirfd, long path, long times, long flags) {
  return __syscall(SYS_utimensat, long dirfd, long path, long times, long flags);
}
long SYS_IMPORT(fallocate) __sys_fallocate(long fd, long mode, long off_low, long off_high, long len_low, long len_high) {
  return __syscall(SYS_fallocate, long fd, long mode, long off_low, long off_high, long len_low, long len_high);
}
long SYS_IMPORT(dup3) __sys_dup3(long fd, long suggestfd, long flags) {
  return __syscall(SYS_dup3, long fd, long suggestfd, long flags);
}
long SYS_IMPORT(pipe2) __sys_pipe2(long fds, long flags) {
  return __syscall(SYS_pipe2, long fds, long flags);
}
long SYS_IMPORT(recvmmsg) __sys_recvmmsg(long sockfd, long msgvec, long vlen, long flags, ...) {
  return __syscall(SYS_recvmmsg, long sockfd, long msgvec, long vlen, long flags, ...);
}
long SYS_IMPORT(prlimit64) __sys_prlimit64(long pid, long resource, long new_limit, long old_limit) {
  return __syscall(SYS_prlimit64, long pid, long resource, long new_limit, long old_limit);
}
long SYS_IMPORT(sendmmsg) __sys_sendmmsg(long sockfd, long msgvec, long vlen, long flags, ...) {
  return __syscall(SYS_sendmmsg, long sockfd, long msgvec, long vlen, long flags, ...);
}
long SYS_IMPORT(socket) __sys_socket(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_socket, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(socketpair) __sys_socketpair(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_socketpair, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(bind) __sys_bind(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_bind, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(connect) __sys_connect(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_connect, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(listen) __sys_listen(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_listen, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(accept4) __sys_accept4(long sockfd, long addr, long addrlen, long flags, long dummy1, long dummy2) {
  return __syscall(SYS_accept4, long sockfd, long addr, long addrlen, long flags, long dummy1, long dummy2);
}
long SYS_IMPORT(getsockopt) __sys_getsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_getsockopt, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(setsockopt) __sys_setsockopt(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_setsockopt, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(getsockname) __sys_getsockname(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_getsockname, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(getpeername) __sys_getpeername(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_getpeername, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(sendto) __sys_sendto(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_sendto, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(sendmsg) __sys_sendmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_sendmsg, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(recvfrom) __sys_recvfrom(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_recvfrom, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(recvmsg) __sys_recvmsg(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_recvmsg, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}
long SYS_IMPORT(shutdown) __sys_shutdown(long sockfd, long level, long optname, long optval, long optlen, long dummy) {
  return __syscall(SYS_shutdown, long sockfd, long level, long optname, long optval, long optlen, long dummy);
}


#ifdef __cplusplus
}
#endif
