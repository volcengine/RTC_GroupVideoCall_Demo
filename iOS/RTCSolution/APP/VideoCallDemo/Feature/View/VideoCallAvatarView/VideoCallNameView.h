// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>
#import "VideoCallAvatarView.h"

NS_ASSUME_NONNULL_BEGIN


@interface VideoCallNameView : UIView

- (void)setMicStatus:(VideoCallAvatarViewMicStatus)status;

- (void)setName:(NSString *)name;

@end

NS_ASSUME_NONNULL_END
