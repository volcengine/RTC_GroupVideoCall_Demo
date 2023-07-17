const app = getApp();

const DeviceOpen = 1;
const DeviceClose = 0;

const startAudioCapture = async () => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeAudioCapture({
      state: DeviceOpen,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};
const stopAudioCapture = async () => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeAudioCapture({
      state: DeviceClose,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const startVideoCapture = async () => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeVideoCapture({
      state: DeviceOpen,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};
const stopVideoCapture = async () => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeVideoCapture({
      state: DeviceClose,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const switchSound = async (device) => {
  return new Promise((resolve, reject) => {
    app.RtcClient.setAudioPlaybackDevice({
      device,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const switchCamera = async (camera) => {
  return new Promise((resolve, reject) => {
    app.RtcClient.switchCamera({
      camera: camera,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const joinRoom = async (config) => {
  console.log('joinRoom config', config);
  return new Promise((resolve, reject) => {
    app.RtcClient.joinRtcRoom({
      ...config,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const leaveRoom = async (roomId) => {
  return new Promise((resolve, reject) => {
    app.RtcClient.exitRtcRoom({
      roomId,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const publishStream = async (streamType) => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeStreamPublishState({
      state: 1,
      streamType,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

const unPublishStream = async (streamType) => {
  return new Promise((resolve, reject) => {
    app.RtcClient.changeStreamPublishState({
      state: 0,
      streamType,
      success() {
        resolve();
      },
      fail(err) {
        reject(err);
      },
    });
  });
};

module.exports.startAudioCapture = startAudioCapture;
module.exports.stopAudioCapture = stopAudioCapture;
module.exports.startVideoCapture = startVideoCapture;
module.exports.stopVideoCapture = stopVideoCapture;
module.exports.publishStream = publishStream;
module.exports.unPublishStream = unPublishStream;
module.exports.switchSound = switchSound;
module.exports.switchCamera = switchCamera;
module.exports.joinRoom = joinRoom;
module.exports.leaveRoom = leaveRoom;
