// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.graphics.Color;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatDialog;

import com.volcengine.vertcdemo.common.WindowUtils;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.databinding.DialogSubSettingBinding;
import com.wx.wheelview.adapter.ArrayWheelAdapter;
import com.wx.wheelview.widget.WheelView;

import java.util.ArrayList;
import java.util.List;

/**
 * 子设置对话框
 *
 * 功能：
 * 1.展示列表类型的选项 SubSettingDialog#setData(String, List, String, SubSettingDialog.OnSelectedCallback)
 *
 * 说明：
 * 1.设置好默认选择的选项后，会延迟500ms滑动。原因看com.wx.wheelview.widget.WheelView#setSelection(int)
 */
@SuppressWarnings("unchecked")
public class SubSettingDialog extends AppCompatDialog {

    private final DialogSubSettingBinding mViewBinding;
    private OnSelectedCallback mOnSelectedCallback;

    private int mSelectPosition;
    private String mSelectValue;

    public SubSettingDialog(Context context) {
        super(context, R.style.SolutionCommonDialog);
        setCancelable(true);

        mViewBinding = DialogSubSettingBinding.inflate(getLayoutInflater());

        mViewBinding.getRoot().setOnClickListener((v) -> dismiss());
        mViewBinding.subSettingContent.setOnClickListener((v) -> {});
        mViewBinding.subSettingCancel.setOnClickListener((v) -> dismiss());
        mViewBinding.subSettingConfirm.setOnClickListener(v -> {
            if (mOnSelectedCallback != null) {
                mOnSelectedCallback.onSelected(mSelectPosition, mSelectValue);
            }
            dismiss();
        });
        WheelView.WheelViewStyle style = new WheelView.WheelViewStyle();
        style.backgroundColor = Color.parseColor("#272E3B");
        style.selectedTextColor = Color.parseColor("#E5E6EB");
        style.textColor = Color.parseColor("#86909C");
        mViewBinding.subSettingWheelView.setStyle(style);
    }

    /**
     * 设置展示内容和回调
     *
     * @param title       对话框标题
     * @param data        列表数据
     * @param selectValue 默认选中的数据
     * @param callback    选中回调
     */
    public void setData(String title, List<String> data, String selectValue,
                        OnSelectedCallback callback) {
        mSelectValue = selectValue;
        mOnSelectedCallback = callback;

        mViewBinding.subSettingTitle.setText(title);
        if (data == null) {
            data = new ArrayList<>();
        }
        mViewBinding.subSettingWheelView.setWheelAdapter(new ArrayWheelAdapter(getContext()));
        mViewBinding.subSettingWheelView.setWheelData(data);
        int index = 0;
        for (int i = 0; i < data.size(); i++) {
            if (TextUtils.equals(selectValue, data.get(i))) {
                index = i;
                break;
            }
        }
        mViewBinding.subSettingWheelView.setSelection(index);
        mViewBinding.subSettingWheelView.setOnWheelItemSelectedListener((position, s) -> {
            mSelectPosition = position;
            mSelectValue = s.toString();
        });
    }

    @Override
    public void show() {
        super.show();

        WindowManager.LayoutParams params = getWindow().getAttributes();
        params.width = WindowUtils.getScreenWidth(getContext());
        params.height = ViewGroup.LayoutParams.MATCH_PARENT;
        getWindow().setAttributes(params);
        getWindow().setContentView(mViewBinding.getRoot(), new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        getWindow().setGravity(Gravity.BOTTOM);
    }

    public interface OnSelectedCallback {
        void onSelected(int position, String str);
    }
}
