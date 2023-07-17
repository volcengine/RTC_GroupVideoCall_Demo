// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallAvatarView.h"
#import "VideoCallNameView.h"

@interface VideoCallAvatarView ()

@property (nonatomic, assign) VideoCallAvatarViewMicStatus currentMicStatus;

@property (nonatomic, strong) UILabel *centerNameLabel;

@property (nonatomic, strong) UIImageView *volumeImageView;

@property (nonatomic, strong) VideoCallNameView *nameTagView;

@end

@implementation VideoCallAvatarView

- (instancetype)init {
    self = [super init];
    if (self) {
        [self addSubview:self.volumeImageView];
        [self.volumeImageView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(0, 0));
            make.center.equalTo(self);
        }];
        
        self.centerNameLabel = [UILabel new];
        self.centerNameLabel.backgroundColor = [UIColor colorFromHexString:@"#3E4045"];
        self.centerNameLabel.textAlignment = NSTextAlignmentCenter;
        self.centerNameLabel.font = [UIFont boldSystemFontOfSize:17.f];
        self.centerNameLabel.textColor = [UIColor whiteColor];
        self.centerNameLabel.layer.masksToBounds = YES;
        [self addSubview:self.centerNameLabel];
        [self.centerNameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.center.equalTo(self);
            make.size.mas_equalTo(CGSizeMake(0, 0));
        }];

        self.videoContainerView = [UIView new];
        [self addSubview:self.videoContainerView];
        [self.videoContainerView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.edges.equalTo(self);
        }];
        
        self.nameTagView = [VideoCallNameView new];
        [self addSubview:self.nameTagView];
        [self.nameTagView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self).offset(4.f);
            make.bottom.equalTo(self).offset(-4.f);
            make.right.lessThanOrEqualTo(self).offset(-4.f);
        }];
    }
    return self;
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    CGFloat labelWidth = 0;
    CGFloat volumeWidth = 0;
    if (self.frame.size.width >= SCREEN_WIDTH) {
        // One full screen style
        labelWidth = 80;
        volumeWidth = 111;
        self.backgroundColor = [UIColor colorFromHexString:@"#1D2129"];
    } else if (self.frame.size.width > 90) {
        // UI style for more than 3 people
        labelWidth = 80;
        volumeWidth = 111;
        self.backgroundColor = [UIColor colorFromHexString:@"#282B30"];
    } else {
        // Top right UI
        labelWidth = 42;
        volumeWidth = 58;
        self.backgroundColor = [UIColor colorFromHexString:@"#282B30"];
    }
    
    [self.volumeImageView mas_updateConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(volumeWidth, volumeWidth));
    }];
    
    [self.centerNameLabel mas_updateConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(labelWidth, labelWidth));
    }];
    self.centerNameLabel.layer.cornerRadius = labelWidth / 2;
}

- (void)setVideoStatus:(VideoCallAvatarViewVideoStatus)status {
    self.videoContainerView.hidden = status == VideoCallAvatarViewVideoStatusOff;
    self.centerNameLabel.hidden = !self.videoContainerView.hidden;
}

- (void)setMicStatus:(VideoCallAvatarViewMicStatus)status {
    self.currentMicStatus = status;
    if (status == VideoCallAvatarViewMicStatusSpeaking) {
        self.volumeImageView.hidden = NO;
    } else {
        self.volumeImageView.hidden = YES;
    }
    
    [self.nameTagView setMicStatus:status];
}

- (void)setName:(NSString *)name {
    if (name.length >= 1) {
        NSString *firstLetter = [name substringToIndex:1];
        self.centerNameLabel.text = firstLetter;
    }else {
        self.centerNameLabel.text = nil;
    }
    
    [self.nameTagView setName:name];
}

#pragma mark - Getter

- (UIImageView *)volumeImageView {
    if (!_volumeImageView) {
        _volumeImageView = [[UIImageView alloc] init];
        _volumeImageView.image = [UIImage imageNamed:@"volume_icon" bundleName:HomeBundleName];
        _volumeImageView.hidden = YES;
    }
    return _volumeImageView;
}

@end
