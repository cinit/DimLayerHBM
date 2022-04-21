//
// Created by kinit on 2021-11-17.
//

#ifndef DISP_TUNER_NATIVES_BASEHWHALHANDLER_H
#define DISP_TUNER_NATIVES_BASEHWHALHANDLER_H

#include <atomic>
#include <string>
#include <vector>
#include <pthread.h>
#include <tuple>
#include <mutex>
#include <condition_variable>

#include "libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "rpcprotocol/utils/HashMap.h"
#include "../IBaseService.h"

namespace hwhal {

class BaseHwHalHandler : public IBaseService {
public:
    struct StartupInfo {
        int fd;
        int pid;
    };

    BaseHwHalHandler() = default;

    ~BaseHwHalHandler() override = default;

    [[nodiscard]] int onStart(void *args, const std::shared_ptr<IBaseService> &sp) override;

    [[nodiscard]] bool onStop() override;

    [[nodiscard]] virtual int doOnStart(void *args, const std::shared_ptr<IBaseService> &sp) = 0;

    [[nodiscard]] virtual bool doOnStop() = 0;

    [[nodiscard]] virtual bool isConnected() const;

    /**
     * Send a message to the HAL service and wait for a response.
     * @param request the request to send
     * @param payload optional payload argument, may be nullptr if request does not have a payload
     * @param timeout_ms maximum time to wait for a response, 0 means no timeout(wait forever)
     * @return tuple of error code, response, payload. If error code is 0, response and payload are valid.
     */
    [[nodiscard]]
    std::tuple<int, halpatch::HalResponse, std::vector<uint8_t>> sendHalRequest(const halpatch::HalRequest &request,
                                                                                const void *payload, int timeout_ms);

    [[nodiscard]] virtual int driverRawWrite(const std::vector<uint8_t> &buffer) = 0;

    [[nodiscard]] virtual int driverRawIoctl0(uint32_t request, uint64_t arg) = 0;

    void stopSelf();

private:
    class LpcReturnStatus {
    public:
        std::condition_variable *cond;
        std::vector<uint8_t> *buffer;
        uint32_t error;
    };

    std::atomic<bool> mIsRunning = false;
    std::atomic<uint32_t> mSequenceId = 0x10000000;
    std::mutex mIpcMutex;
    int mRemotePid = -1;
    pthread_t mThread = 0;
    int mFd = -1;
    HashMap<uint32_t, LpcReturnStatus *> mWaitingRequests;

    static void workerThread(BaseHwHalHandler *self);

    void dispatchHwHalPatchIpcPacket(const void *packet, size_t length);

protected:
    virtual void dispatchHwHalIoEvent(const halpatch::IoSyscallEvent &event, const void *payload) = 0;

    virtual void dispatchRemoteProcessDeathEvent() = 0;

public:
    [[nodiscard]] inline int getFd() const {
        return mFd;
    }

    [[nodiscard]] inline int getRemotePid() const {
        return mRemotePid;
    }

};

}

#endif //DISP_TUNER_NATIVES_BASEHWHALHANDLER_H
