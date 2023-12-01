import { Button, Modal } from 'antd';
import { useState } from 'react';
import { ExclamationOutlined } from '@ant-design/icons';

import { useNavigate } from 'react-router-dom';
import { useSelector, useDispatch } from 'react-redux';
import { useTranslation } from 'react-i18next';
import MediaButton from '@/components/MediaButton';
import { getIcon } from '@/components/MediaButton/utils';
import styles from './index.module.less';
import { RootState } from '@/store';
import { localLeaveRoom } from '@/store/slices/room';
import { resetConfig } from '@/store/slices/stream';
import TeaClient from '@/lib/TeaClient';

function Stop() {
  const [modalVisible, setModalVisible] = useState(false);

  const room = useSelector((state: RootState) => state.room);
  const { t } = useTranslation();
  const navigate = useNavigate();
  const dispatch = useDispatch();

  const handleStop = async () => {
    dispatch(localLeaveRoom());
    dispatch(resetConfig());
    try {
      navigate(`/login?roomId=${room.roomId?.replace('call_', '')}`);
    } catch (error: any) {
      console.error('error', error);
      const failureReason = error?.toString() || 'unknown';
      TeaClient.reportDemoInternalException(failureReason);
    }
  };

  return (
    <div className={styles.menuRight}>
      <MediaButton
        iconClassName={styles.menuCloseIcon}
        className={styles.menuButton}
        onClick={() => {
          TeaClient.reportOpenUserLeaveRoomMenu();
          setModalVisible(true);
        }}
        text={t('End')}
        icon={getIcon('stop')}
      />

      <Modal
        title={null}
        visible={modalVisible}
        footer={null}
        closable={false}
        transitionName=""
        maskTransitionName=""
      >
        <div className={styles.stopModal}>
          <div className="header">
            <ExclamationOutlined
              style={{
                color: '#fff',
                backgroundColor: '#FF7D00',
                borderRadius: '50%',
              }}
            />
            <span className="stopText">{t('leaveReconfirm')}</span>
          </div>
          <Button
            type="primary"
            danger
            onClick={() => {
              TeaClient.reportUserLeaveRoom(true);
              setModalVisible(false);
              handleStop();
            }}
          >
            {t('OK')}
          </Button>
          <Button
            onClick={() => {
              TeaClient.reportUserLeaveRoom(false);
              setModalVisible(false);
            }}
          >
            {t('Cancel')}
          </Button>
        </div>
      </Modal>
    </div>
  );
}

export default Stop;
