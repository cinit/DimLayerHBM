package cc.ioctl.opdisplaytuner.startup.step;

import android.content.Context;

import com.tencent.mmkv.MMKV;

import java.io.File;

import cc.ioctl.opdisplaytuner.startup.BaseApplicationImpl;
import cc.ioctl.opdisplaytuner.startup.Step;

public class StartDaemonService extends Step {
    @Override
    public boolean doStep() {
        Context ctx = BaseApplicationImpl.getInstance();
        System.loadLibrary("mmkv");
        MMKV.initialize(ctx);
        File dir = ctx.getNoBackupFilesDir();
        if (!dir.exists()) {
            dir.mkdirs();
        }
//        try {
//            Thread.sleep(300);
//        } catch (InterruptedException e) {
//            e.printStackTrace();
//        }
        return true;
    }
}
