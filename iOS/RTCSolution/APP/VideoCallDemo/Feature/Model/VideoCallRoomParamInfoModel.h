// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, VideoCallRoomParamNetQuality) {
    VideoCallRoomParamNetQualityGood,
    VideoCallRoomParamNetQualityNormal,
    VideoCallRoomParamNetQualityBad
};

@interface VideoCallRoomParamInfoModel : NSObject

@property (nonatomic, strong) NSString *uid;
@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) NSInteger width;
@property (nonatomic, assign) NSInteger height;
@property (nonatomic, assign) NSInteger bitRate;
@property (nonatomic, assign) NSInteger fps;
@property (nonatomic, assign) NSInteger delay;
@property (nonatomic, assign) float lost;
@property (nonatomic, assign) VideoCallRoomParamNetQuality netQuality;

@end

NS_ASSUME_NONNULL_END
