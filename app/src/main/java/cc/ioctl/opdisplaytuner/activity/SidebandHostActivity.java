package cc.ioctl.opdisplaytuner.activity;

import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;

import cc.ioctl.opdisplaytuner.R;
import cc.ioctl.opdisplaytuner.activity.base.BaseActivity;

/**
 * Fragment host activity for the secondary fragments, eg. saved dump viewer.
 */
public class SidebandHostActivity extends BaseActivity {

    @Override
    protected boolean doOnCreate(@Nullable Bundle savedInstanceState) {
        super.doOnCreate(savedInstanceState);
        setContentView(R.layout.activity_sideband_host);
        if (savedInstanceState == null) {
            Intent intent = getIntent();
            String action = intent.getAction();
            // TODO: 2022-04-10 handle action...
        }
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        return true;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.menu_activity_main_ui_common, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.action_exit) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
