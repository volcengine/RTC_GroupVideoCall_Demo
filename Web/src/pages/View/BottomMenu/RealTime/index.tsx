import { Modal, Radio } from 'antd';
import { useMemo, useState } from 'react';
import { useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';
import { getIcon } from '@/components/MediaButton/utils';
import MediaButton from '@/components/MediaButton';
import DataCard, { AudioRealTime, VideoRealTime } from './DataCard';
import { RootState } from '@/store';

import styles from './index.module.less';
import TeaClient from '@/lib/TeaClient';

interface RealTimeProps {
  shared?: boolean;
}

function RealTime(props: RealTimeProps) {
  const { shared } = props;
  const room = useSelector((state: RootState) => state.room);
  const { t } = useTranslation();

  const [modalVisible, setModalVisible] = useState(false);
  const [dataType, setDataType] = useState<'video' | 'audio'>('video');

  const realTimeData = useMemo(() => {
    const renderUsers = [room.localUser, ...room.remoteUsers];

    return renderUsers.reduce(
      (realTimeData, cur, index) => {
        const baseStat = {
          userId: cur.userId,
          username: cur.username!,
        };
        const videoStat = {
          ...baseStat,
          videoStats: cur.videoStats,
        };

        const audioStat = {
          ...baseStat,
          audioStats: cur.audioStats,
        };

        if (index === 0) {
          realTimeData.audio.push({
            isLocal: true,
            ...audioStat,
          });
          realTimeData.video.push({
            isLocal: true,
            ...videoStat,
          });
        } else {
          realTimeData.audio.push(audioStat);
          realTimeData.video.push(videoStat);
        }

        return realTimeData;
      },
      {
        video: [],
        audio: [],
      } as {
        video: VideoRealTime[];
        audio: AudioRealTime[];
      }
    );
  }, [room.localUser, room.remoteUsers]);

  const handleClose = () => {
    setModalVisible(false);
    TeaClient.reportToggleStatisticMenu(false);
  };

  const switchDataType = (e: any) => {
    setDataType(e.target.value);
    TeaClient.reportToggleStatisticMediaType(e.target.value);
  };

  return (
    <>
      <MediaButton
        className={shared ? styles.shareMenuButton : styles.menuButton}
        iconClassName={styles.menuButtonIcon}
        text={shared ? undefined : t('realTimeData')}
        icon={getIcon('realtime')}
        onClick={() => {
          setModalVisible(true);
          TeaClient.reportToggleStatisticMenu(true);
        }}
      />

      <Modal
        title={t('realTimeData')}
        visible={modalVisible}
        cancelText={t('Cancel')}
        okText={t('OK')}
        closeIcon={<img src={getIcon('close')} />}
        transitionName=""
        maskTransitionName=""
        width={400}
        wrapClassName={styles.realTimeDataModal}
        onOk={handleClose}
        onCancel={handleClose}
      >
        <Radio.Group onChange={switchDataType} value={dataType}>
          <Radio value="video">{t('Video')}</Radio>
          <Radio value="audio">{t('Audio')}</Radio>
        </Radio.Group>
        <div className={styles.realTimeDataWrapper} />
        {realTimeData[dataType].map((data, index) => (
          <DataCard data={data} type={dataType} key={data.userId || index} />
        ))}
      </Modal>
    </>
  );
}

export default RealTime;
