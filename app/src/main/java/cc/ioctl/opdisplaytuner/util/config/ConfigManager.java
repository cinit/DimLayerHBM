package cc.ioctl.opdisplaytuner.util.config;

import android.content.SharedPreferences;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;
import java.io.IOException;
import java.util.Map;

public abstract class ConfigManager implements SharedPreferences, SharedPreferences.Editor {

    private static ConfigManager sDefaultConfig;
    private static ConfigManager sCache;

    protected ConfigManager() {
    }

    @NonNull
    public static synchronized ConfigManager getDefaultConfig() {
        if (sDefaultConfig == null) {
            sDefaultConfig = new MmkvConfigManagerImpl("default_config");
        }
        return sDefaultConfig;
    }

    @NonNull
    public static synchronized ConfigManager getCache() {
        if (sCache == null) {
            sCache = new MmkvConfigManagerImpl("global_cache");
        }
        return sCache;
    }

    public abstract void reinit() throws IOException;

    @Nullable
    public abstract File getFile();

    @Nullable
    public Object getOrDefault(@NonNull String key, @Nullable Object def) {
        if (!containsKey(key)) {
            return def;
        }
        return getObject(key);
    }

    public boolean getBooleanOrFalse(@NonNull String key) {
        return getBooleanOrDefault(key, false);
    }

    public boolean getBooleanOrDefault(@NonNull String key, boolean def) {
        return getBoolean(key, def);
    }

    public int getIntOrDefault(@NonNull String key, int def) {
        return getInt(key, def);
    }

    @Nullable
    public abstract String getString(@NonNull String key);

    @NonNull
    public String getStringOrDefault(@NonNull String key, @NonNull String defVal) {
        return getString(key, defVal);
    }

    @Nullable
    public abstract Object getObject(@NonNull String key);

    @Nullable
    public byte[] getBytes(@NonNull String key) {
        return getBytes(key, null);
    }

    @Nullable
    public abstract byte[] getBytes(@NonNull String key, @Nullable byte[] defValue);

    @NonNull
    public abstract byte[] getBytesOrDefault(@NonNull String key, @NonNull byte[] defValue);

    @NonNull
    public abstract ConfigManager putBytes(@NonNull String key, @NonNull byte[] value);

    /**
     * @return READ-ONLY all config
     * @deprecated Avoid use getAll(), MMKV only have limited support for this.
     */
    @Override
    @Deprecated
    @NonNull
    public abstract Map<String, ?> getAll();

    public abstract void reload() throws IOException;

    public abstract void save() throws IOException;

    public abstract void saveAndNotify(int what) throws IOException;

    public abstract void saveWithoutNotify() throws IOException;

    public long getLongOrDefault(@Nullable String key, long i) {
        return getLong(key, i);
    }

    @NonNull
    public abstract ConfigManager putObject(@NonNull String key, @NonNull Object v);

    public boolean containsKey(@NonNull String k) {
        return contains(k);
    }

    @NonNull
    @Override
    public Editor edit() {
        return this;
    }

    @Override
    public void registerOnSharedPreferenceChangeListener(
            @NonNull OnSharedPreferenceChangeListener listener) {
        throw new UnsupportedOperationException("not implemented");
    }

    @Override
    public void unregisterOnSharedPreferenceChangeListener(
            @NonNull OnSharedPreferenceChangeListener listener) {
        throw new UnsupportedOperationException("not implemented");
    }

    public abstract boolean isReadOnly();

    public abstract boolean isPersistent();
}
