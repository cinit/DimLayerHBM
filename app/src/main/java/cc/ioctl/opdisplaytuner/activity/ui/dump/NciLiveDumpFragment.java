package cc.ioctl.opdisplaytuner.activity.ui.dump;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;
import androidx.appcompat.app.AlertDialog;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.DefaultItemAnimator;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Objects;

import cc.ioctl.opdisplaytuner.R;
import cc.ioctl.opdisplaytuner.activity.MainUiFragmentActivity;
import cc.ioctl.opdisplaytuner.databinding.FragmentMainDumpBinding;
import cc.ioctl.opdisplaytuner.decoder.NxpHalV2EventTranslator;
import cc.ioctl.opdisplaytuner.ipc.daemon.IDispTunerDaemon;
import cc.ioctl.opdisplaytuner.ipc.daemon.IpcNativeHandler;
import cc.ioctl.opdisplaytuner.util.SafUtils;

public class NciLiveDumpFragment extends BaseHalDumpFragment implements Observer<ArrayList<NxpHalV2EventTranslator.TransactionEvent>> {

    private static final String STATE_CURRENT_DECODER_INDEX = "STATE_CURRENT_DECODER_INDEX";
    private FragmentMainDumpBinding mBinding;
    private NciLiveDumpViewModel mNciDumpViewModel;
    private final int mCurrentDecoderIndex = 0;
    private final AbsNciDumpAdapter mDumpAdapter = new AbsNciDumpAdapter() {
        @Override
        public void onBindViewHolder(@NonNull NciDumpViewHolder holder, int position) {
            if (mCurrentDecoderIndex == 0) {
                // raw aux IO events
                IDispTunerDaemon.IoEventPacket event = mNciDumpViewModel.getAuxIoEvents().getValue().get(position);
                updateListViewItemForAuxIoEvent(holder, event);
            } else {
                throw new IllegalStateException("Unknown decoder index: " + mCurrentDecoderIndex);
            }
        }

        @Override
        public int getItemCount() {
            if (mCurrentDecoderIndex == 0) {
                // raw aux IO events
                return mNciDumpViewModel.getAuxIoEvents().getValue().size();
            } else if (mCurrentDecoderIndex == 1) {
                // decoded NCI packets
                return mNciDumpViewModel.getTransactionEvents().getValue().size();
            } else {
                throw new IllegalStateException("Unknown decoder index: " + mCurrentDecoderIndex);
            }
        }

        @Override
        public int getEventSequenceByPosition(int position) {
            if (mCurrentDecoderIndex == 0) {
                // raw aux IO events
                return mNciDumpViewModel.getAuxIoEvents().getValue().get(position).sequence;
            } else if (mCurrentDecoderIndex == 1) {
                // decoded NCI packets
                return (int) mNciDumpViewModel.getTransactionEvents().getValue().get(position).sequence;
            } else {
                throw new IllegalStateException("Unknown decoder index: " + mCurrentDecoderIndex);
            }
        }

        @Override
        public int findEventPositionBySequence(int sequence) {
            if (mCurrentDecoderIndex == 0) {
                // raw aux IO events
                return BaseHalDumpFragment.findItemIndexBySequenceIoEventPacket(mNciDumpViewModel.getAuxIoEvents().getValue(), sequence);
            } else if (mCurrentDecoderIndex == 1) {
                // decoded NCI packets
                return BaseHalDumpFragment.findItemIndexBySequenceTransactionEvent(mNciDumpViewModel.getTransactionEvents().getValue(), sequence);
            } else {
                throw new IllegalStateException("Unknown decoder index: " + mCurrentDecoderIndex);
            }
        }
    };

