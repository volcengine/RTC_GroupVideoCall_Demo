// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface VideoCallFullScreenView : UIView

@property (nonatomic, copy) void (^clickOrientationBlock)(BOOL isLandscape);

- (void)show:(NSString *)uid
    userName:(NSString *)userName
      roomId:(NSString *)roomId
       block:(void (^)(BOOL isRemove))block;

- (void)dismiss:(BOOL)isRemove;

@end

NS_ASSUME_NONNULL_END
