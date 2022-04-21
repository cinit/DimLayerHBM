//
// Created by kinit on 2021-06-09.
//

#ifndef DISP_TUNER_INCIHOSTDAEMON_H
#define DISP_TUNER_INCIHOSTDAEMON_H

#include <string>
#include <vector>

#include "../libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "log/LogEntryRecord.h"
#include "protocol/ArgList.h"
#include "protocol/LpcResult.h"

namespace ipcprotocol {

class IDispTunerDaemon {
public:
    static constexpr uint32_t PROXY_ID =
            uint32_t('D') | (uint32_t('T') << 8) | (uint32_t('D') << 16) | (uint32_t('0') << 24);

    class HistoryIoOperationEventList {
    public:
        uint32_t totalStartIndex = 0;
        uint32_t totalCount = 0;
        std::vector<halpatch::IoSyscallEvent> events; // contains index and length
        std::vector<std::vector<uint8_t>> payloads;

        [[nodiscard]] bool deserializeFromByteVector(const std::vector<uint8_t> &src);

        [[nodiscard]] std::vector<uint8_t> serializeToByteVector() const;
    };

    class DaemonStatus {
    public:
        class HalServiceStatus {
        public:
            bool isHalServiceAttached;
            int halServicePid;
            int halServiceUid;
            std::string halServiceExePath;
            int halServiceArch;
            std::string halServiceProcessSecurityContext;
            std::string halServiceExecutableSecurityLabel;
        };

        // daemon process info
        int processId;
        std::string versionName;
        int abiArch;
        std::string daemonProcessSecurityContext;
        // HAL service info
        HalServiceStatus opDispFeatureHidlServiceStatus;

        [[nodiscard]] bool deserializeFromByteVector(const std::vector<uint8_t> &src);

        [[nodiscard]] std::vector<uint8_t> serializeToByteVector() const;
    };

    IDispTunerDaemon() = default;

    virtual ~IDispTunerDaemon() = default;

    [[nodiscard]]
    virtual TypedLpcResult<std::string> getVersionName() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<int> getVersionCode() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<std::string> getBuildUuid() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<int> getSelfPid() = 0;

    virtual TypedLpcResult<void> exitProcess() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> isDeviceSupported() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> isHwServiceConnected() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> attachHwHidlService(const std::vector<std::string> &soPath) = 0;

    [[nodiscard]]
    virtual TypedLpcResult<HistoryIoOperationEventList> getHistoryIoOperations(uint32_t start, uint32_t length) = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> clearHistoryIoEvents() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<DaemonStatus> getDaemonStatus() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<int> deviceDriverIoctl0(uint32_t request, uint64_t arg) = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> isOpDimLayerHbmEnabled() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<void> setOpDimLayerHbmEnabled(bool enabled) = 0;

    [[nodiscard]]
    virtual TypedLpcResult<bool> isOpDimLayerHbmForceEnabled() = 0;

    [[nodiscard]]
    virtual TypedLpcResult<void> setOpDimLayerHbmForceEnabled(bool enabled) = 0;

    [[nodiscard]]
    virtual TypedLpcResult<std::vector<LogEntryRecord>> getLogsPartial(uint32_t startIndex, uint32_t count) = 0;

    class TransactionIds {
    public:
        static constexpr uint32_t getVersionName = 1;
        static constexpr uint32_t getVersionCode = 2;
        static constexpr uint32_t getBuildUuid = 3;
        static constexpr uint32_t exitProcess = 4;
        static constexpr uint32_t isDeviceSupported = 10;
        static constexpr uint32_t isHwServiceConnected = 11;
        static constexpr uint32_t attachHwHidlService = 12;
        static constexpr uint32_t getSelfPid = 13;
        static constexpr uint32_t getHistoryIoOperations = 14;
        static constexpr uint32_t clearHistoryIoEvents = 15;
        static constexpr uint32_t getDaemonStatus = 16;
        static constexpr uint32_t deviceDriverIoctl0 = 18;

        static constexpr uint32_t isOpDimLayerHbmEnabled = 19;
        static constexpr uint32_t setOpDimLayerHbmEnabled = 20;
        static constexpr uint32_t isOpDimLayerHbmForceEnabled = 21;
        static constexpr uint32_t setOpDimLayerHbmForceEnabled = 22;

        static constexpr uint32_t getLogsPartial = 23;
    };
};

}

#endif //DISP_TUNER_INCIHOSTDAEMON_H
