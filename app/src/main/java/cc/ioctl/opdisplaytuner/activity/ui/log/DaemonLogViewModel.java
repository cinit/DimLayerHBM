package cc.ioctl.opdisplaytuner.activity.ui.log;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.util.ArrayList;
import java.util.Arrays;

import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;
import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;

public class DaemonLogViewModel extends ViewModel {

    private static final String TAG = "DaemonLogViewModel";

    private MutableLiveData<ArrayList<IDispTunerDaemon.LogEntryRecord>> mLogEntries;
    private ArrayList<IDispTunerDaemon.LogEntryRecord> mLogEntriesArrayList;
    private int mLastLogEntrySequence = 0;

    public DaemonLogViewModel() {
        super();
        mLogEntriesArrayList = new ArrayList<>();
        mLogEntries = new MutableLiveData<>(mLogEntriesArrayList);
    }

    public void refreshLogs() {
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon != null && daemon.isConnected()) {
            long startIndex = mLastLogEntrySequence;
            final int maxCountPerRequest = 100;
            long start = startIndex;
            IDispTunerDaemon.LogEntryRecord[] historyLogs;
            ArrayList<IDispTunerDaemon.LogEntryRecord> deltaLogEntries = new ArrayList<>();
            do {
                historyLogs = daemon.getLogsPartial((int) start, maxCountPerRequest);
                if (historyLogs.length > 0) {
                    deltaLogEntries.addAll(Arrays.asList(historyLogs));
                    start = historyLogs[historyLogs.length - 1].id + 1;
                }
                // if we got less than maxCountPerRequest, we have reached the end
            } while (historyLogs.length >= maxCountPerRequest);
            if (deltaLogEntries.size() > 0) {
                mLogEntries.postValue(deltaLogEntries);
            }
        }
    }

    public LiveData<ArrayList<IDispTunerDaemon.LogEntryRecord>> getLogEntries() {
        return mLogEntries;
    }
}
