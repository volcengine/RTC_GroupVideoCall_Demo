// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>
#import "VideoCallRoomUserModel.h"
@class VideoCallRoomNavView;

typedef NS_ENUM(NSInteger, RoomNavStatus) {
    RoomNavStatusSwitchCamera,
    RoomNavStatusHangeup
};

NS_ASSUME_NONNULL_BEGIN

@protocol VideoCallRoomNavViewDelegate <NSObject>

- (void)VideoCallRoomNavView:(VideoCallRoomNavView *)VideoCallRoomNavView didSelectStatus:(RoomNavStatus)status;

@end

@interface VideoCallRoomNavView : UIView

@property (nonatomic, strong) VideoCallRoomUserModel *localVideoSession;

@property (nonatomic, weak) id<VideoCallRoomNavViewDelegate> delegate;

@property (nonatomic, assign) NSInteger meetingTime;

@end

NS_ASSUME_NONNULL_END
