//
// Created by kinit on 2021-05-14.
//

#ifndef DISP_TUNER_DAEMON_H
#define DISP_TUNER_DAEMON_H

#ifdef __cplusplus
extern "C" {
#endif

void startDaemon(int uid, const char *ipcFilePath, int daemonize);

#ifdef __cplusplus
};
#endif

#endif //DISP_TUNER_DAEMON_H
