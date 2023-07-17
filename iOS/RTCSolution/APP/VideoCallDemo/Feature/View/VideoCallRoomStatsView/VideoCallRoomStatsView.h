// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface VideoCallRoomStatsView : UIView

@property (nonatomic, readonly) UITableView *statsTableView;
@property (nonatomic, readonly) NSUInteger currentSelectedIndex;

@end

NS_ASSUME_NONNULL_END
