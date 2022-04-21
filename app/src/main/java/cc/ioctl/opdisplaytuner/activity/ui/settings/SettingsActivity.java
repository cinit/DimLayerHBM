package cc.ioctl.opdisplaytuner.activity.ui.settings;

import android.os.Bundle;

import androidx.appcompat.app.ActionBar;

import cc.ioctl.opdisplaytuner.R;
import cc.ioctl.opdisplaytuner.activity.base.BaseActivity;
import cc.ioctl.opdisplaytuner.activity.ui.settings.SettingsFragment;

public class SettingsActivity extends BaseActivity {

    private static final String TAG = "SettingsActivity";

    @Override
    protected boolean doOnCreate(Bundle savedInstanceState) {
        super.doOnCreate(savedInstanceState);
        setContentView(R.layout.activity_empty_fragment_container);
        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.fragment_container, new SettingsFragment())
                    .commit();
        }
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        return true;
    }
}
