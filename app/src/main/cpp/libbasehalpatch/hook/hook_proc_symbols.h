//
// Created by kinit on 2021-10-22.
//

#ifndef DISP_TUNER_NATIVES_HOOK_PROC_SYMBOLS_H
#define DISP_TUNER_NATIVES_HOOK_PROC_SYMBOLS_H

#include <cstdint>
#include <sys/types.h>

#include "hook_sym_struct.h"
#include "hook_trampoline.h"

namespace halpatchhook::callback {

template<typename R, typename... Ts>
using HookCallback = void (*)(HookParams<R, Ts...> &params);

void setReadCallback(HookCallback<ssize_t, int, void *, size_t> before, HookCallback<ssize_t, int, void *, size_t> after);

void setWriteCallback(HookCallback<ssize_t, int, const void *, size_t> before, HookCallback<ssize_t, int, const void *, size_t> after);

void setOpenCallback(HookCallback<int, const char *, int, uint32_t> before, HookCallback<int, const char *, int, uint32_t> after);

void setCloseCallback(HookCallback<int, int> before, HookCallback<int, int> after);

void setIoctlCallback(HookCallback<int, int, unsigned long int, uint64_t> before, HookCallback<int, int, unsigned long int, uint64_t> after);

void setSelectCallback(HookCallback<int, int, void *, void *, void *, void *> before, HookCallback<int, int, void *, void *, void *, void *> after);

}

#endif //DISP_TUNER_NATIVES_HOOK_PROC_SYMBOLS_H
