package cc.ioctl.opdisplaytuner.activity.ui.home;

import android.util.Log;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;
import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;

public class HomeViewModel extends ViewModel implements IpcNativeHandler.IpcConnectionListener {

    private final MutableLiveData<IDispTunerDaemon.DaemonStatus> nciHostDaemonStatus = new MutableLiveData<>();

    public HomeViewModel() {
        IpcNativeHandler.registerConnectionListener(this);
        Log.i("HomeViewModel", "HomeViewModel created");
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null && daemon.isConnected()) {
            IDispTunerDaemon.DaemonStatus status = daemon.getDaemonStatus();
            nciHostDaemonStatus.setValue(status);
        }
    }

    public LiveData<IDispTunerDaemon.DaemonStatus> getDaemonStatus() {
        return nciHostDaemonStatus;
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        IpcNativeHandler.unregisterConnectionListener(this);
    }

    public void refreshDaemonStatus() {
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null && daemon.isConnected()) {
            IDispTunerDaemon.DaemonStatus status = daemon.getDaemonStatus();
            nciHostDaemonStatus.postValue(status);
        } else {
            nciHostDaemonStatus.postValue(null);
        }
    }

    public boolean isDisplayFeatureHidlAttached() {
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null && daemon.isConnected()) {
            return daemon.isDisplayFeatureHidlServivceAttached();
        }
        return false;
    }

    public IDispTunerDaemon getDaemonIfDisplayFeatureHidlAttached() {
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null && daemon.isConnected() && daemon.isDisplayFeatureHidlServivceAttached()) {
            return daemon;
        }
        return null;
    }

    @Override
    public void onConnect(IDispTunerDaemon daemon) {
        IDispTunerDaemon.DaemonStatus status = daemon.getDaemonStatus();
        nciHostDaemonStatus.postValue(status);
    }

    @Override
    public void onDisconnect(IDispTunerDaemon daemon) {
        nciHostDaemonStatus.postValue(null);
    }
}
