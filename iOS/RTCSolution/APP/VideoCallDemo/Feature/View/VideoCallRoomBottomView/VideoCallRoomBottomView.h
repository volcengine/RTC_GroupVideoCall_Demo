// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>
#import "VideoCallRoomItemButton.h"
@class VideoCallRoomBottomView;

typedef NS_ENUM(NSInteger, RoomBottomStatus) {
    RoomBottomStatusMic = 0,
    RoomBottomStatusCamera,
    RoomBottomStatusAudio,
    RoomBottomStatusParameter,
    RoomBottomStatusSet,
};

@protocol VideoCallRoomBottomViewDelegate <NSObject>

- (void)VideoCallRoomBottomView:(VideoCallRoomBottomView *_Nonnull)VideoCallRoomBottomView itemButton:(VideoCallRoomItemButton *_Nullable)itemButton didSelectStatus:(RoomBottomStatus)status;

@end

NS_ASSUME_NONNULL_BEGIN

@interface VideoCallRoomBottomView : UIView

@property (nonatomic, weak) id<VideoCallRoomBottomViewDelegate> delegate;

- (void)updateButtonStatus:(RoomBottomStatus)status close:(BOOL)isClose;

- (ButtonStatus)getButtonStatus:(RoomBottomStatus)status;

- (void)updateButtonStatus:(RoomBottomStatus)status enable:(BOOL)isEnable;

@end

NS_ASSUME_NONNULL_END
