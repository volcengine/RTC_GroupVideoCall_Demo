// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.ss.bytertc.engine.type.LocalStreamStats;

/**
 * 本地音视频流上报事件，用于UI展示
 */
public class LocalStreamStatsEvent {

    public final LocalStreamStats localStreamStats;

    public LocalStreamStatsEvent(LocalStreamStats localStreamStats) {
        this.localStreamStats = localStreamStats;
    }
}