    @SuppressLint("NotifyDataSetChanged")
    @Override
    public void onChanged(ArrayList<NxpHalV2EventTranslator.TransactionEvent> transactionEvents) {
//        int delta = transactionEvents.size() - mLastNciTransactionCount;
//        if (delta > 0) {
//            mDumpAdapter.notifyItemRangeInserted(mLastNciTransactionCount, transactionEvents.size() - mLastNciTransactionCount);
//        } else {
//            // remove items, should be empty now
        // I have no idea why this is would crash the app
        // Inconsistency detected. Invalid view holder adapter positionNciDumpViewHolder
        mDumpAdapter.notifyDataSetChanged();
//        }
//        mLastNciTransactionCount = transactionEvents.size();
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        Context context = inflater.getContext();
        mNciDumpViewModel = new ViewModelProvider(this).get(NciLiveDumpViewModel.class);
        mBinding = FragmentMainDumpBinding.inflate(inflater, container, false);
        mNciDumpViewModel.getTransactionEvents().observe(getViewLifecycleOwner(), this);
        LinearLayoutManager layoutManager = new LinearLayoutManager(context);
        RecyclerView recyclerView = mBinding.recyclerViewMainFragmentDumpList;
        recyclerView.setLayoutManager(layoutManager);
        layoutManager.setOrientation(RecyclerView.VERTICAL);
        recyclerView.addItemDecoration(new DividerItemDecoration(context, DividerItemDecoration.VERTICAL));
        recyclerView.setItemAnimator(new DefaultItemAnimator());
        mBinding.recyclerViewMainFragmentDumpList.setAdapter(mDumpAdapter);
        return mBinding.getRoot();
    }

    @Override
    public void onResume() {
        super.onResume();
        Activity activity = getActivity();
        if (activity instanceof MainUiFragmentActivity) {
            FloatingActionButton fab = ((MainUiFragmentActivity) activity).showFloatingActionButton();
        }
        IDispTunerDaemon daemon = IpcNativeHandler.peekConnection();
        if (daemon == null) {
            Snackbar.make(mBinding.getRoot(), R.string.ui_toast_daemon_is_not_running, Snackbar.LENGTH_LONG)
                    .setAction(R.string.ui_snackbar_action_view_or_jump, v -> jumpToHomeFragment()).show();
        } else {
            if (!daemon.isDisplayFeatureHidlServivceAttached()) {
                Snackbar.make(mBinding.getRoot(), R.string.ui_toast_hal_service_not_attached, Snackbar.LENGTH_LONG)
                        .setAction(R.string.ui_snackbar_action_view_or_jump, v -> jumpToHomeFragment()).show();
            }
        }
    }

    @UiThread
    public void jumpToHomeFragment() {
        MainUiFragmentActivity activity = (MainUiFragmentActivity) requireActivity();
        activity.getNavController().navigate(R.id.nav_main_home);
    }

    @Override
    public void onCreateOptionsMenu(@NonNull Menu menu, @NonNull MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        requireActivity().getMenuInflater().inflate(R.menu.menu_main_fragment_dump_live, menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_clear: {
                mNciDumpViewModel.clearTransactionEvents();
                return true;
            }
            case R.id.action_save: {
                if (Objects.requireNonNull(mNciDumpViewModel.getTransactionEvents().getValue()).isEmpty()) {
                    Snackbar.make(mBinding.getRoot(), R.string.ui_toast_no_data_to_save, Snackbar.LENGTH_SHORT).show();
                    return true;
                }
                String shortFileName = "nfc_hal_dump_" + System.currentTimeMillis() + ".haldump";
                SafUtils.requestSaveFile(requireActivity()).setDefaultFileName(shortFileName).onResult(uri -> {
                    try {
                        StringBuilder data = mNciDumpViewModel.getTranslator().exportRawEventsAsCsv();
                        OutputStream os = requireContext().getContentResolver().openOutputStream(uri);
                        os.write(data.toString().getBytes(StandardCharsets.UTF_8));
                        os.flush();
                        os.close();
                        Snackbar.make(mBinding.getRoot(), R.string.ui_toast_data_saved, Snackbar.LENGTH_SHORT).show();
                    } catch (IOException e) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(requireContext())
                                .setTitle(R.string.ui_dialog_error_title)
                                .setMessage(e.getMessage())
                                .setPositiveButton(android.R.string.ok, null);
                        builder.create().show();
                    }
                }).onCancel(() -> Toast.makeText(requireContext(), R.string.ui_toast_operation_canceled, Toast.LENGTH_SHORT).show()).commit();
                return true;
            }
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
