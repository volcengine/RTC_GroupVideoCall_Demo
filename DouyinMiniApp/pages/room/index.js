const app = getApp();
const RtcClient = require('../../common/RtcClient/index');
const request = require('../../common/request/index');

let timer = null;

Page({
  data: {
    roomMembers: [],
    videoMembers: [],
    audioMembers: [],
    screenMembers: [],
    streamPages: [],
    roomId: '',
    userId: '',
    camera: 'front',
    micOn: app.micOn,
    cameraOn: app.cameraOn,
    sound: app.sound,
    curPage: 1,
    totalPages: 1,
    viewMode: 'stream',
  },

  async onLoad(options) {
    console.log('Room onload', options, app);

    if (app.cameraOn) {
      await RtcClient.startVideoCapture().catch((err) => {
        console.error('startVideoCapture', err);
        tt.showToast({ title: '请打开相机权限！', icon: 'fail' });
      });
    } else {
      await RtcClient.stopVideoCapture();
    }

    if (app.micOn) {
      await RtcClient.startAudioCapture().catch((err) => {
        console.error('startAudioCapture', err);
        tt.showToast({ title: '请打开麦克风权限！', icon: 'fail' });
      });

      await RtcClient.publishStream('audio');
    } else {
      await RtcClient.stopAudioCapture();
      await RtcClient.unPublishStream('audio');
    }

    tt.setNavigationBarTitle({
      title: `ID:${options.roomId}`,
    });

    this.setData(
      {
        micOn: app.micOn,
        cameraOn: app.cameraOn,
        sound: app.sound,
        userId: options.userId,
        roomId: options.roomId,
        token: decodeURIComponent(options.token),
      },
      () => {
        this.joinRtcRoom();
        this.addEventListener();
      }
    );

    timer = setTimeout(async () => {
      tt.showToast({ title: '体验已超过15分钟，房间解散', icon: 'none' });
      await this.exitRtcRoom(false);
      setTimeout(() => {
        tt.navigateBack();
      }, 1500);

      timer = null;
    }, (15 * 60 - app.duration) * 1000);
  },

  async onHide() {
    // await RtcClient.stopVideoCapture();
    // await RtcClient.stopAudioCapture();
  },

  onUnload: async function () {
    app.micOn = true;
    app.cameraOn = true;
    app.sound = 'speakerphone';
    app.duration = 0;

    await RtcClient.switchCamera('front');
    await RtcClient.leaveRoom(this.data.roomId);
    request.post('/common', {
      event_name: 'videocallLeaveRoom',
      device_id: this.data.userId,
      room_id: `call_${this.data.roomId}`,
      user_id: this.data.userId,
      app_id: app.appId,
      content: JSON.stringify({
        room_id: `call_${this.data.roomId}`,
        user_id: this.data.userId,
        login_token: app.login_token,
      }),
    });

    console.log('room unload');
  },

  onShow: function () {},

  addEventListener() {
    app.RtcClient.onRtcStateChanged(this.onRtcStateChanged);
    app.RtcClient.onRtcVideoMembersChanged(this.onVideoMembersChanged);
    app.RtcClient.onRtcChatMembersChanged(this.onRoomMembersChanged);
    app.RtcClient.onRtcChatSpeakersChanged(this.onSpeakersChanged);
    app.RtcClient.onRtcPublishScreenMembersChanged(this.onScreenUsersChanged);
  },

  removeEventListener() {
    app.RtcClient.offRtcStateChanged(this.onRtcStateChanged);
    app.RtcClient.offRtcVideoMembersChanged(this.onVideoMembersChanged);
    app.RtcClient.offRtcChatMembersChanged(this.onRoomMembersChanged);
    app.RtcClient.offRtcChatSpeakersChanged(this.onSpeakersChanged);
    app.RtcClient.offRtcPublishScreenMembersChanged(this.onScreenUsersChanged);
  },

  handleError(e) {
    console.log(' rtc-room component error ', e);
  },

  calcStreamPages(recheck = false) {
    const roomMembers = [...this.data.roomMembers];
    const videoMembers = [...this.data.videoMembers];
    const audioMembers = [...this.data.audioMembers];
    const screenMembers = [...this.data.screenMembers];

    let streams = roomMembers.map((userId) => {
      const self = this.data.userId === userId;
      return {
        userId,
        mode: self ? 'camera' : 'video',
        cameraOn: videoMembers.includes(userId),
        audioOn: audioMembers.includes(userId),
        isScreen: false,
        isMe: self,
      };
    });

    if (screenMembers.length > 0) {
      streams = [
        ...screenMembers.map((userId) => {
          return {
            userId,
            mode: 'screen',
            cameraOn: videoMembers.includes(userId),
            audioOn: audioMembers.includes(userId),
            isScreen: true,
            isMe: self,
          };
        }),
        ...streams,
      ];
    }

    let pages = [];

    while (streams.length) {
      let page = streams.splice(0, 4);
      pages.push(page);
    }

    if (recheck) {
      this.setData({ streamPages: [], totalPages: pages.length }, () => {
        this.setData({ streamPages: pages, totalPages: pages.length });

        if (screenMembers.length === 0) {
          this.setData({ viewMode: 'stream' });
        }
      });
    } else {
      this.setData({ streamPages: pages, totalPages: pages.length });
    }
  },

  onVideoMembersChanged({ userIdList, errCode, errMsg }) {
    console.log('onVideoMembersChanged', userIdList, errCode, errMsg);

    this.setData(
      {
        videoMembers: userIdList,
      },
      () => {
        this.calcStreamPages();
      }
    );
  },

  onRoomMembersChanged(params) {
    const { userIdList } = params;
    console.log(' onRoomMembersChanged ', params, userIdList);
    this.setData(
      {
        roomMembers: userIdList,
      },
      () => {
        this.calcStreamPages();
      }
    );
  },

  onSpeakersChanged({ userIdList, errCode, errMsg }) {
    console.log(' onSpeakersChanged ', userIdList, errCode, errMsg);
    this.setData(
      {
        audioMembers: userIdList,
      },
      () => {
        this.calcStreamPages();
      }
    );
  },

  onScreenUsersChanged({ userIdList, errCode, errMsg }) {
    console.log(' onScreenUsersChanged ', userIdList);
    this.setData(
      {
        screenMembers: userIdList,
      },
      () => {
        this.calcStreamPages(true);
      }
    );
  },

  async onRtcStateChanged({ data, errNo, errMsg }) {
    console.log(' onRtcStateChanged ', data, errNo, errMsg);
    if (errNo === 21108) {
      tt.showToast({ title: 'token过期,退出房间' });
      await this.exitRtcRoom(false);
      tt.navigateBack();
    }
    if (errNo === 21105) {
      tt.showToast({ title: '没有相机权限,退出房间' });
      await this.exitRtcRoom(false);
      tt.navigateBack();
    }
    if (errNo === 21106) {
      tt.showToast({ title: '没有麦克风权限,退出房间' });
      await this.exitRtcRoom(false);
      tt.navigateBack();
    }
  },

  async joinRtcRoom() {
    try {
      await RtcClient.joinRoom({
        roomId: `call_${this.data.roomId}`,
        token: this.data.token,
        userId: this.data.userId,
      });
      tt.showToast({ title: '进房成功' });
    } catch (err) {
      console.log(' joinRtcRoom fail ', err);
      tt.showToast({ title: '进房失败', icon: 'fail' });
    }
  },

  async handleVideoCapture() {
    console.log('handleVideoCapture', this.data.cameraOn);

    if (this.data.cameraOn) {
      await RtcClient.stopVideoCapture().then(() => {
        this.setData({
          cameraOn: !this.data.cameraOn,
        });

        app.cameraOn = !app.cameraOn;
      });
    } else {
      await RtcClient.startVideoCapture()
        .then(() => {
          this.setData({
            cameraOn: !this.data.cameraOn,
          });

          app.cameraOn = !app.cameraOn;
        })
        .catch((err) => {
          console.error('startVideoCapture', err);
          if (err.errNo === 10200) {
            tt.showToast({ title: '请打开相机权限！', icon: 'fail' });
            app.cameraOn = false;
            this.setData({
              cameraOn: false,
            });
          }
        });
    }
  },

  async handleAudioCapture() {
    if (this.data.micOn) {
      await RtcClient.unPublishStream('audio');
      await RtcClient.stopAudioCapture().then(() => {
        this.setData({
          micOn: !this.data.micOn,
        });

        app.micOn = !app.micOn;
      });
    } else {
      await RtcClient.startAudioCapture()
        .then(() => {
          this.setData({
            micOn: !this.data.micOn,
          });

          app.micOn = !app.micOn;
        })
        .catch((err) => {
          if (err.errNo === 10200) {
            tt.showToast({ title: '请打开麦克风权限！', icon: 'fail' });
            app.micOn = false;
            this.setData({
              micOn: false,
            });
          }
        });
      await RtcClient.publishStream('audio');
    }
  },

  async handleSwitchCamera() {
    await RtcClient.switchCamera(this.data.camera == 'back' ? 'front' : 'back');

    this.setData({
      camera: this.data.camera == 'back' ? 'front' : 'back',
    });
  },
  async handleSwitchSound() {
    await RtcClient.switchSound(this.data.sound === 'earpiece' ? 'speakerphone' : 'earpiece');

    this.setData({
      sound: this.data.sound === 'speakerphone' ? 'earpiece' : 'speakerphone',
    });
    app.sound = app.sound === 'speakerphone' ? 'earpiece' : 'speakerphone';
  },

  hanldePageWrapperScroll(e) {
    const curPage = Math.ceil(e.detail.scrollLeft / app.screenWidth) || 1;

    if (curPage !== this.data.curPage)
      this.setData({
        curPage,
      });
  },

  handlePlayerTap(e) {
    const { stream = {} } = e.currentTarget.dataset;
    if (stream?.isScreen) {
      this.setData({ viewMode: 'screen' });

      //   if (this.data.viewMode === 'stream') {
      //     this.setData({ viewMode: 'screen' });
      //   } else {
      //     this.setData({ viewMode: 'stream' });
      //   }
    }
  },

  handleScreenPlayerTap(e) {
    this.setData({ viewMode: 'stream' });
  },

  handleFullScreen(e) {
    this.setData({ viewMode: this.data.viewMode === 'screen' ? 'fullScreen' : 'screen' });
  },

  async handleLeaveRoom() {
    await this.exitRtcRoom();
    tt.navigateBack();
  },

  async exitRtcRoom(showToast = true) {
    if (showToast) {
      tt.showToast({ title: '离房成功' });
    }

    this.removeEventListener();
    this.setData({
      streamPages: [],
      //   dummyList: [],
    });
  },
});
