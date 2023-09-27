import { LocalVideoStats, RemoteVideoStats } from '@volcengine/rtc';
import { AudioRealTime, DataItemProps, Status, VideoRealTime } from './DataCard';

const AudioStats = ['receivedKBitrate', 'rtt'];
const VideoStats = [
  'resolution',
  'receivedKBitrate',
  'decoderOutputFrameRate',
  'rtt',
  'videoLossRate',
];
type GetDataParameters =
  | {
      data: VideoRealTime;
      type: 'video';
    }
  | {
      data: AudioRealTime;
      type: 'audio';
    };

const translation = (stat: string, t: (s: string) => string) => {
  if (stat.includes('Bitrate')) {
    return t('Bitrate');
  }

  if (stat.includes('FrameRate')) {
    return t('frameRate');
  }
  if (stat.includes('LossRate')) {
    return t('LossRate');
  }
  if (stat.includes('rtt')) {
    return t('Delay');
  }

  return t('resolution');
};

export const getDataList = ({ data, type }: GetDataParameters, t: any): DataItemProps[] => {
  const Stats = type === 'video' ? VideoStats : AudioStats;

  return Stats.map((stat) => {
    if (stat === 'receivedKBitrate' && !!data.isLocal) {
      if (type === 'audio') {
        stat = 'sendKBitrate';
      } else {
        stat = 'sentKBitrate';
      }
    }

    if (stat === 'decoderOutputFrameRate' && !!data.isLocal) {
      stat = 'encoderOutputFrameRate';
    }

    const baseItem = {
      name: translation(stat, t),
      status: Status.UNKNOWN,
    };

    let statData;
    if (type === 'video') {
      statData = data.videoStats;
    } else {
      statData = data.audioStats;
    }

    let text = `${(statData as unknown as any)?.[stat] ?? ''}`;
    const num = Math.floor(Number(text) * 1000) / 1000;
    text = num.toString();

    if (stat === 'resolution') {
      if (data.isLocal) {
        text = `${(statData as unknown as LocalVideoStats)?.encodedFrameHeight! || '-'}P`;
      } else {
        text = `${(statData as unknown as RemoteVideoStats)?.height! || '-'}P`;
      }
    }
    if (['Bitrate', 'LossRate', 'FrameRate'].some((t) => stat.includes(t))) {
      text = text.split('.')[0] || '-';
    }

    return {
      ...baseItem,
      text,
    };
  });
};
