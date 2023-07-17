const app = getApp();
const request = require('../../common/request/index');
const RtcClient = require('../../common/RtcClient/index');

let joining = false;

Page({
  data: {
    roomIdError: '',
    micOn: true,
    cameraOn: true,
    sound: app.sound,
    roomId: '',
    userId: app.userId,
    show: true,
  },

  async onShow() {
    console.log('index load', app);
    joining = false;

    if (app.cameraOn) {
      await RtcClient.startVideoCapture().catch((err) => {
        console.error('startVideoCapture', err);
        if (err.errNo === 10200) {
          tt.showToast({ title: '请打开相机权限！', icon: 'fail' });
          app.cameraOn = false;
          this.setData({
            cameraOn: false,
            show: false,
          });
        }
      });
    }
    if (app.micOn) {
      await RtcClient.startAudioCapture().catch((err) => {
        console.error('startAudioCapture', err);
        if (err.errNo === 10200) {
          tt.showToast({ title: '请打开麦克风权限！', icon: 'fail' });
          app.micOn = false;
          this.setData({
            micOn: false,
          });
        }
      });
    }

    RtcClient.switchSound(app.sound);
    this.setData({
      micOn: app.micOn,
      cameraOn: app.cameraOn,
      sound: app.sound,
      show: true,
    });
  },

  async onHide() {
    // await RtcClient.stopVideoCapture();
    // await RtcClient.stopAudioCapture();
    this.setData({
      show: false,
    });
  },

  validate(values) {
    const { roomId } = values;
    let ok = true;
    const reg = /[0-9]{1,18}/;

    if (!roomId) {
      this.setData({ roomIdError: '请输入1~18位纯数字' });
      ok = false;
    } else if (!roomId.match(reg) || roomId.match(reg)[0] !== roomId) {
      this.setData({ roomIdError: '请输入1~18位纯数字' });
      ok = false;
    }

    return ok;
  },

  handleRoomIdinput(e) {
    this.setData({ roomIdError: '', roomId: e.detail.value });
  },

  async ttLogin() {
    return new Promise((resolve, reject) => {
      tt.login({
        success(res) {
          resolve(res);
        },
      });
    });
  },

  async handleJoinRoom(e) {
    if (joining) {
      return;
    }
    joining = true;
    const roomId = this.data.roomId;

    if (!this.validate({ roomId })) {
      joining = false;

      return;
    }

    let userId = this.data.userId;
    if (!userId) {
      const loginRes = await this.ttLogin();
      console.log('loginRes', loginRes);
      userId = loginRes.anonymousCode.slice(0, 8);
      app.userId = userId;
      this.setData({ userId: userId });
    }

    tt.showLoading({ title: '请求中' });
    // 免密登录，获取login_token
    const freeLoginRes = await request.post('/login', {
      event_name: 'passwordFreeLogin',
      device_id: userId,
      content: JSON.stringify({
        user_name: userId,
      }),
    });

    console.log('freeLoginRes:', freeLoginRes);

    if (freeLoginRes.statusCode !== 200) {
      console.log('请求token失败', freeLoginRes.errMsg);
      tt.showToast({ title: '登录失败', icon: 'fail' });
      joining = false;

      return;
    }

    await request.post('/login', {
      event_name: 'setAppInfo',
      device_id: userId,
      content: JSON.stringify({
        app_id: app.appId,
        app_key: app.appKey,
        volc_ak: app.accessKeyID,
        volc_sk: app.secretAccesskey,
        scenes_name: 'videocall',
        login_token: freeLoginRes.data.response.login_token,
      }),
    });

    const getTokenRes = await request.post('/common', {
      event_name: 'coGetToken',
      content: JSON.stringify({
        room_id: `call_${roomId}`,
        user_id: `${userId}`,
        scene_name: '',
        expire: 15 * 60,
      }),
      app_id: app.appId,
      device_id: userId,
    });

    joining = false;

    if (getTokenRes.statusCode === 200) {
      tt.hideLoading();
      tt.navigateTo({
        url: `/pages/room/index?roomId=${roomId}&userId=${userId}&token=${encodeURIComponent(
          getTokenRes.data.response.rtc_token
        )}`,
      });
    } else {
      console.log('请求token失败', getTokenRes.errMsg);
      tt.showToast({ title: '请求token失败', icon: 'fail' });
    }
  },
  async handleMic(e) {
    if (!this.data.micOn) {
      await RtcClient.startAudioCapture()
        .then(() => {
          this.setData({
            micOn: true,
          });

          app.micOn = true;
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
    } else {
      await RtcClient.stopAudioCapture().then(() => {
        this.setData({
          micOn: false,
        });

        app.micOn = false;
      });
    }
  },

  async handleCamera(e) {
    if (!this.data.cameraOn) {
      await RtcClient.startVideoCapture()
        .then(() => {
          this.setData({
            cameraOn: true,
            show: true,
          });

          app.cameraOn = true;
        })
        .catch((err) => {
          console.error('startVideoCapture', err);
          if (err.errNo === 10200) {
            tt.showToast({ title: '请打开相机权限！', icon: 'fail' });
            app.cameraOn = false;
            this.setData({
              cameraOn: false,
              show: false,
            });
          }
        });
    } else {
      await RtcClient.stopVideoCapture().then(() => {
        this.setData({
          cameraOn: false,
          show: false,
        });

        app.cameraOn = false;
      });
    }
  },

  async handleSound(e) {
    await RtcClient.switchSound(this.data.sound === 'earpiece' ? 'speakerphone' : 'earpiece');

    this.setData({
      sound: this.data.sound === 'speakerphone' ? 'earpiece' : 'speakerphone',
    });
    app.sound = app.sound === 'speakerphone' ? 'earpiece' : 'speakerphone';
  },

  handleRtcRoomError: function (err) {
    console.error('handleRtcRoomError', err);
  },
});
