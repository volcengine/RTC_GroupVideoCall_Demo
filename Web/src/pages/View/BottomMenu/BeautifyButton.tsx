import { useEffect, useRef, useState } from 'react';
import { EffectBeautyMode } from '@volcengine/rtc/extension-beauty';
import { Slider } from 'antd';
import { useDispatch, useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';
import MediaButton from '@/components/MediaButton';
import ArrowIcon from '@/assets/img/Arrow.svg';
import BeautifyIcon from '@/assets/img/Beautify.svg';
import BeautifyOffIcon from '@/assets/img/BeautifyOff.svg';

import styles from './index.module.less';
import { RootState } from '@/store';
import { beautyExtension } from '@/lib/RtcClient';
import { setBeauty } from '@/store/slices/room';
import TeaClient from '@/lib/TeaClient';

interface BeautifyButtonProps {
  shared?: boolean;
}

let teaReportTimer: any = null;

function BeautifyButton(props: BeautifyButtonProps) {
  const { shared } = props;

  const [showOptions, setShowOptions] = useState(false);
  const { t } = useTranslation();
  const dispatch = useDispatch();

  const beautyOn = useSelector((state: RootState) => state.room.beautyOn);
  const beautyEnabled = useSelector((state: RootState) => state.room.beautyEnabled);

  const [white, setWhite] = useState<number>(50);
  const [smooth, setSmooth] = useState<number>(50);
  const [sharpen, setSharpen] = useState<number>(50);

  const btnRef = useRef<HTMLDivElement>(null);

  const handleShowOptions = () => {
    if (!beautyEnabled) {
      return;
    }
    setShowOptions(!showOptions);
  };

  const handleBeauty = (mode: EffectBeautyMode, value: number) => {
    if (mode === EffectBeautyMode.EFFECT_WHITE_MODE) {
      setWhite(value);
    }
    if (mode === EffectBeautyMode.EFFECT_SMOOTH_MODE) {
      setSmooth(value);
    }
    if (mode === EffectBeautyMode.EFFECT_SHARPEN_MODE) {
      setSharpen(value);
    }
    beautyExtension.setBeautyIntensity(mode, value / 100);

    clearTimeout(teaReportTimer);

    teaReportTimer = setTimeout(() => {
      TeaClient.reportUpdateBeautyOptions(mode, white, smooth, sharpen);
    }, 1000);
  };

  const handleStartBeauty = () => {
    if (beautyOn) {
      TeaClient.reportToggleBeauty(false);
      dispatch(setBeauty(false));
      beautyExtension.disableBeauty();
    } else {
      TeaClient.reportToggleBeauty(true);
      dispatch(setBeauty(true));
      beautyExtension.enableBeauty();
    }
  };

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

  return (
    <div className={styles.menuButton} ref={btnRef}>
      <MediaButton
        iconClassName={styles.beautyButtonIcon}
        onClick={handleStartBeauty}
        text={shared ? undefined : t('Beautify')}
        icon={beautyOn ? BeautifyIcon : BeautifyOffIcon}
        disabled={!beautyEnabled}
        disableMsg={t('notSupported')}
      />
      <div className={styles.ArrowIcon} onClick={handleShowOptions}>
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
        <div className={styles.beautyContent}>
          <div className={styles.beautyItem}>
            <span>{t('White')}</span>
            <Slider
              value={white}
              onChange={(v) => handleBeauty(EffectBeautyMode.EFFECT_WHITE_MODE, v)}
            />
          </div>
          <div className={styles.beautyItem}>
            <span>{t('Smooth')}</span>
            <Slider
              value={smooth}
              onChange={(v) => handleBeauty(EffectBeautyMode.EFFECT_SMOOTH_MODE, v)}
            />
          </div>
          <div className={styles.beautyItem}>
            <span>{t('Sharpen')}</span>
            <Slider
              value={sharpen}
              onChange={(v) => handleBeauty(EffectBeautyMode.EFFECT_SHARPEN_MODE, v)}
            />
          </div>
        </div>
      </div>
    </div>
  );
}

export default BeautifyButton;
