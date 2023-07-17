// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.databinding.ItemSettingLayoutBinding;

/**
 * 设置对话框单条设置控件
 *
 * 功能：
 * 1.设置字符串型数值
 * 2.设置开关型数值
 */
public class SettingItemLayout extends FrameLayout {

    private ItemSettingLayoutBinding mViewBinding;

    public SettingItemLayout(@NonNull Context context) {
        super(context);
        initView();
    }

    public SettingItemLayout(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public SettingItemLayout(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private void initView() {
        View view = View.inflate(getContext(), R.layout.item_setting_layout, this);
        mViewBinding = ItemSettingLayoutBinding.bind(view);
    }

    /**
     * 设置字符串类型数值
     *
     * @param key   设置名
     * @param value 设置值
     */
    public void setData(String key, String value) {
        mViewBinding.settingKey.setText(key);
        mViewBinding.settingTextValue.setText(value);

        mViewBinding.settingTextValueLayout.setVisibility(VISIBLE);
        mViewBinding.settingSwitchValue.setVisibility(GONE);
    }

    /**
     * 设置开关类型数值
     *
     * @param key   设置名
     * @param value 设置值
     */
    public void setData(String key, boolean value) {
        mViewBinding.settingKey.setText(key);
        mViewBinding.settingSwitchValue.setChecked(value);

        mViewBinding.settingTextValueLayout.setVisibility(GONE);
        mViewBinding.settingSwitchValue.setVisibility(VISIBLE);
    }

    /**
     * 设置开关状态变化回调
     *
     * @param listener 回调
     */
    public void setOnCheckListener(CompoundButton.OnCheckedChangeListener listener) {
        mViewBinding.settingSwitchValue.setOnCheckedChangeListener(listener);
    }
}
