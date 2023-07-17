App({
  micOn: true,
  cameraOn: true,
  sound: 'speakerphone',
  RtcClient: null,
  appId: 'Your_AppID', 
  appKey: 'Your_AppKey',
  accessKeyID: 'Your_AccessKey',
  secretAccesskey: 'Your_SecretAccesskey',
  userId: '',
  host: 'https://common.rtc.volcvideo.com',
  screenWidth: 0,
  duration: 0,
  onLaunch: function () {
    const RtcClient = tt.createRtcRoomContext({
      appId: this.appId,
    });

    this.RtcClient = RtcClient;

    const { screenWidth } = tt.getSystemInfoSync();

    this.screenWidth = screenWidth;
  },
});
