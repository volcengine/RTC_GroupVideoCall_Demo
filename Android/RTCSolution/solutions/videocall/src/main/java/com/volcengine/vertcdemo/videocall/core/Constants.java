// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.core;

import android.util.Pair;

import androidx.annotation.IntDef;

import com.ss.bytertc.engine.type.AudioProfileType;
import com.volcengine.vertcdemo.utils.AppUtil;
import com.volcengine.vertcdemo.videocall.R;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.LinkedHashMap;

/**
 * 定义使用到的常量
 *
 * 包含：
 * 1.场景缩略名（用于向服务端请求数据，前后端统一）
 * 2.intent传递数据的key
 * 3.音量检测相关值
 * 4.分辨率文案和值映射
 * 5.音质文案和值映射
 * 6.默认分辨率和音质定义
 * 7.媒体相关常量定义
 */
public class Constants {

    // 场景名
    public static final String SOLUTION_NAME_ABBR = "videocall";

    public static final String EXTRA_ROOM_ID = "room_id";
    public static final String EXTRA_TOKEN = "token";
    public static final String EXTRA_LAST_TS = "last_ts";

    // 音量检测时，认为用户说话的音量阈值
    public static final int VOLUME_SPEAKING_THRESHOLD = 10;
    // 音量检测时间间隔
    public static final int VOLUME_SPEAKING_INTERVAL = 2000;

    // 分辨率文案与具体值的映射
    public static final LinkedHashMap<String, Pair<Integer, Integer>> RESOLUTION_MAP
            = new LinkedHashMap<String, Pair<Integer, Integer>>() {
        {
            put("720*1280", new Pair<>(720, 1280));
            put("540*960", new Pair<>(540, 960));
            put("360*640", new Pair<>(360, 640));
            put("180*320", new Pair<>(180, 320));
        }
    };

    // 音质文案与RTC值域映射
    public static final LinkedHashMap<String, AudioProfileType> QUALITY_MAP
            = new LinkedHashMap<String, AudioProfileType>() {
        {
            put(AppUtil.getApplicationContext().getString(R.string.clarity),
                    AudioProfileType.AUDIO_PROFILE_FLUENT);
            put(AppUtil.getApplicationContext().getString(R.string.high_definition),
                    AudioProfileType.AUDIO_PROFILE_STANDARD);
            put(AppUtil.getApplicationContext().getString(R.string.extreme),
                    AudioProfileType.AUDIO_PROFILE_HD);
        }
    };

    // 默认分辨率定义
    public static final String DEFAULT_RESOLUTION = "720*1280";
    // 默认音质定义
    public static final String DEFAULT_QUALITY = AppUtil.getApplicationContext()
            .getString(R.string.high_definition);

    // 媒体状态定义
    public static final int MEDIA_STATUS_ON = 1;
    public static final int MEDIA_STATUS_OFF = 0;

    @IntDef({MEDIA_STATUS_ON, MEDIA_STATUS_OFF})
    @Retention(RetentionPolicy.SOURCE)
    public @interface MediaStatus {
    }

    // 媒体类型定义
    public static final int MEDIA_TYPE_VIDEO = 0;
    public static final int MEDIA_TYPE_AUDIO = 1;

    @IntDef({MEDIA_TYPE_VIDEO, MEDIA_TYPE_AUDIO})
    @Retention(RetentionPolicy.SOURCE)
    public @interface MediaType {

    }
}
