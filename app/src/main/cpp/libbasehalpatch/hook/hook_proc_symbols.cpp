//
// Created by kinit on 2021-10-22.
//

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <dlfcn.h>
#include <sys/mman.h>

#include "hook_proc_symbols.h"
#include "hook_trampoline.h"
#include "../ipc/ipc_io_event.h"

#define EXPORT __attribute__((visibility("default")))

int (*pf_orig_open_2)(const char *pathname, int flags) = nullptr;

int (*pf_orig_open)(const char *pathname, int flags, ...) = nullptr;

ssize_t (*pf_orig_read_chk)(int fd, void *buf, size_t count, size_t buf_size) = nullptr;

ssize_t (*pf_orig_read)(int fd, void *buf, size_t nbytes) = nullptr;

ssize_t (*pf_orig_write_chk)(int fd, const void *buf, size_t count, size_t buf_size) = nullptr;

ssize_t (*pf_orig_write)(int fd, const void *buf, size_t nbytes) = nullptr;

int (*pf_orig_close)(int fd) = nullptr;

int (*pf_orig_ioctl)(int fd, unsigned long int request, ...) = nullptr;

int (*pf_orig_select)(int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout) = nullptr;

int gsIsInitialized = 0;

using halpatchhook::callback::HookCallback;

std::array<HookCallback<int, const char *, int, uint32_t>, 2> sOpenHookCallback = {};
std::array<HookCallback<ssize_t, int, void *, size_t>, 2> sReadHookCallback = {};
std::array<HookCallback<ssize_t, int, const void *, size_t>, 2> sWriteHookCallback = {};
std::array<HookCallback<int, int>, 2> sCloseHookCallback = {};
std::array<HookCallback<int, int, unsigned long int, uint64_t>, 2> sIoctlHookCallback = {};
std::array<HookCallback<int, int, void *, void *, void *, void *>, 2> sSelectHookCallback = {};

template<typename R, typename ...Ts>
static constexpr void *as_void_ptr(R(*f)(Ts...)) {
    return reinterpret_cast<void *>(f);
}

template<typename R, typename ...Ts>
using func_ptr = R(*)(Ts...);
template<typename R, typename ...Ts>
using funcv_ptr = R(*)(Ts..., ...);

template<typename R, typename ...Ts>
inline static void assign(func_ptr<R, Ts...> &pf, void *p) {
    pf = reinterpret_cast<func_ptr<R, Ts...>>(p);
}

template<typename R, typename ...Ts>
inline static void assign(funcv_ptr<R, Ts...> &pf, void *p) {
    pf = reinterpret_cast<funcv_ptr<R, Ts...>>(p);
}

static void *hookPltImpl(void *base, uint32_t off, void *hookProc, void *libc, const char *sym_name) {
    if (base == nullptr || off == 0) {
        return nullptr;
    }
    size_t addr = ((size_t) base) + off;
    void **p = (void **) addr;
    void *orig_ptr;
    orig_ptr = *p;
    if (orig_ptr == hookProc) {
        // already hooked?
        return dlsym(libc, sym_name);
    }
    void *aligned_ptr = (void *) (((size_t) p) & ~(size_t) (0x1000 - 1));
    mprotect(aligned_ptr, 0x1000, PROT_WRITE | PROT_READ);
    *p = hookProc;
    mprotect(aligned_ptr, 0x1000, PROT_READ);
    __builtin___clear_cache(static_cast<char *>(aligned_ptr), static_cast<char *>((void *) ((size_t) aligned_ptr + 0x1000U)));
    return orig_ptr;
}

template<typename R, typename ...Ts>
static inline void *hookPlt(void *base, uint32_t off, func_ptr<R, Ts...> hookProc, void *libc, const char *sym_name) {
    return hookPltImpl(base, off, as_void_ptr(hookProc), libc, sym_name);
}

