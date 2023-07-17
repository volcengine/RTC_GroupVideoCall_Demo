import VERTC from '@volcengine/rtc';
import { useState } from 'react';
import { Popover } from 'antd';
import Logo from '@/assets/img/Logo.png';
import Setting from '@/assets/img/Setting.svg';
import styles from './index.module.less';
import Icon from '../Icon';
import { DEMO_VERSION, Disclaimer, ReversoContex, UserAgreement } from '@/config';

interface HeaderProps {
  children?: React.ReactNode;
  hide?: boolean;
}

function Header(props: HeaderProps) {
  const { children, hide } = props;

  const [popoverOpen, setPopoverOpen] = useState<boolean>(false);

  return (
    <div
      className={styles.header}
      style={{
        display: hide ? 'none' : 'flex',
      }}
    >
      <div className={styles['header-logo']}>
        <img src={Logo} alt="Logo" />
      </div>

      {children}

      <div className={styles['header-right']}>
        <span className={styles['header-right-text']}>Demo Version:{DEMO_VERSION}</span>
        <span className={styles['header-right-text']}> / </span>
        <span className={styles['header-right-text']}>
          <span>SDK Version:{VERTC.getSdkVersion()}</span>
        </span>
        <Popover
          trigger="click"
          title={null}
          visible={popoverOpen}
          overlayClassName={styles['header-pop']}
          onVisibleChange={setPopoverOpen}
          content={
            <ul>
              <li
                onClick={() => {
                  setPopoverOpen(false);
                  window.open(Disclaimer, '_blank');
                }}
              >
                免责声明
              </li>
              <li
                onClick={() => {
                  setPopoverOpen(false);
                  window.open(ReversoContex, '_blank');
                }}
              >
                隐私政策
              </li>
              <li
                onClick={() => {
                  setPopoverOpen(false);
                  window.open(UserAgreement, '_blank');
                }}
              >
                用户协议
              </li>
            </ul>
          }
        >
          <button className={styles['header-setting-btn']}>
            <Icon src={Setting} />
          </button>
        </Popover>
      </div>
    </div>
  );
}

export default Header;
