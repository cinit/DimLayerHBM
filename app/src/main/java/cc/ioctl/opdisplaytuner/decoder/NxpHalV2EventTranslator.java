package cc.ioctl.opdisplaytuner.decoder;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;

public class NxpHalV2EventTranslator {
    private final HashMap<Integer, IDispTunerDaemon.IoEventPacket> mRawIoEventPackets = new HashMap<>();
    private int mLastRawEventSequence = 0;
    // guarded by this
    private final ArrayList<TransactionEvent> mTransactionEvents = new ArrayList<>();
    // guarded by this
    private final ArrayList<IDispTunerDaemon.IoEventPacket> mAuxIoEvents = new ArrayList<>();
    // guarded by this
    private int mLastUpdateTransactionSequence = 0;
    // guarded by this
    private int mLastSuccessTransactionSequence = 0;

    private final NxpHalV2IoEventHandler mNxpHalIoEventHandler = new NxpHalV2IoEventHandler();

    public ArrayList<TransactionEvent> getTransactionEvents() {
        return mTransactionEvents;
    }

    public ArrayList<IDispTunerDaemon.IoEventPacket> getAuxIoEvents() {
        return mAuxIoEvents;
    }

    public enum Direction {
        /**
         * write and ioctl
         */
        HOST_TO_DEVICE,
        /**
         * read
         */
        DEVICE_TO_HOST
    }

    public static class TransactionEvent {
        public long sequence;
        public int sourceType;
        public int sourceSequence;
        public long timestamp;
    }

    public static class RawTransactionEvent extends TransactionEvent {
        public IDispTunerDaemon.IoEventPacket packet;

        public RawTransactionEvent(IDispTunerDaemon.IoEventPacket rawIoEventPacket) {
            this.sequence = rawIoEventPacket.sequence;
            this.sourceType = rawIoEventPacket.sourceType;
            this.sourceSequence = rawIoEventPacket.sourceSequence;
            this.timestamp = rawIoEventPacket.timestamp;
            this.packet = rawIoEventPacket;
        }
    }

    private synchronized int translateRawIoEvents() {
        ArrayList<TransactionEvent> transactionEvents = mTransactionEvents;
        boolean auxIoEventUpdated = false;
        boolean transactionEventUpdated = false;
        while (mLastUpdateTransactionSequence < mLastRawEventSequence) {
            // translate the events
            IDispTunerDaemon.IoEventPacket eventPacket = mRawIoEventPackets.get(mLastUpdateTransactionSequence + 1);
            if (eventPacket != null) {
                // do not ignore the failed event for mAuxIoEvents
                NxpHalV2IoEventHandler.BaseEvent baseEvent = mNxpHalIoEventHandler.update(eventPacket);
                mAuxIoEvents.add(eventPacket);
                auxIoEventUpdated = true;
                boolean isFailedReadOrWrite = (eventPacket.opType == IDispTunerDaemon.IoEventPacket.IoOperationType.READ
                        || eventPacket.opType == IDispTunerDaemon.IoEventPacket.IoOperationType.WRITE)
                        && eventPacket.retValue < 0;
                if (isFailedReadOrWrite) {
                    // failed read or write, ignore
                    mLastUpdateTransactionSequence++;
                    continue;
                }
                if (baseEvent != null) {
                    if (baseEvent instanceof NxpHalV2IoEventHandler.WriteEvent
                            || baseEvent instanceof NxpHalV2IoEventHandler.ReadEvent) {
                        // NCI packet
                        Direction direction = (baseEvent instanceof NxpHalV2IoEventHandler.ReadEvent) ?
                                Direction.DEVICE_TO_HOST : Direction.HOST_TO_DEVICE;
                        byte[] data = (baseEvent instanceof NxpHalV2IoEventHandler.ReadEvent) ?
                                ((NxpHalV2IoEventHandler.ReadEvent) baseEvent).data
                                : ((NxpHalV2IoEventHandler.WriteEvent) baseEvent).data;
                        // FF: NXP NCI decoder removed
                        // decode failed, push as a raw event
                        RawTransactionEvent result = new RawTransactionEvent(eventPacket);
                        transactionEvents.add(result);
                    } else {
                        // ioctl event
                        RawTransactionEvent result = new RawTransactionEvent(eventPacket);
                        transactionEvents.add(result);
                    }
                    transactionEventUpdated = true;
                    mLastSuccessTransactionSequence = eventPacket.sequence;
                }
            }
            mLastUpdateTransactionSequence++;
        }
        return (auxIoEventUpdated ? 1 : 0) | (transactionEventUpdated ? 2 : 0);
    }

