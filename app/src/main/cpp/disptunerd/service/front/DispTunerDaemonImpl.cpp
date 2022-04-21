//
// Created by kinit on 2021-10-13.
//

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cerrno>
#include <string_view>

#include "disptunerd/inject/SysServicePatch.h"
#include "disptunerd/ipc/IpcStateController.h"

#include "rpcprotocol/utils/TextUtils.h"
#include "rpcprotocol/utils/auto_close_fd.h"
#include "rpcprotocol/utils/SELinux.h"
#include "rpcprotocol/log/Log.h"
#include "rpcprotocol/protocol/LpcArgListExtractor.h"
#include "rpcprotocol/utils/ProcessUtils.h"
#include "disptunerd/ipc/logbuffer/LocalLogBuffer.h"

#include "DispTunerDaemonImpl.h"
#include "disptunerd/service/HwServiceStatus.h"
#include "../hw/opdispfeature/OpDispFeatureHandler.h"

static constexpr auto LOG_TAG = "DispTunerDaemonImpl";

using namespace ipcprotocol;
using namespace halpatch;
using namespace hwhal;

static constexpr uint32_t MAX_HISTORY_EVENT_SIZE = 3000;

uint32_t DispTunerDaemonImpl::getProxyId() const {
    return PROXY_ID;
}

TypedLpcResult<std::string> DispTunerDaemonImpl::getVersionName() {
    return {DISPLAY_TUNER_VERSION};
}

TypedLpcResult<int> DispTunerDaemonImpl::getVersionCode() {
    return {1};
}

TypedLpcResult<std::string> DispTunerDaemonImpl::getBuildUuid() {
    return {"NOT_AVAILABLE"};
}

TypedLpcResult<int> DispTunerDaemonImpl::getSelfPid() {
    return {int(getpid())};
}

TypedLpcResult<void> DispTunerDaemonImpl::exitProcess() {
    LOGI("client requested exit process, exit now...");
    exit(0);
}

bool DispTunerDaemonImpl::dispatchLpcInvocation([[maybe_unused]] const IpcTransactor::LpcEnv &env,
                                                LpcResult &result, uint32_t funcId, const ArgList &args) {
    using T = DispTunerDaemonImpl;
    using R = LpcArgListExtractor<T>;
    using Ids = DispTunerDaemonImpl::TransactionIds;
    switch (funcId) {
        case Ids::getVersionName: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->getVersionName(); }));
            return true;
        }
        case Ids::getVersionCode: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->getVersionCode(); }));
            return true;
        }
        case Ids::getBuildUuid: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->getBuildUuid(); }));
            return true;
        }
        case Ids::exitProcess: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->exitProcess(); }));
            return true;
        }
        case Ids::isDeviceSupported: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->isDeviceSupported(); }));
            return true;
        }
        case Ids::isHwServiceConnected: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->isHwServiceConnected(); }));
            return true;
        }
        case Ids::attachHwHidlService: {
            result = R::invoke(this, args, R::is(+[](T *p, const std::vector<std::string> &s) {
                return p->attachHwHidlService(s);
            }));
            return true;
        }
        case Ids::getSelfPid: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->getSelfPid(); }));
            return true;
        }
        case Ids::getHistoryIoOperations: {
            result = R::invoke(this, args, R::is(+[](T *p, uint32_t start, uint32_t length) {
                return p->getHistoryIoOperations(start, length);
            }));
            return true;
        }
        case Ids::clearHistoryIoEvents: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->clearHistoryIoEvents(); }));
            return true;
        }
        case Ids::getDaemonStatus: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->getDaemonStatus(); }));
            return true;
        }
        case Ids::deviceDriverIoctl0: {
            result = R::invoke(this, args, R::is(+[](T *p, uint32_t req, uint64_t arg) {
                return p->deviceDriverIoctl0(req, arg);
            }));
            return true;
        }
        case Ids::isOpDimLayerHbmEnabled: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->isOpDimLayerHbmEnabled(); }));
            return true;
        }
        case Ids::setOpDimLayerHbmEnabled: {
            result = R::invoke(this, args, R::is(+[](T *p, bool enabled) {
                return p->setOpDimLayerHbmEnabled(enabled);
            }));
            return true;
        }
        case Ids::isOpDimLayerHbmForceEnabled: {
            result = R::invoke(this, args, R::is(+[](T *p) { return p->isOpDimLayerHbmForceEnabled(); }));
            return true;
        }
        case Ids::setOpDimLayerHbmForceEnabled: {
            result = R::invoke(this, args, R::is(+[](T *p, bool enabled) {
                return p->setOpDimLayerHbmForceEnabled(enabled);
            }));
            return true;
        }
        case Ids::getLogsPartial: {
            result = R::invoke(this, args, R::is(+[](T *p, uint32_t start, uint32_t length) {
                return p->getLogsPartial(start, length);
            }));
            return true;
        }
        default:
            return false;
    }
}

