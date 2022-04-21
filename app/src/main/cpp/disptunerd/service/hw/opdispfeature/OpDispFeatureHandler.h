//
// Created by kinit on 2021-11-17.
//

#ifndef DISP_TUNER_NATIVES_NXPHALHANDLER_H
#define DISP_TUNER_NATIVES_NXPHALHANDLER_H

#include <string>
#include <memory>
#include <deque>
#include <mutex>
#include <tuple>

#include "rpcprotocol/IDispTunerDaemon.h"
#include "../../HwServiceStatus.h"
#include "../BaseHwHalHandler.h"

namespace hwhal {

class OpDispFeatureHandler : public BaseHwHalHandler {
public:
    static constexpr const char *EXEC_NAME_BASE = "vendor.oplus.hardware.displaypanelfeature@";
    static constexpr const char *EXEC_NAME_SUFFIX = "-service";
    static constexpr const char *TARGET_SO_NAME = "vendor.oplus.hardware.displaypanelfeature@1.0-service";
    static constexpr const char *INIT_SYMBOL = "op_disp_feature_patch_inject_init";

    using StartupInfo = BaseHwHalHandler::StartupInfo;

    OpDispFeatureHandler() = default;

    ~OpDispFeatureHandler() override = default;

    [[nodiscard]] std::string_view getName() const noexcept override;

    [[nodiscard]] std::string_view getDescription() const noexcept override;

    [[nodiscard]] int doOnStart(void *args, const std::shared_ptr<IBaseService> &sp) override;

    [[nodiscard]] bool doOnStop() override;

    [[nodiscard]] int getRemotePltHookStatus();

    [[nodiscard]] int initRemotePltHook(const OriginHookProcedure &hookProc);

    void dispatchHwHalIoEvent(const halpatch::IoSyscallEvent &event, const void *payload) override;

    void dispatchRemoteProcessDeathEvent() override;

    [[nodiscard]] int driverRawWrite(const std::vector<uint8_t> &buffer) override;

    [[nodiscard]] int driverRawIoctl0(uint32_t request, uint64_t arg) override;

    [[nodiscard]] int isOpDimLayerHbmEnabled();

    [[nodiscard]] int setOpDimLayerHbmEnabled(bool enabled);

    [[nodiscard]] int isOpDimLayerHbmForceEnabled();

    [[nodiscard]] int setOpDimLayerHbmForceEnabled(bool enabled);

    [[nodiscard]] static HwServiceStatus getHwServiceStatus();

    [[nodiscard]] static std::shared_ptr<IBaseService> getWeakInstance();

private:
    std::string mDescription;
    static std::weak_ptr<IBaseService> sWpInstance;
};

}

#endif //DISP_TUNER_NATIVES_NXPHALHANDLER_H
