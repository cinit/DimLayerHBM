package cc.ioctl.opdisplaytuner.util.config;

import androidx.annotation.Keep;

import cc.ioctl.opdisplaytuner.util.annotation.ConfigGroup;
import cc.ioctl.opdisplaytuner.util.config.LazyNonNullMmkvItemConfig;

@Keep
@ConfigGroup("core")
public class GenericConfig {

    public static final NonNullConfigItem<Boolean> cfgStartDaemonStartBoot =
            new LazyNonNullMmkvItemConfig<>("core", "start_daemon_after_boot", false);
    public static final NonNullConfigItem<String> cfgCustomSuCommand =
            new LazyNonNullMmkvItemConfig<>("core", "cfg_custom_su_command", "");
    public static final NonNullConfigItem<String> cfgNfcDeviceType =
            new LazyNonNullMmkvItemConfig<>("core", "cfg_nfc_device_type", "unknown");
    public static final NonNullConfigItem<Boolean> cfgDisableNfcDiscoverySound =
            new LazyNonNullMmkvItemConfig<>("core", "cfg_disable_nfc_discovery_sound", false);
    public static final NonNullConfigItem<Boolean> cfgEnableNfcEmulationWhenScreenOff =
            new NonNullConstantValueConfigItem<>(false, false);

}