bool DispTunerDaemonImpl::dispatchEvent(const IpcTransactor::LpcEnv &env, uint32_t eventId, const ArgList &args) {
    // we have no events to handle
    return false;
}

void DispTunerDaemonImpl::handleIoSyscallEvent(const IoSyscallEvent &event, const std::vector<uint8_t> &payload) {
    {
        // add the event to the queue tail
        std::scoped_lock lock(mEventMutex);
        mHistoryIoEvents.emplace_back(event, payload);
        // drop the oldest event if the queue is full
        if (mHistoryIoEvents.size() > MAX_HISTORY_EVENT_SIZE) {
            mHistoryIoEvents.pop_front();
        }
    }
    // notify the client
    IpcStateController::getInstance().getNciClientProxy().onIoEvent(event, payload);
}

TypedLpcResult<bool> DispTunerDaemonImpl::isDeviceSupported() {
    HwServiceStatus status = OpDispFeatureHandler::getHwServiceStatus();
    return status.valid;
}

TypedLpcResult<bool> DispTunerDaemonImpl::isHwServiceConnected() {
    HwServiceStatus status = OpDispFeatureHandler::getHwServiceStatus();
    return !status.currentAdapter.expired();
}

TypedLpcResult<ipcprotocol::IDispTunerDaemon::HistoryIoOperationEventList>
DispTunerDaemonImpl::getHistoryIoOperations(uint32_t start, uint32_t count) {
    IDispTunerDaemon::HistoryIoOperationEventList result;
    std::scoped_lock lock(mEventMutex);
    if (mHistoryIoEvents.empty()) {
        result.totalStartIndex = 0;
        result.totalCount = 0;
        return {result};
    }
    // history event list is not empty
    result.totalStartIndex = std::get<0>(mHistoryIoEvents.front()).uniqueSequence;
    result.totalCount = uint32_t(mHistoryIoEvents.size());
    // iterate the history events
    auto it = mHistoryIoEvents.cbegin();
    while (it != mHistoryIoEvents.cend()) {
        auto &[event, payload] = *it;
        if (event.uniqueSequence >= start) {
            // found the start index
            if (result.events.size() >= count) {
                break;
            }
            // add the event to the result list
            result.events.emplace_back(event);
            result.payloads.emplace_back(payload);
        }
        it++;
    }
    return {result};
}

/**
 * Get the HW service status, eg. connection with daemon, adapter status, etc.
 * @tparam TargetHalHandler The target HAL handler type.
 * @param halStatus The HW result service status, only valid if the return value is true.
 * @param lpcResult the exception result, only valid if the return value is false.
 * @param isConnected whether the HW service is connected to the daemon.
 * @return true if the HW service supported by the daemon, false otherwise.
 */
template<typename TargetHalHandler>
bool getHwServiceSupportStatus(HwServiceStatus &halStatus, LpcResult &lpcResult, bool &isConnected) {
    halStatus = TargetHalHandler::getHwServiceStatus();
    if (!halStatus.valid || halStatus.serviceProcessId <= 0) {
        std::string msg = std::string(typeid(TargetHalHandler).name()) + ": device or HAL service is not supported";
        LOGE("%s", msg.c_str());
        lpcResult = LpcResult::throwException({1, ENOTSUP, msg});
        isConnected = false;
        return false;
    }
    if (auto sp = halStatus.currentAdapter.lock()) {
        const auto *h = dynamic_cast<TargetHalHandler *>(sp.get());
        if (h && h->isConnected()) {
            // already connected
            isConnected = true;
            return true;
        }
    }
    isConnected = false;
    return true;
}

