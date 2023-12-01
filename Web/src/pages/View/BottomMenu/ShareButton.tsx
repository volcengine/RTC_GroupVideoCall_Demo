import { useDispatch, useSelector } from 'react-redux';
import { useEffect, useMemo, useRef, useState } from 'react';
import { Radio, RadioChangeEvent, message as Message, Modal, Button } from 'antd';
import { useTranslation } from 'react-i18next';
import { ExclamationOutlined } from '@ant-design/icons';
import MediaButton from '@/components/MediaButton';
import { getIcon } from '@/components/MediaButton/utils';
import styles from './index.module.less';
import ArrowIcon from '@/assets/img/Arrow.svg';
import { RootState } from '@/store';
import RtcClient from '@/lib/RtcClient';
import { startShare } from '@/store/slices/room';
import { RESOLUTIOIN_LIST, CLARITY_RESOLUTION, FLUENCY_RESOLUTION } from '@/config';
import { updateShareConfig } from '@/store/slices/stream';
import TeaClient from '@/lib/TeaClient';

interface ShareButtonProps {
  shared?: boolean;
}

function ShareButton(props: ShareButtonProps) {
  const { shared } = props;
  const dispatch = useDispatch();
  const [showOptions, setShowOptions] = useState(false);
  const room = useSelector((state: RootState) => state.room);
  const { t } = useTranslation();

  const isLocalUserShared = useMemo(() => {
    const user = room.localUser;
    return user?.userId === room.shareUser;
  }, [room.localUser, room.shareUser]);

  const shareLoadingRef = useRef<boolean>(false);

  const [modalVisible, setModalVisible] = useState(false);
  const [allowShare, setAllowShare] = useState(false);

  const isSafari = useMemo(() => {
    return /Safari/.test(navigator.userAgent) && !/Chrome/.test(navigator.userAgent);
  }, []);

  const shareScreenConfig = useSelector((state: RootState) => state.stream.shareScreenConfig);
  const btnRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const hidePop = (e: Event) => {
      if (!btnRef.current?.contains(e.target as unknown as HTMLElement)) {
        setShowOptions(false);
      }
    };

    window.addEventListener('click', hidePop);
    return () => {
      window.removeEventListener('click', hidePop);
    };
  }, [btnRef]);

  const handleShare = async () => {
    const res = await RtcClient.startScreenCapture();
    if (res === 'success') {
      TeaClient.reportStartScreenSharing();
      dispatch(startShare({ shareUser: room.localUser?.userId }));
    } else {
      TeaClient.reportStopScreenSharing();
      if (!isLocalUserShared && res === 'Permission denied') {
        RtcClient.sendServerMessage('videocallEndShareScreen');
      }
    }
  };

  const handleShareScreen = async () => {
    TeaClient.reportToggleShareScreen();

    setShowOptions(false);
    if (shareLoadingRef.current) {
      return;
    }

    shareLoadingRef.current = true;
    const shareRes: any = await RtcClient.sendServerMessage('videocallStartShareScreen');

    shareLoadingRef.current = false;
    if (shareRes.message_type !== 'return') {
      return;
    }
    if (shareRes.code !== 200) {
      if (shareRes.code === 485) {
        // todo “xxx正在屏幕共享中，请稍后…
        Message.error(t('isSharing'));
      }
      setAllowShare(false);
      return;
    }

    if (isSafari) {
      setAllowShare(true);
      setModalVisible(true);
    } else {
      handleShare();
    }
  };

  const handleSetShareResolution = () => {
    setShowOptions(!showOptions);
  };

  const handleShareScreenConfigChange = async (e: RadioChangeEvent) => {
    const newConfig = e.target.value;

    const encodeConfig = RESOLUTIOIN_LIST.find((resolution) => resolution.text === newConfig);

    if (newConfig === CLARITY_RESOLUTION) {
      TeaClient.reportSwitchShareScreenPreferenceToClarity(true);
    } else {
      TeaClient.reportSwitchShareScreenPreferenceToClarity(false);
    }

    const screenEncoderConfig = {
      ...encodeConfig!.val,
      contentHint: newConfig === CLARITY_RESOLUTION ? 'text' : 'motion',
    };

    // @ts-expect-error make sure contentHint of screenEncoderConfig works
    await RtcClient.setScreenEncoderConfig(screenEncoderConfig);

    setShowOptions(false);

    dispatch(
      updateShareConfig({
        shareScreenConfig: newConfig,
      })
    );
  };

  return (
    <div className={styles.menuButton} ref={btnRef}>
      <MediaButton
        iconClassName={styles.shareButtonIcon}
        text={shared ? undefined : t('ShareScreen')}
        icon={getIcon('shareScreen')}
        onClick={handleShareScreen}
      />
      <div className={styles.ArrowIcon} onClick={handleSetShareResolution}>
        <img src={ArrowIcon} alt="arrow" />
      </div>

      <div
        className={styles.devicesList}
        onMouseLeave={() => {
          setShowOptions(false);
        }}
        style={{
          display: showOptions ? 'block' : 'none',
        }}
      >
        <div className={styles.mediaDevicesContent}>
          <Radio.Group onChange={handleShareScreenConfigChange} value={shareScreenConfig}>
            <Radio value={CLARITY_RESOLUTION}>{t('ClarityPreferred')}</Radio>
            <Radio value={FLUENCY_RESOLUTION}>{t('FluencyPreferred')}</Radio>
          </Radio.Group>
        </div>
      </div>

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
            <span className="stopText">{t('safariShare')}</span>
          </div>
          <Button
            type="primary"
            danger
            disabled={!allowShare}
            onClick={() => {
              setModalVisible(false);

              handleShare();
            }}
          >
            {t('OK')}
          </Button>
          <Button
            onClick={() => {
              RtcClient.sendServerMessage('videocallEndShareScreen');
              setModalVisible(false);
              setAllowShare(false);
            }}
          >
            {t('Cancel')}
          </Button>
        </div>
      </Modal>
    </div>
  );
}

export default ShareButton;
