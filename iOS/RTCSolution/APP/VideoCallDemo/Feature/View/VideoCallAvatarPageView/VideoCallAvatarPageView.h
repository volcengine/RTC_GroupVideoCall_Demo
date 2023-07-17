// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@protocol VideoCallAvatarPageViewDelegate <NSObject>

/**
 * 多页时翻页停止后呈现的对应view以及index
 */
- (void)onShowAvatarView:(UIView *)avatarView index:(NSUInteger)index;

/**
 * 多页时翻页停止后隐藏的对应view以及index
 */
- (void)onHideAvatarView:(UIView *)avatarView index:(NSUInteger)index;

/**
 * 点击的对应view以及index
 */
- (void)onClickAvatarView:(UIView *)avatarView index:(NSUInteger)index;


/**
 * 多页时翻页停止后对应的pageIndex
 */
- (void)onScrollToPageIndex:(NSUInteger)pageIndex;

@end


@interface VideoCallAvatarPageView : UIView

@property (nonatomic, weak) id <VideoCallAvatarPageViewDelegate> avatarPageViewDelegate;

@property (nonatomic, readonly) NSUInteger currentPageIndex;

@property (nonatomic, readonly) NSUInteger currentPageCount;

@property (nonatomic, readonly) NSUInteger avatarViewCount;

@property (nonatomic, readonly) NSUInteger mainViewIndex;

/**
 * @brief 不要通过init初始化,用initWithFrame:
 */
- (instancetype)init NS_UNAVAILABLE;

/**
 * @brief 不要通过initWithCoder初始化,用initWithFrame:
 */
- (instancetype)initWithCoder:(NSCoder *)coder NS_UNAVAILABLE;

/**
 * @brief 初始化时,指定正确的frame
 */
- (instancetype)initWithFrame:(CGRect)frame NS_DESIGNATED_INITIALIZER;

/**
 * @brief 添加视频窗口
 */
- (void)addAvatarView:(UIView *)avatarView;

/**
 * @brief 根据index添加视频窗口
 */
- (void)addAvatarView:(UIView *)avatarView atIndex:(NSUInteger)index;

/**
 * @brief 删除视频窗口
 */
- (void)removeAvatarView:(UIView *)avatarView;

/**
 * @brief 根据index删除视频窗口
 */
- (void)removeAvatarViewAtIndex:(NSUInteger)index;

/**
 * @brief 获取对应index的视频窗口
 */
- (nullable UIView *)avatarViewAtIndex:(NSUInteger)index;

/**
 * @brief 将对应index的视频窗口设为主窗口（只有两个视频窗口时生效）
 */
- (void)bringViewToMainAvatarViewOfIndex:(NSUInteger)index;


@end

NS_ASSUME_NONNULL_END