template<typename TargetHalHandler>
bool injectIntoVendorHalService(LpcResult &lpcResult, const HwServiceStatus &halStatus, const std::string &soPatchPath,
                                const std::string &targetSoName, uint32_t hookFunctions) {
    auto fd = auto_close_fd(open(soPatchPath.c_str(), O_RDONLY));
    if (!fd) {
        int err = errno;
        std::string errMsg = "failed to open " + soPatchPath + ": " + strerror(err);
        lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
        return false;
    }
    std::string shortName = soPatchPath.find_last_of('/') == std::string::npos ? soPatchPath :
                            soPatchPath.substr(soPatchPath.find_last_of('/') + 1);
    auto &serviceManager = ServiceManager::getInstance();
    SysServicePatch servicePatch;
    if (int err = servicePatch.init(shortName, fd.get(), TargetHalHandler::INIT_SYMBOL); err != 0) {
        std::string errMsg = "failed to init service patch: " + std::to_string(err);
        lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
        return false;
    }
    int targetPid = halStatus.serviceProcessId;
    LOGI("service patch initialized, service pid: %d", targetPid);
    if (int err = servicePatch.setSystemServicePid(targetPid); err != 0) {
        std::string errMsg = "failed to set service pid: " + std::to_string(err);
        lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
        return false;
    }
    if (!servicePatch.isServicePatched()) {
        LOGI("service patch is not applied, apply it now...");
        if (int err = servicePatch.patchService(); err != 0) {
            std::string errMsg = "failed to patch service: " + std::to_string(err);
            lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
            return false;
        }
    }
    int localSock = -1;
    int remoteSock = -1;
    if (int err = servicePatch.connectToService(localSock, remoteSock); err != 0) {
        std::string errMsg = "failed to connect to service: " + std::to_string(err);
        lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
        return false;
    }
    typename TargetHalHandler::StartupInfo info = {localSock, targetPid};
    auto[result, wp] = serviceManager.startService<TargetHalHandler>(&info);
    if (result < 0) {
        std::string errMsg = "failed to start service: " + std::to_string(result);
        lpcResult = LpcResult::throwException({1, uint32_t(result), errMsg});
        return false;
    }
    std::shared_ptr<IBaseService> spSvc;
    TargetHalHandler *service;
    if (!(spSvc = wp.lock()) || !(service = dynamic_cast<TargetHalHandler *>(spSvc.get()))) {
        std::string errMsg = "failed to start service";
        lpcResult = LpcResult::throwException({1, EFAULT, errMsg});
        return false;
    }
    int currentStatus = service->getRemotePltHookStatus();
    LOGI("service started, remote status: %d", currentStatus);
    if (currentStatus == 0) {
        OriginHookProcedure hookProc = {};
        if (int err = servicePatch.getPltHookEntries(hookProc, targetSoName, hookFunctions); err != 0) {
            std::string errMsg = "failed to get hook entries: " + std::to_string(err);
            lpcResult = LpcResult::throwException({1, uint32_t(err), errMsg});
            return false;
        }
        int initResult = service->initRemotePltHook(hookProc);
        if (initResult < 0) {
            std::string errMsg = "failed to init remote hook: " + std::to_string(initResult);
            lpcResult = LpcResult::throwException({1, uint32_t(initResult), errMsg});
            return false;
        } else {
            LOGI("remote hook initialized");
        }
    }
    return true;
}

