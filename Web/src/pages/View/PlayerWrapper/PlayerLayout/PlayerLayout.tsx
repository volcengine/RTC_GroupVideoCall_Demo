import { useEffect, useRef } from 'react';
import { useSelector } from 'react-redux';
import { IUser, LocalUser } from '@/store/slices/room';
import { RootState } from '@/store';
import styles from './playerLayout.module.less';
import VideoPlayer from './VideoPlayer';

interface IProps {
  renderUsers: (IUser | LocalUser)[];
  curPlayerPage: number;
  isLocalUserShared: boolean;
  isRemoteUserShared: boolean;
}

const setSize = (ele: HTMLElement, width: number, height: number) => {
  ele.style.minWidth = `${width}px`;
  ele.style.minHeight = `${height}px`;
  ele.style.maxWidth = `${width}px`;
  ele.style.maxHeight = `${height}px`;
  ele.style.width = `${width}px`;
  ele.style.height = `${height}px`;
};

const FLEX_GAP = 8;

function PlayerLayout(props: IProps) {
  const { renderUsers, curPlayerPage, isLocalUserShared, isRemoteUserShared } = props;
  const room = useSelector((state: RootState) => state.room);

  const layoutRef = useRef<HTMLDivElement>(null);
  const playersRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const resize = () => {
      const layoutDom = layoutRef.current!;

      if (!layoutDom.offsetHeight) {
        return;
      }

      const Height = layoutDom.offsetHeight;
      const Width = layoutDom.offsetWidth;
      const len = renderUsers.length;

      const rows = len > 2 ? 2 : 1;

      const cols = len > 1 ? 2 : 1;

      const w = (Width - (cols - 1) * FLEX_GAP) / cols;

      const h = (Height - (rows - 1) * FLEX_GAP) / rows;
      // 默认宽度有剩余可以放下16 / 9  的视频区
      let eleHeight = h;
      let eleWidth = (h / 9) * 16;
      if (w / h < 16 / 9) {
        eleWidth = w;
        eleHeight = (w / 16) * 9;
      }

      Array.prototype.forEach.call(playersRef.current?.children, (ele) => {
        setSize(ele as HTMLDivElement, eleWidth, eleHeight);
      });

      setSize(
        playersRef?.current!,
        eleWidth * cols + FLEX_GAP * (cols - 1),
        eleHeight * rows + FLEX_GAP * (rows - 1)
      );
    };

    if (!room.shareUser) {
      window.addEventListener('resize', resize);
    }

    if (isRemoteUserShared) {
      layoutRef.current!.style.width = 'initial';
      layoutRef.current!.style.height = 'initial';
      layoutRef.current!.style.alignItems = 'initial';
      playersRef.current!.style.width = 'initial';
      playersRef.current!.style.height = 'initial';
      playersRef.current!.style.minWidth = 'initial';
      playersRef.current!.style.minHeight = 'initial';
      playersRef.current!.style.maxWidth = 'initial';
      playersRef.current!.style.maxHeight = 'initial';
      Array.prototype.forEach.call(playersRef.current?.children, (ele) => {
        setSize(ele as HTMLDivElement, 332, 187);
      });
    }

    if (layoutRef.current && !room.shareUser) {
      resize();
      layoutRef.current!.style.width = '100%';
      layoutRef.current!.style.height = '100%';
      layoutRef.current!.style.alignItems = 'center';
    }

    return () => {
      window.removeEventListener('resize', resize);
    };
  }, [renderUsers, room.shareUser, curPlayerPage]);

  return (
    <div
      className={styles.playerLayout}
      style={{
        display: isLocalUserShared ? 'none' : 'flex',
        width: '100%',
        height: '100%',
      }}
      ref={layoutRef}
    >
      <div
        ref={playersRef}
        className={
          isRemoteUserShared
            ? styles.remoteShareVideoPlayers
            : `${curPlayerPage > 0 ? styles.noLocalUser : styles.hasLocalUser}`
        }
      >
        {renderUsers.map((user) => {
          return <VideoPlayer user={user} key={user.userId} />;
        })}
      </div>
    </div>
  );
}

export default PlayerLayout;
