package cc.ioctl.opdisplaytuner.startup.step;

import android.content.Context;

import com.tencent.mmkv.MMKV;

import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;
import cc.ioctl.opdisplaytuner.startup.BaseApplicationImpl;
import cc.ioctl.opdisplaytuner.startup.Step;
import cc.ioctl.opdisplaytuner.util.config.ConfigManager;

public class EarlyInit extends Step {
    @Override
    public boolean doStep() {
        // init mmkv config
        Context ctx = BaseApplicationImpl.getInstance();
        System.loadLibrary("mmkv");
        MMKV.initialize(ctx);
        MMKV.mmkvWithID("default_config");
        ConfigManager.getDefaultConfig();
        ConfigManager.getCache();
        // if in service process, init the NCI daemon IPC
        if (BaseApplicationImpl.isServiceProcess()) {
            IpcNativeHandler.init(ctx);
            IpcNativeHandler.initForSocketDir();
        }
        return true;
    }
}