TypedLpcResult<bool> DispTunerDaemonImpl::attachHwHidlService(const std::vector<std::string> &soPath) {
    // check argument count
    if (soPath.empty()) {
        std::string msg = "invalid arguments: required at least 2 strings: soPath_nxphalpatch, soPath_qtiesepatch";
        LOGE("%s", msg.c_str());
        return LpcResult::throwException({1, EINVAL, msg});
    }
    std::string soPath_opDispPanelFeature = soPath[0];
    // check absolute path
    if (soPath_opDispPanelFeature.find('/') != 0) {
        std::string msg = "invalid arguments: soPath_nxphalpatch or soPath_qtiesepatch is not absolute path";
        LOGE("%s", msg.c_str());
        return LpcResult::throwException({1, EINVAL, msg});
    }
    {
        using T = SysServicePatch::PltHookTarget;
        constexpr uint32_t nfcHalHook = T::OPEN | T::CLOSE | T::READ | T::WRITE | T::IOCTL;
        LpcResult tmpResult;
        HwServiceStatus halStatus = {};
        bool connected = false;
        if (!getHwServiceSupportStatus<OpDispFeatureHandler>(halStatus, tmpResult, connected)) {
            // hw hal not supported
            return tmpResult;
        }
        LOGV("OpDispFeatureHandler connected: %d", connected);
        if (!connected) {
            constexpr uint32_t dispFeatHalHook = T::OPEN | T::CLOSE | T::IOCTL | T::READ | T::WRITE;
            std::string shortExeName = halStatus.serviceExecPath
                    .substr(halStatus.serviceExecPath.find_last_of('/') + 1);
            // exe: vendor.qti.esepowermanager@X.Y-service
            // so: vendor.qti.esepowermanager@X.Y-impl.so
            // std::string soName = shortExeName.substr(0, shortExeName.find_last_of('-')) + "-impl.so";
            if (!injectIntoVendorHalService<OpDispFeatureHandler>(tmpResult, halStatus, soPath_opDispPanelFeature,
                                                                  halStatus.serviceName, dispFeatHalHook)) {
                return tmpResult;
            }
        }
    }
    return true;
}

TypedLpcResult<bool> DispTunerDaemonImpl::clearHistoryIoEvents() {
    std::scoped_lock lock(mEventMutex);
    bool isEmpty = mHistoryIoEvents.empty();
    mHistoryIoEvents.clear();
    return {!isEmpty};
}

template<typename TargetHalHandler>
static void fillInHalServiceStatus(IDispTunerDaemon::DaemonStatus::HalServiceStatus &svcStatus) {
    auto sp = TargetHalHandler::getWeakInstance();
    const TargetHalHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<TargetHalHandler *>(p)) && handler->isConnected()) {
        // HAL service is attached
        svcStatus.isHalServiceAttached = true;
        svcStatus.halServicePid = handler->getRemotePid();
    } else {
        svcStatus.isHalServiceAttached = false;
        auto unHwStatus = TargetHalHandler::getHwServiceStatus();
        svcStatus.halServicePid = unHwStatus.serviceProcessId;
    }
    if (svcStatus.halServicePid > 0) {
        if (utils::ProcessInfo procInfo = {}; utils::getProcessInfo(svcStatus.halServicePid, procInfo)) {
            svcStatus.halServiceUid = procInfo.uid;
            svcStatus.halServiceExePath = procInfo.exe;
        } else {
            LOGE("failed to get process info for pid: %d", svcStatus.halServicePid);
            svcStatus.halServiceUid = -1;
            svcStatus.halServiceExePath = "";
        }
        elfsym::ProcessView procView;
        if (int err = procView.readProcess(svcStatus.halServicePid); err == 0) {
            svcStatus.halServiceArch = procView.getArchitecture();
        } else {
            LOGE("failed to read process info for pid: %d, error: %d", svcStatus.halServicePid, err);
            svcStatus.halServiceArch = -1;
        }
        if (int err = SELinux::getProcessSecurityContext(svcStatus.halServicePid,
                                                         &svcStatus.halServiceProcessSecurityContext); err != 0) {
            LOGE("failed to get process security context for pid: %d, error: %d", svcStatus.halServicePid, err);
            svcStatus.halServiceProcessSecurityContext = "";
        }
        if (!svcStatus.halServiceExePath.empty()) {
            std::string label;
            if (int err = SELinux::getFileSEContext(svcStatus.halServiceExePath.c_str(), &label); err == 0) {
                svcStatus.halServiceExecutableSecurityLabel = label;
            } else {
                LOGE("failed to get file security context for path: %s, error: %d",
                     svcStatus.halServiceExePath.c_str(), err);
                svcStatus.halServiceExecutableSecurityLabel = "";
            }
        } else {
            svcStatus.halServiceExecutableSecurityLabel = "";
        }
    } else {
        svcStatus.halServiceUid = -1;
        svcStatus.halServiceExePath = "";
        svcStatus.halServiceArch = -1;
    }
}

