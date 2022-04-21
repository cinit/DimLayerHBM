package cc.ioctl.opdisplaytuner.activity.ui.misc

import android.annotation.SuppressLint
import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import cc.ioctl.opdisplaytuner.activity.base.BaseBottomSheetDialogFragment
import cc.ioctl.opdisplaytuner.databinding.DialogLicenseNoticeBinding
import cc.ioctl.opdisplaytuner.databinding.ItemLicenseNoticeBinding
import io.noties.markwon.Markwon

class LicenseNoticeDialogFragment : BaseBottomSheetDialogFragment() {

    private var binding: DialogLicenseNoticeBinding? = null

    override fun createRootView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = DialogLicenseNoticeBinding.inflate(inflater, container, false)
        binding!!.dialogButtonClose.setOnClickListener { dismiss() }
        val recycleView = binding!!.dialogRecyclerViewNotices
        recycleView.adapter = LicenseNoticesAdapter(requireContext())
        return binding!!.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }

    class LicenseNoticeViewHolder(val binding: ItemLicenseNoticeBinding) :
        RecyclerView.ViewHolder(binding.root) {
    }

    class LicenseNoticesAdapter(val context: Context) :
        RecyclerView.Adapter<LicenseNoticeViewHolder>() {
        private val markwon: Markwon = Markwon.create(context)
        private val notices: ArrayList<LicenseNotice> = arrayListOf(
            LicenseNotice(
                "MMKV", "https://github.com/Tencent/MMKV",
                "Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.",
                "BSD 3-Clause License"
            ),
            LicenseNotice(
                "xHook", "https://github.com/iqiyi/xHook",
                "Copyright (c) 2018-present, iQIYI, Inc. All rights reserved.",
                "Most source code in xhook are MIT licensed. " +
                        "Some other source code have BSD-style licenses."
            ),
            LicenseNotice(
                "libsu",
                "https://github.com/topjohnwu/libsu",
                "Copyright 2021 John \"topjohnwu\" Wu",
                "Apache License 2.0"
            ),
            LicenseNotice(
                "AndroidHiddenApiBypass",
                "https://github.com/LSPosed/AndroidHiddenApiBypass",
                "Copyright (C) 2021 LSPosed",
                "Apache License 2.0"
            ),
            LicenseNotice(
                "Xposed",
                "https://github.com/rovo89/XposedBridge",
                "Copyright 2013 rovo89, Tungstwenty",
                "Apache License 2.0"
            ),
            LicenseNotice(
                "Markwon",
                "https://github.com/noties/Markwon",
                "Copyright 2017 Dimitry Ivanov (mail@dimitryivanov.ru)",
                "Apache License 2.0"
            )
        )

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int) =
            LicenseNoticeViewHolder(
                ItemLicenseNoticeBinding.inflate(
                    LayoutInflater.from(parent.context),
                    parent,
                    false
                )
            )

        @SuppressLint("SetTextI18n")
        override fun onBindViewHolder(holder: LicenseNoticeViewHolder, position: Int) {
            val notice = notices[position]
            val title: TextView = holder.binding.sLicenseItemTitle
            val licenseView: TextView = holder.binding.sLicenseItemLicensePrev
            markwon.setMarkdown(title, "- " + notice.name + "  \n<" + notice.url + ">")
            licenseView.text = notice.copyright + "\n" + notice.license
        }

        override fun getItemCount() = notices.size
    }

    data class LicenseNotice(
        val name: String,
        val url: String,
        val copyright: String,
        val license: String
    )
}