    public StringBuilder exportRawEventsAsCsv() {
        StringBuilder sb = new StringBuilder();
        for (IDispTunerDaemon.IoEventPacket eventPacket : mRawIoEventPackets.values()) {
            sb.append(eventPacket.sequence).append(",");
            sb.append(eventPacket.timestamp).append(",");
            sb.append(eventPacket.fd).append(",");
            sb.append(eventPacket.opType.getValue()).append(",");
            sb.append(eventPacket.retValue).append(",");
            sb.append(eventPacket.directArg1).append(",");
            sb.append(eventPacket.directArg2).append(",");
            sb.append(Arrays.toString(eventPacket.buffer).replace(" ", "")).append("\n");
        }
        return sb;
    }

    public static ArrayList<IDispTunerDaemon.IoEventPacket> loadIoEventPacketsFromString(@NonNull String source) {
        ArrayList<IDispTunerDaemon.IoEventPacket> result = new ArrayList<>();
        String[] lines = source.split("\n");
        for (String line : lines) {
            line = line.replace(" ", "");
            if (line.isEmpty()) {
                continue;
            }
            int index = Math.max(line.indexOf('['), line.indexOf("null"));
            String part1 = line.substring(0, index);
            String[] items = part1.split(",");
            IDispTunerDaemon.IoEventPacket packet = new IDispTunerDaemon.IoEventPacket();
            packet.sequence = Integer.parseInt(items[0]);
            packet.timestamp = Long.parseLong(items[1]);
            packet.fd = Integer.parseInt(items[2]);
            packet.opType = IDispTunerDaemon.IoEventPacket.IoOperationType.fromValue(Integer.parseInt(items[3]));
            packet.retValue = Long.parseLong(items[4]);
            packet.directArg1 = Long.parseLong(items[5]);
            packet.directArg2 = Long.parseLong(items[6]);
            String[] bufferStrArray = line.substring(index).replace("[", "").replace("]", "").split(",");
            if ("null".equals(bufferStrArray[0])) {
                packet.buffer = null;
            } else {
                byte[] buffer = new byte[bufferStrArray.length];
                for (int i = 0; i < bufferStrArray.length; i++) {
                    buffer[i] = Byte.parseByte(bufferStrArray[i]);
                }
                packet.buffer = buffer;
            }
            result.add(packet);
        }
        return result;
    }

    /**
     * Add a raw event to the list end. This event sequence must exactly match the last event sequence in the list.
     * That is @{code eventPacket.sequence == mLastRawEventSequence + 1}.
     *
     * @param eventPacket the event packet to add
     * @return true if there is a transaction event generated from this raw event
     */
    public int pushBackRawIoEvent(@NonNull IDispTunerDaemon.IoEventPacket eventPacket) {
        mRawIoEventPackets.put(eventPacket.sequence, eventPacket);
        return translateRawIoEvents();
    }

    /**
     * Add a raw event to the list front. This event sequence must exactly match the last event sequence in the list.
     * That is @{code eventPacket.sequence == mLastRawEventSequence + 1}.
     *
     * @param eventPackets the event packets to add
     * @return true if there are any transaction events generated from these raw events
     */
    public int pushBackRawIoEvents(@NonNull List<IDispTunerDaemon.IoEventPacket> eventPackets) {
        for (IDispTunerDaemon.IoEventPacket eventPacket : eventPackets) {
            mRawIoEventPackets.put(eventPacket.sequence, eventPacket);
            if (eventPacket.sequence > mLastRawEventSequence) {
                mLastRawEventSequence = eventPacket.sequence;
            }
        }
        return translateRawIoEvents();
    }

    public int getLastRawEventSequence() {
        return mLastRawEventSequence;
    }

    public int getLastSuccessTransactionSequence() {
        return mLastSuccessTransactionSequence;
    }

    /**
     * Reset the decoder state machine.
     * Note that this method dose not clear the event list.
     */
    public void resetDecoders() {
        mNxpHalIoEventHandler.reset();
    }

    /**
     * Clear all the events and reset the decoder state machine.
     */
    public void clearTransactionEvents() {
        mNxpHalIoEventHandler.reset();
        mLastRawEventSequence = 0;
        mLastUpdateTransactionSequence = 0;
        mLastSuccessTransactionSequence = 0;
        mRawIoEventPackets.clear();
        mTransactionEvents.clear();
        mAuxIoEvents.clear();
    }
}