int hook_sym_init(const struct OriginHookProcedure *op) {
    if (gsIsInitialized != 0) {
        return 0;
    }
    if (op == nullptr) {
        return -EINVAL;
    }
    if (op->struct_size != sizeof(struct OriginHookProcedure)) {
        return -EINVAL;
    }
    void *libc = dlopen("libc.so", RTLD_NOW | RTLD_NOLOAD);
    if (libc == nullptr) {
        // should not happen, we have DT_NEEDED
        return -EFAULT;
    }
    void *base = (void *) (op->target_base);
    if (base == nullptr) {
        return -EINVAL;
    }
    if (op->off_plt_open_2 != 0) {
        assign(pf_orig_open_2, hookPlt(base, op->off_plt_open_2, &hook_proc_open_2, libc, "__open_2"));
        if (pf_orig_open_2 == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_open != 0) {
        assign(pf_orig_open, hookPlt(base, op->off_plt_open, &hook_proc_open, libc, "open"));
        if (pf_orig_open == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_read_chk != 0) {
        assign(pf_orig_read_chk, hookPlt(base, op->off_plt_read_chk, &hook_proc_read_chk, libc, "__read_chk"));
        if (pf_orig_read_chk == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_read != 0) {
        assign(pf_orig_read, hookPlt(base, op->off_plt_read, &hook_proc_read, libc, "read"));
        if (pf_orig_read == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_write_chk != 0) {
        assign(pf_orig_write_chk, hookPlt(base, op->off_plt_write_chk, &hook_proc_write_chk, libc, "__write_chk"));
        if (pf_orig_write_chk == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_write != 0) {
        assign(pf_orig_write, hookPlt(base, op->off_plt_write, &hook_proc_write, libc, "write"));
        if (pf_orig_write == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_close != 0) {
        assign(pf_orig_close, hookPlt(base, op->off_plt_close, &hook_proc_close, libc, "close"));
        if (pf_orig_close == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_ioctl != 0) {
        assign(pf_orig_ioctl, hookPlt(base, op->off_plt_ioctl, &hook_proc_ioctl, libc, "ioctl"));
        if (pf_orig_ioctl == nullptr) {
            return -EFAULT;
        }
    }
    if (op->off_plt_select != 0) {
        assign(pf_orig_select, hookPlt(base, op->off_plt_select, &hook_proc_select, libc, "select"));
        if (pf_orig_select == nullptr) {
            return -EFAULT;
        }
    }
    gsIsInitialized = 1;
    return 0;
}

EXPORT int hook_proc_open_2(const char *pathname, int flags) {
    if (pf_orig_open_2 == nullptr) {
        abort();
    }
    auto before = sOpenHookCallback[0];
    auto after = sOpenHookCallback[1];
    if (before == nullptr && after == nullptr) {
        int result = pf_orig_open_2(pathname, flags);
        int err = errno;
        invokeOpenResultCallback(result < 0 ? -err : result, pathname, flags, 0);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<int, const char *, int, uint32_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(pathname, flags, 0);
        if (before != nullptr) {
            before(params);
        }
        int result;
        if (!params.isReturnEarly()) {
            result = pf_orig_open_2(params.getArgv<0>(), params.getArgv<1>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeOpenResultCallback(result < 0 ? -err : result, pathname, flags, 0);
        errno = err;
        return result;
    }
}

EXPORT int hook_proc_open(const char *pathname, int flags, uint32_t mode) {
    if (pf_orig_open == nullptr) {
        abort();
    }
    auto before = sOpenHookCallback[0];
    auto after = sOpenHookCallback[1];
    if (before == nullptr && after == nullptr) {
        int result = pf_orig_open(pathname, flags, mode);
        int err = errno;
        invokeOpenResultCallback(result < 0 ? -err : result, pathname, flags, 0);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<int, const char *, int, uint32_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(pathname, flags, 0);
        if (before != nullptr) {
            before(params);
        }
        int result;
        if (!params.isReturnEarly()) {
            result = pf_orig_open(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeOpenResultCallback(result < 0 ? -err : result, pathname, flags, 0);
        errno = err;
        return result;
    }
}

EXPORT ssize_t hook_proc_read_chk(int fd, void *buf, size_t count, size_t buf_size) {
    if (pf_orig_read_chk == nullptr) {
        abort();
    }
    auto before = sReadHookCallback[0];
    auto after = sReadHookCallback[1];
    if (before == nullptr && after == nullptr) {
        ssize_t result = pf_orig_read_chk(fd, buf, count, buf_size);
        int err = errno;
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, count);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<ssize_t, int, void *, size_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd, buf, count);
        if (before != nullptr) {
            before(params);
        }
        ssize_t result;
        if (!params.isReturnEarly()) {
            result = pf_orig_read_chk(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>(), buf_size);
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, count);
        errno = err;
        return result;
    }
}

EXPORT ssize_t hook_proc_read(int fd, void *buf, size_t nbytes) {
    if (pf_orig_read_chk == nullptr) {
        abort();
    }
    auto before = sReadHookCallback[0];
    auto after = sReadHookCallback[1];
    if (before == nullptr && after == nullptr) {
        ssize_t result = pf_orig_read(fd, buf, nbytes);
        int err = errno;
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, nbytes);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<ssize_t, int, void *, size_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd, buf, nbytes);
        if (before != nullptr) {
            before(params);
        }
        ssize_t result;
        if (!params.isReturnEarly()) {
            result = pf_orig_read(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, nbytes);
        errno = err;
        return result;
    }
}

EXPORT ssize_t hook_proc_write_chk(int fd, const void *buf, size_t count, size_t buf_size) {
    if (pf_orig_write_chk == nullptr) {
        abort();
    }
    auto before = sWriteHookCallback[0];
    auto after = sWriteHookCallback[1];
    if (before == nullptr && after == nullptr) {
        ssize_t result = pf_orig_write_chk(fd, buf, count, buf_size);
        int err = errno;
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, count);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<ssize_t, int, const void *, size_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd, buf, count);
        if (before != nullptr) {
            before(params);
        }
        ssize_t result;
        if (!params.isReturnEarly()) {
            result = pf_orig_write_chk(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>(), buf_size);
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeWriteResultCallback(result < 0 ? -err : result, fd, buf, count);
        errno = err;
        return result;
    }
}

EXPORT ssize_t hook_proc_write(int fd, const void *buf, size_t nbytes) {
    if (pf_orig_write == nullptr) {
        abort();
    }
    auto before = sWriteHookCallback[0];
    auto after = sWriteHookCallback[1];
    if (before == nullptr && after == nullptr) {
        ssize_t result = pf_orig_write_chk(fd, buf, nbytes, nbytes);
        int err = errno;
        invokeReadResultCallback(result < 0 ? -err : result, fd, buf, nbytes);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<ssize_t, int, const void *, size_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd, buf, nbytes);
        if (before != nullptr) {
            before(params);
        }
        ssize_t result;
        if (!params.isReturnEarly()) {
            result = pf_orig_write(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeWriteResultCallback(result < 0 ? -err : result, fd, buf, nbytes);
        errno = err;
        return result;
    }
}

EXPORT int hook_proc_close(int fd) {
    if (pf_orig_close == nullptr) {
        abort();
    }
    auto before = sCloseHookCallback[0];
    auto after = sCloseHookCallback[1];
    if (before == nullptr && after == nullptr) {
        int result = pf_orig_close(fd);
        int err = errno;
        invokeCloseResultCallback(result < 0 ? -err : result, fd);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<int, int> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd);
        if (before != nullptr) {
            before(params);
        }
        int result;
        if (!params.isReturnEarly()) {
            result = pf_orig_close(params.getArgv<0>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeCloseResultCallback(result < 0 ? -err : result, fd);
        errno = err;
        return result;
    }
}

EXPORT int hook_proc_ioctl(int fd, unsigned long int request, unsigned long arg) {
    if (pf_orig_ioctl == nullptr) {
        abort();
    }
    auto before = sIoctlHookCallback[0];
    auto after = sIoctlHookCallback[1];
    if (before == nullptr && after == nullptr) {
        int result = pf_orig_ioctl(fd, request, arg);
        int err = errno;
        invokeIoctlResultCallback(result < 0 ? -err : result, fd, request, arg);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<int, int, unsigned long int, uint64_t> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(fd, request, arg);
        if (before != nullptr) {
            before(params);
        }
        int result;
        if (!params.isReturnEarly()) {
            result = pf_orig_ioctl(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeIoctlResultCallback(result < 0 ? -err : result, fd, request, arg);
        errno = err;
        return result;
    }
}

EXPORT int hook_proc_select(int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout) {
    if (pf_orig_select == nullptr) {
        abort();
    }
    auto before = sSelectHookCallback[0];
    auto after = sSelectHookCallback[1];
    if (before == nullptr && after == nullptr) {
        int result = pf_orig_select(nfds, readfds, writefds, exceptfds, timeout);
        int err = errno;
        invokeSelectResultCallback(result < 0 ? -err : result, nfds, readfds, writefds, exceptfds, timeout);
        errno = err;
        return result;
    } else {
        int err = errno;
        HookParams<int, int, void *, void *, void *, void *> params;
        params.setCallerAddress(__builtin_return_address(0));
        params.setErrno(err);
        params.updateArguments(nfds, readfds, writefds, exceptfds, timeout);
        if (before != nullptr) {
            before(params);
        }
        int result;
        if (!params.isReturnEarly()) {
            result = pf_orig_select(params.getArgv<0>(), params.getArgv<1>(), params.getArgv<2>(), params.getArgv<3>(), params.getArgv<4>());
            err = errno;
            if (after != nullptr) {
                params.setResult(result);
                params.setErrno(err);
                after(params);
            }
            result = params.getResult();
            err = params.getErrno();
        } else {
            result = params.getResult();
            err = params.getErrno();
        }
        invokeSelectResultCallback(result < 0 ? -err : result, nfds, readfds, writefds, exceptfds, timeout);
        errno = err;
        return result;
    }
}

namespace halpatchhook::callback {

void setReadCallback(HookCallback<ssize_t, int, void *, size_t> before,
                     HookCallback<ssize_t, int, void *, size_t> after) {
    sReadHookCallback[0] = before;
    sReadHookCallback[1] = after;
}

void setWriteCallback(HookCallback<ssize_t, int, const void *, size_t> before,
                      HookCallback<ssize_t, int, const void *, size_t> after) {
    sWriteHookCallback[0] = before;
    sWriteHookCallback[1] = after;
}

void setOpenCallback(HookCallback<int, const char *, int, uint32_t> before,
                     HookCallback<int, const char *, int, uint32_t> after) {
    sOpenHookCallback[0] = before;
    sOpenHookCallback[1] = after;
}

void setCloseCallback(HookCallback<int, int> before, HookCallback<int, int> after) {
    sCloseHookCallback[0] = before;
    sCloseHookCallback[1] = after;
}

void setIoctlCallback(HookCallback<int, int, unsigned long int, uint64_t> before,
                      HookCallback<int, int, unsigned long int, uint64_t> after) {
    sIoctlHookCallback[0] = before;
    sIoctlHookCallback[1] = after;
}

void setSelectCallback(HookCallback<int, int, void *, void *, void *, void *> before,
                       HookCallback<int, int, void *, void *, void *, void *> after) {
    sSelectHookCallback[0] = before;
    sSelectHookCallback[1] = after;
}

}