TypedLpcResult<IDispTunerDaemon::DaemonStatus> DispTunerDaemonImpl::getDaemonStatus() {
    DaemonStatus status;
    status.processId = getpid();
    status.versionName = DISPLAY_TUNER_VERSION;
    std::string selfProcSecurityContext;
    if (int err = SELinux::getProcessSecurityContext(getpid(), &status.daemonProcessSecurityContext); err != 0) {
        status.daemonProcessSecurityContext = "";
        LOGE("failed to get self process security context: %d", err);
    }
    status.abiArch = utils::getCurrentProcessArchitecture();
    fillInHalServiceStatus<OpDispFeatureHandler>(status.opDispFeatureHidlServiceStatus);
    return {status};
}

TypedLpcResult<int> DispTunerDaemonImpl::deviceDriverIoctl0(uint32_t request, uint64_t arg) {
    auto sp = OpDispFeatureHandler::getWeakInstance();
    OpDispFeatureHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<OpDispFeatureHandler *>(p))) {
        int ret = handler->driverRawIoctl0(request, arg);
        return {ret};
    } else {
        return {-ENOSYS};
    }
}

TypedLpcResult<std::vector<LogEntryRecord>> DispTunerDaemonImpl::getLogsPartial(uint32_t startIndex, uint32_t count) {
    auto &logBuffer = logbuffer::LocalLogBuffer::getInstance();
    auto logs = logBuffer.getLogsPartial(startIndex, count);
    return {logs};
}

TypedLpcResult<bool> DispTunerDaemonImpl::isOpDimLayerHbmEnabled() {
    auto sp = OpDispFeatureHandler::getWeakInstance();
    OpDispFeatureHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<OpDispFeatureHandler *>(p))) {
        int ret = handler->isOpDimLayerHbmEnabled();
        if (ret < 0) {
            return TypedLpcResult<bool>::throwException({2, uint32_t(ret), "OpDispFeatureHandler returned error"});
        } else {
            return ret == 1;
        }
    } else {
        return TypedLpcResult<bool>::throwException({1, uint32_t(-ENOSYS), "OpDispFeatureHandler not available"});
    }
}

TypedLpcResult<void> DispTunerDaemonImpl::setOpDimLayerHbmEnabled(bool enabled) {
    auto sp = OpDispFeatureHandler::getWeakInstance();
    OpDispFeatureHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<OpDispFeatureHandler *>(p))) {
        int ret = handler->setOpDimLayerHbmEnabled(enabled);
        if (ret < 0) {
            return TypedLpcResult<void>::throwException({2, uint32_t(ret), "OpDispFeatureHandler returned error"});
        }
        return TypedLpcResult<void>::makeVoid();
    } else {
        return TypedLpcResult<void>::throwException({1, uint32_t(-ENOSYS), "OpDispFeatureHandler not available"});
    }
}

TypedLpcResult<bool> DispTunerDaemonImpl::isOpDimLayerHbmForceEnabled() {
    auto sp = OpDispFeatureHandler::getWeakInstance();
    OpDispFeatureHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<OpDispFeatureHandler *>(p))) {
        int ret = handler->isOpDimLayerHbmForceEnabled();
        if (ret < 0) {
            return TypedLpcResult<bool>::throwException({2, uint32_t(ret), "OpDispFeatureHandler returned error"});
        } else {
            return ret == 1;
        }
    } else {
        return TypedLpcResult<bool>::throwException({1, uint32_t(-ENOSYS), "OpDispFeatureHandler not available"});
    }
}

TypedLpcResult<void> DispTunerDaemonImpl::setOpDimLayerHbmForceEnabled(bool enabled) {
    auto sp = OpDispFeatureHandler::getWeakInstance();
    OpDispFeatureHandler *handler;
    if (auto p = sp.get(); p != nullptr && (handler = dynamic_cast<OpDispFeatureHandler *>(p))) {
        int ret = handler->setOpDimLayerHbmForceEnabled(enabled);
        if (ret < 0) {
            return TypedLpcResult<void>::throwException({2, uint32_t(ret), "OpDispFeatureHandler returned error"});
        }
        return TypedLpcResult<void>::makeVoid();
    } else {
        return TypedLpcResult<void>::throwException({1, uint32_t(-ENOSYS), "OpDispFeatureHandler not available"});
    }
}
