package cc.ioctl.opdisplaytuner.activity.ui.dump;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.util.ArrayList;
import java.util.Arrays;

import cc.ioctl.opdisplaytuner.decoder.NxpHalV2EventTranslator;
import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;
import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;

public class NciLiveDumpViewModel extends ViewModel implements IpcNativeHandler.IpcConnectionListener, IDispTunerDaemon.OnRemoteEventListener {

    private final MutableLiveData<IDispTunerDaemon> mNciHostDaemon;
    private final MutableLiveData<ArrayList<NxpHalV2EventTranslator.TransactionEvent>> mTransactionEvents = new MutableLiveData<>(new ArrayList<>());
    private final MutableLiveData<ArrayList<IDispTunerDaemon.IoEventPacket>> mAuxIoEvents = new MutableLiveData<>(new ArrayList<>());
    private final NxpHalV2EventTranslator mTranslator = new NxpHalV2EventTranslator();

    public NciLiveDumpViewModel() {
        mNciHostDaemon = new MutableLiveData<>();
        IpcNativeHandler.registerConnectionListener(this);
        mNciHostDaemon.setValue(IpcNativeHandler.peekConnection());
        IDispTunerDaemon daemon = mNciHostDaemon.getValue();
        if (daemon != null && daemon.isConnected()) {
            daemon.registerRemoteEventListener(this);
            resetAndSyncIoPackets();
        }
    }

    @Override
    public void onDisconnect(IDispTunerDaemon daemon) {
        mTranslator.resetDecoders();
        mNciHostDaemon.postValue(null);
        daemon.unregisterRemoteEventListener(this);
    }

    @Override
    public void onIoEvent(IDispTunerDaemon.IoEventPacket event) {
        int lastSeq = mTranslator.getLastRawEventSequence();
        if (event.sequence == lastSeq + 1) {
            int updateResult = mTranslator.pushBackRawIoEvent(event);
            if ((updateResult & 2) != 0) {
                // notify the observer
                mTransactionEvents.postValue(mTranslator.getTransactionEvents());
            }
            if ((updateResult & 1) != 0) {
                // notify the observer
                mAuxIoEvents.postValue(mTranslator.getAuxIoEvents());
            }
        } else {
            // we are out of sync, sync now
            synchronizeIoEvents();
        }
    }

    @Override
    public void onRemoteDeath(IDispTunerDaemon.RemoteDeathPacket event) {
        // ignore
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        IDispTunerDaemon daemon = mNciHostDaemon.getValue();
        if (daemon != null) {
            daemon.unregisterRemoteEventListener(this);
        }
        IpcNativeHandler.unregisterConnectionListener(this);
    }

    @Override
    public void onConnect(IDispTunerDaemon daemon) {
        mNciHostDaemon.postValue(daemon);
        daemon.registerRemoteEventListener(this);
        resetAndSyncIoPackets();
    }


    public void synchronizeIoEvents() {
        IDispTunerDaemon daemon = mNciHostDaemon.getValue();
        if (daemon != null && daemon.isConnected()) {
            long startIndex = mTranslator.getLastRawEventSequence();
            final int maxCountPerRequest = 100;
            long start = startIndex;
            IDispTunerDaemon.HistoryIoEventList historyList;
            ArrayList<IDispTunerDaemon.IoEventPacket> deltaRawIoEventPackets = new ArrayList<>();
            do {
                historyList = daemon.getHistoryIoEvents((int) start, maxCountPerRequest);
                if (historyList.events.length > 0) {
                    deltaRawIoEventPackets.addAll(Arrays.asList(historyList.events));
                    start = historyList.events[historyList.events.length - 1].sequence + 1;
                }
                // if we got less than maxCountPerRequest, we have reached the end
            } while (historyList.events.length >= maxCountPerRequest);
            if (deltaRawIoEventPackets.size() > 0) {
                int updateStatus = mTranslator.pushBackRawIoEvents(deltaRawIoEventPackets);
                // notify the observer
                if ((updateStatus & 2) != 0) {
                    mTransactionEvents.postValue(mTranslator.getTransactionEvents());
                }
                if ((updateStatus & 1) != 0) {
                    mAuxIoEvents.postValue(mTranslator.getAuxIoEvents());
                }
            }
        }
    }

    public void clearTransactionEvents() {
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null) {
            daemon.clearHistoryIoEvents();
        }
        mTranslator.clearTransactionEvents();
        mTransactionEvents.postValue(mTranslator.getTransactionEvents());
        mAuxIoEvents.postValue(mTranslator.getAuxIoEvents());
    }

    /**
     * Reset the local event list and synchronize with the daemon
     * This is called when the daemon is connected or ViewModel is created
     * Avoid calling this method on the main thread
     */
    private void resetAndSyncIoPackets() {
        IDispTunerDaemon daemon = mNciHostDaemon.getValue();
        if (daemon == null || !daemon.isConnected()) {
            return;
        }
        // clear the old data
        mTranslator.clearTransactionEvents();
        // pull the new history data
        IDispTunerDaemon.HistoryIoEventList historyList = daemon.getHistoryIoEvents(0, 0);
        int startIndex = historyList.totalStartIndex;
        final int maxCountPerRequest = 100;
        int start = startIndex;
        ArrayList<IDispTunerDaemon.IoEventPacket> deltaRawIoEventPackets = new ArrayList<>();
        do {
            historyList = daemon.getHistoryIoEvents(start, maxCountPerRequest);
            if (historyList.events.length > 0) {
                deltaRawIoEventPackets.addAll(Arrays.asList(historyList.events));
                start = historyList.events[historyList.events.length - 1].sequence + 1;
            }
            // if we got less than maxCountPerRequest, we have reached the end
        } while (historyList.events.length >= maxCountPerRequest);
        // translate events to transactions
        mTranslator.pushBackRawIoEvents(deltaRawIoEventPackets);
        // notify the observer
        mTransactionEvents.postValue(mTranslator.getTransactionEvents());
        mAuxIoEvents.postValue(mTranslator.getAuxIoEvents());
    }

    public MutableLiveData<ArrayList<NxpHalV2EventTranslator.TransactionEvent>> getTransactionEvents() {
        return mTransactionEvents;
    }

    public MutableLiveData<ArrayList<IDispTunerDaemon.IoEventPacket>> getAuxIoEvents() {
        return mAuxIoEvents;
    }

    public NxpHalV2EventTranslator getTranslator() {
        return mTranslator;
    }
}
