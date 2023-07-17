// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.ss.bytertc.engine.type.RemoteStreamStats;

/**
 * 远端音视频流上报事件，用于UI展示
 */
public class RemoteStreamStatsEvent {

    public final RemoteStreamStats remoteStreamStats;

    public RemoteStreamStatsEvent(RemoteStreamStats remoteStreamStats) {
        this.remoteStreamStats = remoteStreamStats;
    }
}
