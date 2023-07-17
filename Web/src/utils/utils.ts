import { v4 as uuidv4 } from 'uuid';

interface VideoCall {
  deviceId?: string;
  login_token?: string;
}

class Utils {
  videoCallInfo: VideoCall;

  constructor() {
    const videoCallInfo: VideoCall = JSON.parse(localStorage.getItem('videoCallInfo') || '{}');
    this.videoCallInfo = videoCallInfo;
    this.init();
  }

  private init() {
    if (!this.videoCallInfo.deviceId) {
      const deviceId = uuidv4();
      this.videoCallInfo.deviceId = deviceId;
      localStorage.setItem('videoCallInfo', JSON.stringify(this.videoCallInfo));
    }
  }

  getDeviceId = (): string => this.videoCallInfo.deviceId!;

  setLoginToken = (token: string): void => {
    this.videoCallInfo.login_token = token;
  };

  getLoginToken = (): string => this.videoCallInfo.login_token!;

  formatTime = (time: number): string => {
    if (time < 0) {
      return '00:00';
    }
    let minutes: number | string = Math.floor(time / 60);
    let seconds: number | string = time % 60;
    minutes = minutes > 9 ? `${minutes}` : `0${minutes}`;
    seconds = seconds > 9 ? `${seconds}` : `0${seconds}`;

    return `${minutes}:${seconds}`;
  };

  setSessionInfo = (params: { [key: string]: any }) => {
    Object.keys(params).forEach((key) => {
      sessionStorage.setItem(key, params[key]);
    });
  };

  getUrlArgs = () => {
    const args = {} as { [key: string]: string };
    const query = window.location.search.substring(1);
    const pairs = query.split('&');
    for (let i = 0; i < pairs.length; i++) {
      const pos = pairs[i].indexOf('=');
      if (pos === -1) continue;
      const name = pairs[i].substring(0, pos);
      let value = pairs[i].substring(pos + 1);
      value = decodeURIComponent(value);
      args[name] = value;
    }
    return args;
  };

  checkLoginInfo = () => {
    const { roomId } = this.getUrlArgs();
    roomId && this.setSessionInfo({ roomId });
    const _roomId = sessionStorage.getItem('roomId');
    const _uid = sessionStorage.getItem('username');
    let hasLogin = true;
    if (!_roomId || !_uid) {
      hasLogin = false;
    } else if (
      !/^[0-9a-zA-Z_\-@.]{1,128}$/.test(_roomId) ||
      !/^[0-9a-zA-Z_\-@.]{1,128}$/.test(_uid)
    ) {
      hasLogin = false;
    }
    return hasLogin;
  };

  getLoginInfo = () => {
    const roomId = sessionStorage.getItem('roomId') as string;
    const username = sessionStorage.getItem('username') as string;
    const publishAudio = sessionStorage.getItem('publishAudio');
    const publishVideo = sessionStorage.getItem('publishVideo');

    return {
      roomId,
      username,
      publishAudio,
      publishVideo,
    };
  };

  removeLoginInfo = () => {
    const variable = ['roomId', 'username', 'publishAudio', 'publishVideo'];
    variable.forEach((v) => sessionStorage.removeItem(v));
  };
}

export default new Utils();
