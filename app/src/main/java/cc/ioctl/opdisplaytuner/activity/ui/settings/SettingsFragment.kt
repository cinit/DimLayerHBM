package cc.ioctl.opdisplaytuner.activity.ui.settings

import android.app.Activity
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.Preference
import cc.ioctl.opdisplaytuner.R
import cc.ioctl.opdisplaytuner.activity.MainUiFragmentActivity
import cc.ioctl.opdisplaytuner.activity.base.BasePreferenceFragment
import cc.ioctl.opdisplaytuner.activity.ui.misc.LicenseNoticeDialogFragment

class SettingsFragment : BasePreferenceFragment() {

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val activity: Activity? = activity
        if (activity is AppCompatActivity) {
            val actionBar = activity.supportActionBar
            actionBar?.setDisplayHomeAsUpEnabled(true)
        }
        return super.onCreateView(inflater, container, savedInstanceState)
    }

    override fun onResume() {
        super.onResume()
        val activity: Activity? = activity
        if (activity is MainUiFragmentActivity) {
            activity.hideFloatingActionButton()
        }
    }

    public override fun doOnCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        setPreferencesFromResource(R.xml.root_preferences, rootKey)
        preferenceScreen.findPreference<Preference>("pref_open_source_licenses")!!
            .setOnPreferenceClickListener {
                val dialog = LicenseNoticeDialogFragment()
                dialog.show(requireActivity().supportFragmentManager, tag)
                true
            }
    }
}
