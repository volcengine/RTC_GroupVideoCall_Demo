// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>
#import "VideoCallRoomParamInfoModel.h"

NS_ASSUME_NONNULL_BEGIN

@interface VideoCallStatsView : UIView

- (void)setVideoStats:(NSArray <VideoCallRoomParamInfoModel *>*)videoStats;

- (void)setAudioStats:(NSArray <VideoCallRoomParamInfoModel *>*)audioStats;

- (void)showStatsView;

@end

NS_ASSUME_NONNULL_END
