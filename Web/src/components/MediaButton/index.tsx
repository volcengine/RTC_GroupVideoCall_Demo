import { message as Message } from 'antd';
import Icon from '../Icon';
import styles from './index.module.less';

interface MediaButtonProps {
  text?: string;
  value?: boolean;
  icon: string;
  onChange?: (value: boolean) => void;
  onClick?: () => void;
  className?: string;
  iconClassName?: string;
  disabled?: boolean;
  disableMsg?: string;
}

function MediaButton(props: MediaButtonProps) {
  const {
    onChange,
    value,
    text,
    icon,
    onClick,
    className,
    iconClassName,
    disableMsg,
    disabled = false,
  } = props;

  const handleMediaButtonClick = () => {
    if (!disabled) {
      onChange && onChange(!value);
      onClick && onClick();
    }

    if (disabled && disableMsg) {
      Message.error(disableMsg);
    }
  };

  return (
    <div className={`${styles.mediaButton} ${className || ''}`} onClick={handleMediaButtonClick}>
      <div className={`${styles.iconWrapper} ${iconClassName || ''}`}>
        <Icon src={icon} />
      </div>
      {text && <span className={styles.mediaButtonText}>{text}</span>}
    </div>
  );
}

export default MediaButton;
