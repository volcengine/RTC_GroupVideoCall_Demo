import CameraOn from '@/assets/img/CameraOn.svg';
import CameraOff from '@/assets/img/CameraOff.svg';
import MicrophoneOn from '@/assets/img/MicrophoneOn.svg';
import MicrophoneOff from '@/assets/img/MicrophoneOff.svg';
import SoundOn from '@/assets/img/SoundOn.svg';
import RealTimeData from '@/assets/img/RealTimeData.svg';
import Setting from '@/assets/img/Setting.svg';
import ShareScreen from '@/assets/img/ShareScreen.svg';
import Stop from '@/assets/img/Stop.svg';
import Close from '@/assets/img/Close.svg';

const IconMap = {
  cameraOn: CameraOn,
  cameraOff: CameraOff,
  microphoneOn: MicrophoneOn,
  microphoneOff: MicrophoneOff,
  soundOn: SoundOn,
  realtime: RealTimeData,
  setting: Setting,
  shareScreen: ShareScreen,
  stop: Stop,
  close: Close,
};

export const getIcon = (type: string, status = ''): string =>
  IconMap[`${type}${status}` as keyof typeof IconMap];
