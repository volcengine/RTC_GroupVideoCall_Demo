// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.ss.bytertc.engine.data.StreamIndex;

/**
 * 音量大小事件，用于更新UI
 */
public class AudioPropertiesReportEvent {

    public StreamIndex streamIndex;
    public String uid;
    public boolean isSpeaking;

    public AudioPropertiesReportEvent(StreamIndex streamIndex, String uid, boolean isSpeaking) {
        this.streamIndex = streamIndex;
        this.uid = uid;
        this.isSpeaking = isSpeaking;
    }
}
