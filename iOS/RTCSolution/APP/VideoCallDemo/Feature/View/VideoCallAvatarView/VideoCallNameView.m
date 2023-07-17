// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallNameView.h"

@interface VideoCallNameView ()

@property (nonatomic, strong) UIImageView *micIcon;

@property (nonatomic, strong) UILabel *nameLabel;

@end

@implementation VideoCallNameView

- (instancetype)init {
    self = [super init];
    if (self) {
        self.backgroundColor = [UIColor colorWithWhite:0 alpha:0.4];
        self.layer.cornerRadius = 11.f;
        
        self.micIcon = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"room_mic" bundleName:HomeBundleName]];
        [self addSubview:self.micIcon];
        [self.micIcon mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self).offset(6.f);
            make.centerY.equalTo(self);
            make.width.height.equalTo(@(12.f));
        }];
        
        self.nameLabel = [UILabel new];
        self.nameLabel.font = [UIFont systemFontOfSize:11];
        self.nameLabel.lineBreakMode = NSLineBreakByTruncatingMiddle;
        self.nameLabel.textColor = [UIColor whiteColor];
        [self addSubview:self.nameLabel];
        [self.nameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self.micIcon.mas_right).offset(2.f);
            make.right.equalTo(self).offset(-8.f);
            make.centerY.equalTo(self.micIcon);
        }];

        [self mas_makeConstraints:^(MASConstraintMaker *make) {
            make.height.equalTo(@(22.f));
        }];
    }
    return self;
}

- (void)setMicStatus:(VideoCallAvatarViewMicStatus)status {
    if (status == VideoCallAvatarViewMicStatusOff) {
        self.micIcon.image = [UIImage imageNamed:@"room_mic_s" bundleName:HomeBundleName];
    } else if (status == VideoCallAvatarViewMicStatusSpeaking) {
        self.micIcon.image = [UIImage imageNamed:@"par_mic_i" bundleName:HomeBundleName];
    } else {
        self.micIcon.image = [UIImage imageNamed:@"room_mic" bundleName:HomeBundleName];
    }
}

- (void)setName:(NSString *)name {
    self.nameLabel.text = name;
}


@end
