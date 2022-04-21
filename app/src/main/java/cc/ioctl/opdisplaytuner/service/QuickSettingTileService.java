package cc.ioctl.opdisplaytuner.service;

import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.Nullable;

import java.lang.ref.WeakReference;

import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;
import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;
import cc.ioctl.opdisplaytuner.util.ThreadManager;

public class QuickSettingTileService extends TileService implements IpcNativeHandler.IpcConnectionListener {

    private static final String TAG = "QuickSettingTileService";
    private boolean mServiceStarted = false;
    private boolean mIsListening = false;
    private static WeakReference<QuickSettingTileService> sServiceRef = null;

    @Override
    public void onDestroy() {
        super.onDestroy();
        mServiceStarted = false;
        sServiceRef.clear();
        sServiceRef = null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        IpcNativeHandler.init(this);
        IpcNativeHandler.initForSocketDir();
        mServiceStarted = true;
        sServiceRef = new WeakReference<>(this);
    }

    @Nullable
    public static QuickSettingTileService getServiceIfRunning() {
        if (sServiceRef == null) {
            return null;
        }
        return sServiceRef.get();
    }

    public void invalidateTileState() {
        if (mServiceStarted && mIsListening) {
            updateTileState();
        }
    }

    @Override
    public void onTileAdded() {
        super.onTileAdded();
    }

    @Override
    public void onTileRemoved() {
        super.onTileRemoved();
    }

    @Override
    public void onClick() {
        Tile tile = getQsTile();
        if (tile == null) {
            return;
        }
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon == null || !daemon.isConnected() || !daemon.isDisplayFeatureHidlServivceAttached()) {
            return;
        }
        boolean currentState = tile.getState() == Tile.STATE_ACTIVE;
        boolean targetState = !currentState;
        try {
            daemon.setDimLayerHbmForceEnabled(targetState);
            daemon.setDimLayerHbmEnabled(targetState);
        } catch (Exception e) {
            Log.e(TAG, "onClick: setDimLayerHbmEnabled/setDimLayerHbmForceEnabled error", e);
            ThreadManager.runOnUiThread(() -> Toast.makeText(this, e.toString(), Toast.LENGTH_SHORT).show());
        }
        updateTileState();
    }

    @Override
    public void onStartListening() {
        IpcNativeHandler.registerConnectionListener(this);
        mIsListening = true;
        updateTileState();
    }

    @Override
    public void onStopListening() {
        IpcNativeHandler.unregisterConnectionListener(this);
        mIsListening = false;
    }

    @Override
    public void onConnect(IDispTunerDaemon daemon) {
        if (mServiceStarted) {
            updateTileState();
        }
    }

    @Override
    public void onDisconnect(IDispTunerDaemon daemon) {
        if (mServiceStarted) {
            updateTileState();
        }
    }

    private void updateTileState() {
        if (!mIsListening) {
            return;
        }
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        Tile tile = getQsTile();
        if (tile == null) {
            return;
        }
        if (daemon == null || !daemon.isConnected() || !daemon.isDisplayFeatureHidlServivceAttached()) {
            tile.setState(Tile.STATE_UNAVAILABLE);
        } else {
            boolean isOn = false;
            try {
                isOn = daemon.isDimLayerHbmEnabled() || daemon.isDimLayerHbmForceEnabled();
            } catch (Exception e) {
                Log.e(TAG, "updateTileState: isDimLayerHbmEnabled/isDimLayerHbmForceEnabled error", e);
            }
            tile.setState(isOn ? Tile.STATE_ACTIVE : Tile.STATE_INACTIVE);
        }
        tile.updateTile();
    }
}
