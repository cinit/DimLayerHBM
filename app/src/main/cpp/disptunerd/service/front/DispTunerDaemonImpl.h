//
// Created by kinit on 2021-10-13.
//

#ifndef COM_DAEMON_COM_DAEMONIMPL_H
#define COM_DAEMON_COM_DAEMONIMPL_H

#include <vector>
#include <tuple>
#include <deque>
#include <atomic>

#include "libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "rpcprotocol/protocol/IpcTransactor.h"
#include "rpcprotocol/protocol/BaseIpcObject.h"
#include "rpcprotocol/IDispTunerDaemon.h"

namespace ipcprotocol {

class DispTunerDaemonImpl : public IDispTunerDaemon, public BaseIpcObject {
public:
    DispTunerDaemonImpl() = default;

    ~DispTunerDaemonImpl() override = default;

    DispTunerDaemonImpl(const DispTunerDaemonImpl &) = delete;

    DispTunerDaemonImpl &operator=(const DispTunerDaemonImpl &) = delete;

    [[nodiscard]] uint32_t getProxyId() const override;

    bool dispatchLpcInvocation(const IpcTransactor::LpcEnv &env, LpcResult &result,
                               uint32_t funcId, const ArgList &args) override;

    bool dispatchEvent(const IpcTransactor::LpcEnv &env, uint32_t eventId, const ArgList &args) override;

    void handleIoSyscallEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload);

    TypedLpcResult<std::string> getVersionName() override;

    TypedLpcResult<int> getVersionCode() override;

    TypedLpcResult<std::string> getBuildUuid() override;

    TypedLpcResult<int> getSelfPid() override;

    TypedLpcResult<void> exitProcess() override;

    TypedLpcResult<bool> isDeviceSupported() override;

    TypedLpcResult<bool> isHwServiceConnected() override;

    TypedLpcResult<bool> attachHwHidlService(const std::vector<std::string> &soPath) override;

    TypedLpcResult<HistoryIoOperationEventList> getHistoryIoOperations(uint32_t start, uint32_t length) override;

    TypedLpcResult<bool> clearHistoryIoEvents() override;

    TypedLpcResult<DaemonStatus> getDaemonStatus() override;

    TypedLpcResult<int> deviceDriverIoctl0(uint32_t request, uint64_t arg) override;

    TypedLpcResult<bool> isOpDimLayerHbmEnabled() override;

    TypedLpcResult<void> setOpDimLayerHbmEnabled(bool enabled) override;

    TypedLpcResult<bool> isOpDimLayerHbmForceEnabled() override;

    TypedLpcResult<void> setOpDimLayerHbmForceEnabled(bool enabled) override;

    TypedLpcResult<std::vector<LogEntryRecord>> getLogsPartial(uint32_t startIndex, uint32_t count) override;

private:
    std::mutex mEventMutex;
    std::deque<std::tuple<halpatch::IoSyscallEvent, std::vector<uint8_t>>> mHistoryIoEvents;
    std::atomic_uint32_t mNextIoEventId = 0;

public:
    inline uint32_t nextIoSyscallEventSequence() {
        return mNextIoEventId++;
    }

};

}

#endif //COM_DAEMON_COM_DAEMONIMPL_H
