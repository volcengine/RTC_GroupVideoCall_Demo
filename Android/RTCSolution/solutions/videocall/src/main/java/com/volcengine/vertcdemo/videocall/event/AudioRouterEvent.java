// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

/**
 * 音频路由切换事件，用于更新UI
 */
public class AudioRouterEvent {

    public final boolean isSpeakerPhone;

    public AudioRouterEvent(boolean isSpeakerPhone) {
        this.isSpeakerPhone = isSpeakerPhone;
    }
}
