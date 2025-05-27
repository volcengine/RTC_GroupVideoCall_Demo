import { AudioProfileType } from '@volcengine/rtc';

export const isProd = location.origin.includes('demo.volcvideo.com');
export const isDev = process.env.NODE_ENV === 'development';
export const isBoe = location.origin.includes('rtc_demo_videocall');

export const DEMO_VERSION = '1.1.1';

export const Disclaimer = 'https://www.volcengine.com/docs/6348/68916';
export const ReversoContex = 'https://www.volcengine.com/docs/6348/68918';
export const UserAgreement = 'https://www.volcengine.com/docs/6348/128955';

export const BusinessId = 'veRtc_bid_videocall';

// 是否上报数据
export const ENABLE_TEA = true;

// 数据分离，国内，海外需走不同的App ID
// 国内
export const TEA_APP_ID_VOLC = 554123;
export const TEA_CHANNEL_VOLC = 'cn';

// 海外
export const TEA_APP_ID_BYTEPLUS = 557271;
export const TEA_CHANNEL_BYTEPLUS = 'sg';

export const BASENAME = isDev
  ? '/'
  : isProd
  ? '/rtc/solution/videocall'
  : (window as unknown as any).BASEURL || '/';

export const RESOLUTIOIN_LIST = [
  {
    text: '1920 * 1080',
    val: {
      width: 1920,
      height: 1080,
      frameRate: 15,
      maxKbps: 3000,
    },
  },
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

export const CLARITY_RESOLUTION = '1920 * 1080';
export const FLUENCY_RESOLUTION = '1280 * 720';

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

/**
 * @note 您可以使用 https://common.rtc.volcvideo.com/rtc_demo_special 作为测试服务器域名，仅提供跑通测试服务，无法保障正式需求。
 */
export const HOST = 'https://common.rtc.volcvideo.com/rtc_demo_special';
export const appId = 'your_app_id';
export const appKey = 'your_app_key';
export const volcAk = 'your_volc_ak';
export const volcSk = 'your_volc_sk';
