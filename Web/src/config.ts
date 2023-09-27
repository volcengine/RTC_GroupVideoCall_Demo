import { AudioProfileType } from '@volcengine/rtc';

export const isProd = location.origin.includes('demo.volcvideo.com');

export const DEMO_VERSION = '1.1.1';

export const Disclaimer = 'https://www.volcengine.com/docs/6348/68916';
export const ReversoContex = 'https://www.volcengine.com/docs/6348/68918';
export const UserAgreement = 'https://www.volcengine.com/docs/6348/128955';

export const BusinessId = 'veRtc_bid_videocall';

export const isDev = process.env.NODE_ENV === 'development';

export const BASENAME = isDev
  ? '/'
  : isProd
  ? '/rtc/solution/videocall'
  : (window as unknown as any).BASEURL || '/';

export const RESOLUTIOIN_LIST = [
  {
    text: '1280 * 720',
    val: {
      width: 1280,
      height: 720,
      frameRate: 15,
      maxKbps: 1200,
    },
  },
  {
    text: '960 * 540',
    val: {
      width: 960,
      height: 540,
      frameRate: 15,
      maxKbps: 1000,
    },
  },
  {
    text: '640 * 360',
    val: {
      width: 640,
      height: 360,
      frameRate: 15,
      maxKbps: 600,
    },
  },
  {
    text: '320 * 180',
    val: {
      width: 320,
      height: 180,
      frameRate: 15,
      maxKbps: 300,
    },
  },
];

export const AudioProfile = [
  {
    text: '24kbps',
    type: AudioProfileType.fluent,
  },
  {
    text: '48kbps',
    type: AudioProfileType.standard,
  },
  {
    text: '128kbps',
    type: AudioProfileType.hd,
  },
];

export const MAX_PLAYERS = 4;

export const HOST = 'your_own_host_url';
export const appId = 'your_app_id';
export const appKey = 'your_app_key';
export const volcAk = 'your_volc_ak';
export const volcSk = 'your_volc_sk';
