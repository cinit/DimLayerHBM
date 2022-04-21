package cc.ioctl.opdisplaytuner.util;

import android.app.AlertDialog;
import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;


public class FaultyDialog {

    private FaultyDialog() {
        throw new AssertionError("No " + getClass().getName() + " instances for you!");
    }

    public static void show(@NonNull Context ctx, @NonNull Throwable e) {
        show(ctx, null, e, true);
    }

    public static void show(@NonNull Context ctx, @Nullable String title, @NonNull Throwable e) {
        show(ctx, title, e, true);
    }

    public static void show(@NonNull Context ctx, @Nullable String title, @NonNull Throwable e, boolean cancelable) {
        String t = TextUtils.isEmpty(title) ? e.getClass().getSimpleName() : title;
        assert t != null;
        ThreadManager.runOnUiThread(() -> showImpl(ctx, t, e, cancelable));
    }

    public static void show(@NonNull Context ctx, @NonNull String title, @NonNull String msg) {
        show(ctx, title, msg, false);
    }

    public static void show(@NonNull Context ctx, @NonNull String title, @NonNull String msg, boolean cancelable) {
        ThreadManager.runOnUiThread(() -> showImpl(ctx, title, msg, cancelable));
    }

    private static void showImpl(@NonNull Context ctx, @NonNull String title, @NonNull Throwable e, boolean cancelable) {
        new AlertDialog.Builder(ctx)
                .setTitle(title)
                .setMessage(Log.getStackTraceString(e))
                .setCancelable(cancelable)
                .setPositiveButton(android.R.string.ok, null)
                .show();
    }

    private static void showImpl(@NonNull Context ctx, @NonNull String title, @NonNull CharSequence msg, boolean cancelable) {
        new AlertDialog.Builder(ctx)
                .setTitle(title)
                .setMessage(msg)
                .setCancelable(cancelable)
                .setPositiveButton(android.R.string.ok, null)
                .show();
    }
}
