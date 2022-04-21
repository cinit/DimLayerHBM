//
// Created by kinit on 2021-10-17.
//

#ifndef DISP_TUNER_NATIVES_INJECT_IO_INIT_H
#define DISP_TUNER_NATIVES_INJECT_IO_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

int BaseHalPatchInitSocket(int fd);

void *initElfHeaderInfo(const char *soname, void *initFunction);

int get_tracer_pid();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

int getDaemonIpcSocket();

void closeDaemonIpcSocket();

#endif

#endif //DISP_TUNER_NATIVES_INJECT_IO_INIT_H
