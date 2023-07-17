// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSInteger, VideoCallAvatarViewVideoStatus) {
    VideoCallAvatarViewVideoStatusOff,
    VideoCallAvatarViewVideoStatusOn
};

typedef NS_ENUM(NSInteger, VideoCallAvatarViewMicStatus) {
    VideoCallAvatarViewMicStatusOff,
    VideoCallAvatarViewMicStatusOn,
    VideoCallAvatarViewMicStatusSpeaking
};

NS_ASSUME_NONNULL_BEGIN

@interface VideoCallAvatarView : UIView

@property (nonatomic, strong) UIView *videoContainerView;

- (void)setVideoStatus:(VideoCallAvatarViewVideoStatus)status;

- (void)setMicStatus:(VideoCallAvatarViewMicStatus)status;

- (void)setName:(NSString *)name;

@end

NS_ASSUME_NONNULL_END
