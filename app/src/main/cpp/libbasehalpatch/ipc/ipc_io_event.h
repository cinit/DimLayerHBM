//
// Created by kinit on 2021-10-21.
//

#ifndef DISP_TUNER_NATIVES_IPC_IO_EVENT_H
#define DISP_TUNER_NATIVES_IPC_IO_EVENT_H

#include <cstdint>

void invokeReadResultCallback(ssize_t result, int fd, const void *buffer, size_t size);

void invokeWriteResultCallback(ssize_t result, int fd, const void *buffer, size_t size);

void invokeOpenResultCallback(int result, const char *name, int flags, uint32_t mode);

void invokeCloseResultCallback(int result, int fd);

void invokeIoctlResultCallback(int result, int fd, unsigned long int request, uint64_t arg);

void invokeSelectResultCallback(int result, int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout);

#endif //DISP_TUNER_NATIVES_IPC_IO_EVENT_H
