//
// Created by kinit on 2021-11-17.
//
#include <string>

#include "libopdispfeatpatch/ipc/ipc_requests.h"
#include "libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "rpcprotocol/log/Log.h"
#include "rpcprotocol/utils/ProcessUtils.h"
#include "../../../ipc/IpcStateController.h"

#include "OpDispFeatureHandler.h"

using namespace hwhal;
using namespace halpatch;
using namespace ipcprotocol;
using namespace halpatch::opdispfeature;

static const char *const LOG_TAG = "OpDispFeatureHandler";

static constexpr auto SYS_HBM_PATH = "/sys/kernel/oplus_display/dimlayer_hbm";

std::weak_ptr<IBaseService> OpDispFeatureHandler::sWpInstance;

std::string_view OpDispFeatureHandler::getName() const noexcept {
    return "OpDispFeatureHandler";
}

std::string_view OpDispFeatureHandler::getDescription() const noexcept {
    return mDescription;
}

int OpDispFeatureHandler::doOnStart(void *args, const std::shared_ptr<IBaseService> &sp) {
    mDescription = "OpDispFeatureHandler@fd" + std::to_string(getFd()) + "-pid-" + std::to_string(getRemotePid());
    sWpInstance = sp;
    return 0;
}

bool OpDispFeatureHandler::doOnStop() {
    return true;
}

void OpDispFeatureHandler::dispatchHwHalIoEvent(const IoSyscallEvent &event, const void *payload) {
    std::vector<uint8_t> payloadVec;
    if (payload != nullptr && event.info.bufferLength > 0) {
        payloadVec.assign(static_cast<const uint8_t *>(payload),
                          static_cast<const uint8_t *>(payload) + event.info.bufferLength);
    }
    IoSyscallEvent eventCopy = event;
    DispTunerDaemonImpl &daemonImpl = IpcStateController::getInstance().getNciHostDaemon();
    // fill the missing fields
    eventCopy.uniqueSequence = daemonImpl.nextIoSyscallEventSequence();
    eventCopy.sourceType = 0;
    daemonImpl.handleIoSyscallEvent(eventCopy, payloadVec);
}

void OpDispFeatureHandler::dispatchRemoteProcessDeathEvent() {
    LOGW("Remote process death event received, pid: %d", getRemotePid());
    IpcStateController::getInstance().getNciClientProxy().onRemoteDeath(getRemotePid());
}

int OpDispFeatureHandler::getRemotePltHookStatus() {
    HalRequest request = {};
    request.requestCode = static_cast<uint32_t>(RequestId::GET_HOOK_STATUS);
    auto[rc, response, payload] = sendHalRequest(request, nullptr, 1000);
    if (rc < 0) {
        LOGE("Failed to send request, rc=%d", rc);
        return rc;
    }
    return int(response.result);
}

int OpDispFeatureHandler::initRemotePltHook(const OriginHookProcedure &hookProc) {
    HalRequest request = {};
    request.requestCode = static_cast<uint32_t>(RequestId::INIT_PLT_HOOK);
    request.payloadSize = sizeof(hookProc);
    auto[rc, response, payload] = sendHalRequest(request, &hookProc, 1000);
    if (rc < 0) {
        LOGE("Failed to send request, rc=%d", rc);
        return rc;
    }
    if (response.error != 0) {
        std::string errorMsg;
        if (response.payloadSize != 0 && !payload.empty()) {
            errorMsg = std::string(reinterpret_cast<const char *>(payload.data()), payload.size());
        }
        LOGE("Failed to init remote PLT hook, remote error=%d, %s", response.error, errorMsg.c_str());
        return -int(response.error);
    }
    return int(response.result);
}

HwServiceStatus OpDispFeatureHandler::getHwServiceStatus() {
    HwServiceStatus status = {};
    // test hw device compatibility
    if (access(SYS_HBM_PATH, F_OK) != 0) {
        status.valid = false;
        return status;
    }
    // test hal driver service compatibility
    auto processList = utils::getRunningProcessInfo();
    if (processList.empty()) {
        LOGE("Unable to get running process info");
        status.valid = false;
        return status;
    }
    for (const auto &p: processList) {
        if (!p.name.empty() && p.uid >= 1000 && p.uid < 10000) {
            bool nameMatch = false;
            std::vector<std::string> availableVersions = {"1.0"};
            for (const auto &v: availableVersions) {
                if (p.name == (std::string(EXEC_NAME_BASE) + v + std::string(EXEC_NAME_SUFFIX))) {
                    nameMatch = true;
                    break;
                }
            }
            if (!nameMatch) {
                continue;
            }
            // check executable path, everything is OK except the user writable path
            std::vector<std::string> allowedPathPrefixes = {
                    "/system/",
                    "/system_ext/",
                    "/vendor/",
                    "/oem/",
                    "/odm/",
                    "/dev/", // wtf?
                    "/product/",
                    "/apex/",
                    "/bin/",
                    "/xbin/",
                    "/sbin/"
            };
            bool pathMatch = false;
            for (const auto &prefix: allowedPathPrefixes) {
                if (p.exe.find(prefix) == 0) {
                    pathMatch = true;
                    break;
                }
            }
            if (!pathMatch) {
                continue;
            }
            // found hal driver service
            status.valid = true;
            status.serviceName = p.name;
            status.serviceProcessId = p.pid;
            status.serviceExecPath = p.exe;
            status.devicePath = SYS_HBM_PATH;
            if (auto sp = sWpInstance.lock(); sp) {
                IBaseService *pService = sp.get();
                const OpDispFeatureHandler *h;
                if (pService != nullptr && (h = dynamic_cast<OpDispFeatureHandler *>(pService)) != nullptr
                    && h->isConnected()) {
                    status.currentAdapter = sp;
                }
            }
            return status;
        }
    }
    // no service found
    status.valid = false;
    LOGV("No nxp nfc hal driver service found");
    return status;
}

std::shared_ptr<IBaseService> OpDispFeatureHandler::getWeakInstance() {
    return sWpInstance.lock();
}

int OpDispFeatureHandler::driverRawWrite(const std::vector<uint8_t> &buffer) {
    HalRequest request = {};
    request.requestCode = static_cast<uint32_t>(RequestId::DEVICE_OPERATION_WRITE);
    std::vector<uint8_t> requestPayload;
    requestPayload.resize(4 + buffer.size());
    *reinterpret_cast<uint32_t *>(&requestPayload[0]) = uint32_t(buffer.size());
    memcpy(&requestPayload[4], buffer.data(), buffer.size());
    request.payloadSize = uint32_t(requestPayload.size());
    auto[rc, response, payload] = sendHalRequest(request, requestPayload.data(), 1000);
    if (rc < 0) {
        LOGE("driverRawWrite: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}

int OpDispFeatureHandler::driverRawIoctl0(uint32_t request, uint64_t arg) {
    HalRequest requestPacket = {};
    requestPacket.requestCode = static_cast<uint32_t>(RequestId::DEVICE_OPERATION_IOCTL0);
    DeviceIoctl0Request requestPayload = {};
    requestPayload.request = request;
    requestPayload.arg = arg;
    requestPacket.payloadSize = uint32_t(sizeof(DeviceIoctl0Request));
    auto[rc, response, payload] = sendHalRequest(requestPacket, &requestPayload, 1000);
    if (rc < 0) {
        LOGE("driverRawIoctl0: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}

int OpDispFeatureHandler::isOpDimLayerHbmEnabled() {
    HalRequest requestPacket = {};
    requestPacket.requestCode = static_cast<uint32_t>(RequestId::OP_DISP_GET_DIM_LAYER_HBM);
    auto[rc, response, payload] = sendHalRequest(requestPacket, nullptr, 1000);
    if (rc < 0) {
        LOGE("isOpDimLayerHbmEnabled: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}

int OpDispFeatureHandler::setOpDimLayerHbmEnabled(bool enabled) {
    HalRequest requestPacket = {};
    uint32_t v = enabled ? 1 : 0;
    requestPacket.requestCode = static_cast<uint32_t>(RequestId::OP_DISP_SET_DIM_LAYER_HBM);
    requestPacket.payloadSize = 4;
    auto[rc, response, payload] = sendHalRequest(requestPacket, &v, 1000);
    if (rc < 0) {
        LOGE("setOpDimLayerHbmEnabled: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}

int OpDispFeatureHandler::isOpDimLayerHbmForceEnabled() {
    HalRequest requestPacket = {};
    requestPacket.requestCode = static_cast<uint32_t>(RequestId::OP_DISP_GET_DIM_LAYER_HBM_FORCE_STATUS);
    auto[rc, response, payload] = sendHalRequest(requestPacket, nullptr, 1000);
    if (rc < 0) {
        LOGE("isOpDimLayerHbmForceEnabled: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}

int OpDispFeatureHandler::setOpDimLayerHbmForceEnabled(bool enabled) {
    HalRequest requestPacket = {};
    uint32_t v = enabled ? 1 : 0;
    requestPacket.requestCode = static_cast<uint32_t>(RequestId::OP_DISP_SET_DIM_LAYER_HBM_FORCE_STATUS);
    requestPacket.payloadSize = 4;
    auto[rc, response, payload] = sendHalRequest(requestPacket, &v, 1000);
    if (rc < 0) {
        LOGE("setOpDimLayerHbmForceEnabled: failed to send request, rc=%d", rc);
        return -std::abs(rc);
    }
    return int(response.result);
}
