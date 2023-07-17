// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import static android.view.View.GONE;
import static com.volcengine.vertcdemo.videocall.core.Constants.MEDIA_TYPE_AUDIO;

import android.content.Context;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.TextView;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDialog;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.ss.bytertc.engine.type.LocalAudioStats;
import com.ss.bytertc.engine.type.LocalStreamStats;
import com.ss.bytertc.engine.type.LocalVideoStats;
import com.ss.bytertc.engine.type.NetworkQuality;
import com.ss.bytertc.engine.type.RemoteAudioStats;
import com.ss.bytertc.engine.type.RemoteStreamStats;
import com.ss.bytertc.engine.type.RemoteVideoStats;
import com.volcengine.vertcdemo.utils.AppUtil;
import com.volcengine.vertcdemo.utils.Utils;
import com.volcengine.vertcdemo.common.WindowUtils;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.core.Constants;
import com.volcengine.vertcdemo.videocall.core.VideoCallDataManager;
import com.volcengine.vertcdemo.videocall.databinding.DialogMediaStatsBinding;
import com.volcengine.vertcdemo.videocall.event.LocalStreamStatsEvent;
import com.volcengine.vertcdemo.videocall.event.RemoteStreamStatsEvent;
import com.volcengine.vertcdemo.videocall.event.RoomUserEvent;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * 媒体数据统计对话框
 *
 * 由一个Tab两个RecyclerView组成，用来展示视频、音频统计
 *
 * 接收RTC发过来的媒体统计事件展示对应用户的媒体数据
 */
public class MediaStatsDialog extends AppCompatDialog {

    private final DialogMediaStatsBinding mViewBinding;

    // 音频数据统计adapter
    private final StatsAdapter mAudioAdapter = new StatsAdapter(Constants.MEDIA_TYPE_AUDIO);
    // 视频数据统计adapter
    private final StatsAdapter mVideoAdapter = new StatsAdapter(Constants.MEDIA_TYPE_VIDEO);

    public MediaStatsDialog(Context context) {
        super(context, R.style.SolutionCommonDialog);
        setCancelable(true);

        mViewBinding = DialogMediaStatsBinding.inflate(getLayoutInflater());

        mViewBinding.getRoot().setOnClickListener((v) -> dismiss());
        mViewBinding.statsContent.setOnClickListener((v) -> {
        });
        mViewBinding.videoStatsTab.setOnClickListener(this::switchTab);
        mViewBinding.audioStatsTab.setOnClickListener(this::switchTab);

        mViewBinding.audioStatsRv.setLayoutManager(new LinearLayoutManager(context, RecyclerView.VERTICAL, false));
        mViewBinding.videoStatsRv.setLayoutManager(new LinearLayoutManager(context, RecyclerView.VERTICAL, false));
        mViewBinding.audioStatsRv.setAdapter(mAudioAdapter);
        mViewBinding.videoStatsRv.setAdapter(mVideoAdapter);

        switchTab(mViewBinding.videoStatsTab);
    }

    // 切换视频/音频统计tab
    private void switchTab(View selected) {
        mViewBinding.videoStatsTab.setSelected(mViewBinding.videoStatsTab == selected);
        mViewBinding.audioStatsTab.setSelected(mViewBinding.audioStatsTab == selected);
        mViewBinding.videoStatsIndicator.setSelected(mViewBinding.videoStatsTab == selected);
        mViewBinding.audioStatsIndicator.setSelected(mViewBinding.audioStatsTab == selected);
        mViewBinding.videoStatsRv.setVisibility(mViewBinding.videoStatsTab == selected ? View.VISIBLE : GONE);
        mViewBinding.audioStatsRv.setVisibility(mViewBinding.audioStatsTab == selected ? View.VISIBLE : GONE);
    }

    /**
     * 设置本地用户的统计数据
     * @param localStreamStats 统计数据
     */
    private void setLocalStats(LocalStreamStats localStreamStats) {
        mAudioAdapter.addOrUpdateLocalStat(localStreamStats);
        mVideoAdapter.addOrUpdateLocalStat(localStreamStats);
    }

    /**
     * 设置远端用户的统计数据
     * @param remoteStreamStats 统计数据
     */
    private void setRemoteStats(RemoteStreamStats remoteStreamStats) {
        mAudioAdapter.addOrUpdateRemoteStat(remoteStreamStats);
        mVideoAdapter.addOrUpdateRemoteStat(remoteStreamStats);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRoomUserEvent(RoomUserEvent event) {
        if (!event.isJoin) {
            mAudioAdapter.removeStatus(event.userInfo.userId);
            mVideoAdapter.removeStatus(event.userInfo.userId);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onLocalStreamStatsEvent(LocalStreamStatsEvent event) {
        setLocalStats(event.localStreamStats);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRemoteStreamStatsEvent(RemoteStreamStatsEvent event) {
        setRemoteStats(event.remoteStreamStats);
    }

    @Override
    public void show() {
        super.show();

        WindowManager.LayoutParams params = getWindow().getAttributes();
        params.width = WindowUtils.getScreenWidth(getContext());
        params.height = ViewGroup.LayoutParams.MATCH_PARENT;
        getWindow().setAttributes(params);
        getWindow().setContentView(mViewBinding.getRoot(), new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        getWindow().setGravity(Gravity.BOTTOM);

        SolutionDemoEventManager.register(this);
    }

    @Override
    public void dismiss() {
        super.dismiss();
        SolutionDemoEventManager.unregister(this);
    }

    /**
     * 统计的adapter
     * item类型以自己和别人、视频和音频分类四类
     */
    private static class StatsAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

        @IntDef({VIEW_TYPE_LOCAL_AUDIO, VIEW_TYPE_REMOTE_AUDIO,
                VIEW_TYPE_LOCAL_VIDEO, VIEW_TYPE_REMOTE_VIDEO})
        @Retention(RetentionPolicy.SOURCE)
        public @interface ViewType {
        }

        public static final int VIEW_TYPE_LOCAL_AUDIO = 0;
        public static final int VIEW_TYPE_REMOTE_AUDIO = 1;
        public static final int VIEW_TYPE_LOCAL_VIDEO = 2;
        public static final int VIEW_TYPE_REMOTE_VIDEO = 3;

        // 统计数据的媒体类型
        private final @Constants.MediaType
        int mMediaType;

        // 本地用户的统计数据
        private LocalStreamStats mLocalStreamStats;
        // 远端用户的统计数据
        private final List<RemoteStreamStats> mRemoteStreamStatList = new ArrayList<>();

        public StatsAdapter(@Constants.MediaType int mediaType) {
            mMediaType = mediaType;
        }

        /**
         * 增加或更新本地用户的统计数据，本地数据固定在第一位
         * @param stats 统计数据
         */
        public void addOrUpdateLocalStat(LocalStreamStats stats) {
            mLocalStreamStats = stats;
            notifyItemChanged(0);
        }

        /**
         * 增加或更新远端用户的统计数据，远端数据view中的下标与list中的下标偏移为1
         * @param stats 统计数据
         */
        public void addOrUpdateRemoteStat(RemoteStreamStats stats) {
            boolean contains = false;
            for (int i = 0; i < mRemoteStreamStatList.size(); i++) {
                if (TextUtils.equals(stats.uid, mRemoteStreamStatList.get(i).uid)) {
                    contains = true;
                    mRemoteStreamStatList.set(i, stats);
                    notifyItemChanged(i + 1);
                }
            }
            if (!contains) {
                mRemoteStreamStatList.add(stats);
                notifyItemInserted(mRemoteStreamStatList.size());
            }
        }

        /**
         * 移除用户的统计数据
         * @param uid 用户id
         */
        public void removeStatus(String uid) {
            if (TextUtils.isEmpty(uid)) {
                return;
            }
            for (int i = mRemoteStreamStatList.size() - 1; i >= 0; i--) {
                if (TextUtils.equals(uid, mRemoteStreamStatList.get(i).uid)) {
                    mRemoteStreamStatList.remove(i);
                    notifyItemRemoved(i + 1);
                }
            }
        }

        @NonNull
        @Override
        public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            int resId;
            if (viewType == VIEW_TYPE_LOCAL_AUDIO) {
                resId = R.layout.item_dialog_local_stat_audio;
            } else if (viewType == VIEW_TYPE_LOCAL_VIDEO) {
                resId = R.layout.item_dialog_local_stat_video;
            } else if (viewType == VIEW_TYPE_REMOTE_AUDIO) {
                resId = R.layout.item_dialog_remote_stat_audio;
            } else {
                resId = R.layout.item_dialog_remote_stat_video;
            }
            View view = LayoutInflater.from(parent.getContext()).inflate(resId, parent, false);
            if (viewType == VIEW_TYPE_LOCAL_AUDIO) {
                return new LocalAudioStatsViewHolder(view);
            } else if (viewType == VIEW_TYPE_LOCAL_VIDEO) {
                return new LocalVideoStatsViewHolder(view);
            } else if (viewType == VIEW_TYPE_REMOTE_AUDIO) {
                return new RemoteAudioStatsViewHolder(view);
            } else {
                return new RemoteVideoStatsViewHolder(view);
            }
        }

        @Override
        public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
            LocalStreamStats localStreamStats = mLocalStreamStats;
            if (holder instanceof LocalAudioStatsViewHolder) {
                ((LocalAudioStatsViewHolder) holder).bindLocalStat(localStreamStats);
            } else if (holder instanceof LocalVideoStatsViewHolder) {
                ((LocalVideoStatsViewHolder) holder).bindLocalStat(localStreamStats);
            }

            if (position < 1) {
                return;
            }
            RemoteStreamStats remoteStreamStats = mRemoteStreamStatList.get(position - 1);
            if (holder instanceof RemoteAudioStatsViewHolder) {
                ((RemoteAudioStatsViewHolder) holder).bindRemoteStat(remoteStreamStats);
            } else if (holder instanceof RemoteVideoStatsViewHolder) {
                ((RemoteVideoStatsViewHolder) holder).bindRemoteStat(remoteStreamStats);
            }
        }

        @Override
        public @ViewType
        int getItemViewType(int position) {
            if (position == 0) {
                return mMediaType == MEDIA_TYPE_AUDIO ? VIEW_TYPE_LOCAL_AUDIO : VIEW_TYPE_LOCAL_VIDEO;
            } else {
                return mMediaType == MEDIA_TYPE_AUDIO ? VIEW_TYPE_REMOTE_AUDIO : VIEW_TYPE_REMOTE_VIDEO;
            }
        }

        @Override
        public int getItemCount() {
            int localCount = mLocalStreamStats == null ? 0 : 1;
            return (mRemoteStreamStatList.size() + localCount);
        }
    }

    private static class LocalVideoStatsViewHolder extends RecyclerView.ViewHolder {

        private final TextView mUserName;
        private final TextView mResolution;
        private final TextView mLossRate;
        private final TextView mFrameRate;
        private final TextView mBitrate;
        private final TextView mQuality;
        private final TextView mRtt;

        public LocalVideoStatsViewHolder(@NonNull View itemView) {
            super(itemView);
            mUserName = itemView.findViewById(R.id.stats_user_name);
            mResolution = itemView.findViewById(R.id.stats_resolution_value);
            mLossRate = itemView.findViewById(R.id.stats_loss_value);
            mFrameRate = itemView.findViewById(R.id.stats_frame_rate_value);
            mBitrate = itemView.findViewById(R.id.stats_bitrate_value);
            mQuality = itemView.findViewById(R.id.stats_quality_value);
            mRtt = itemView.findViewById(R.id.stats_delay_value);
        }

        public void bindLocalStat(LocalStreamStats localStats) {
            mUserName.setText(SolutionDataManager.ins().getUserName());
            if (localStats == null || localStats.videoStats == null) {
                mResolution.setText("0-0");
                mLossRate.setText("0(%)");
                mFrameRate.setText("0(fps)");
                mBitrate.setText("0(kb/s)");
                mQuality.setText(getNetworkStrByCode(NetworkQuality.NETWORK_QUALITY_UNKNOWN));
                mRtt.setText("0(ms)");
                return;
            }
            LocalVideoStats localVideoStats = localStats.videoStats;
            mResolution.setText(String.format(Locale.ENGLISH, "%d*%d",
                    localVideoStats.encodedFrameWidth, localVideoStats.encodedFrameHeight));
            mLossRate.setText(String.format(Locale.ENGLISH, "%d(%%)",
                    (int) (localVideoStats.videoLossRate * 100)));
            mFrameRate.setText(String.format(Locale.ENGLISH, "%d(fps)",
                    localVideoStats.rendererOutputFrameRate));
            mBitrate.setText(String.format(Locale.ENGLISH, "%d(kb/s)",
                    (int) (localVideoStats.sentKBitrate)));
            mQuality.setText(getNetworkStrByCode(localStats.txQuality));
            mRtt.setText(String.format(Locale.ENGLISH, "%d(ms)", localVideoStats.rtt));
        }
    }

    private static class LocalAudioStatsViewHolder extends RecyclerView.ViewHolder {

        private final TextView mUserName;
        private final TextView mLossRate;
        private final TextView mBitrate;
        private final TextView mQuality;
        private final TextView mRtt;

        public LocalAudioStatsViewHolder(@NonNull View itemView) {
            super(itemView);
            mUserName = itemView.findViewById(R.id.stats_user_name);
            mLossRate = itemView.findViewById(R.id.stats_loss_value);
            mBitrate = itemView.findViewById(R.id.stats_bitrate_value);
            mQuality = itemView.findViewById(R.id.stats_quality_value);
            mRtt = itemView.findViewById(R.id.stats_delay_value);
        }

        public void bindLocalStat(LocalStreamStats localStats) {
            mUserName.setText(SolutionDataManager.ins().getUserName());
            if (localStats == null || localStats.audioStats == null) {
                mLossRate.setText("0(%)");
                mBitrate.setText("0(kb/s)");
                mQuality.setText(getNetworkStrByCode(NetworkQuality.NETWORK_QUALITY_UNKNOWN));
                mRtt.setText("0(ms)");
                return;
            }
            LocalAudioStats localAudioStats = localStats.audioStats;
            mLossRate.setText(String.format(Locale.ENGLISH, "%d(%%)", (int) (localAudioStats.audioLossRate * 100)));
            mBitrate.setText(String.format(Locale.ENGLISH, "%d(kb/s)", (int) (localAudioStats.sendKBitrate)));
            mQuality.setText(getNetworkStrByCode(localStats.txQuality));
            mRtt.setText(String.format(Locale.ENGLISH, "%d(ms)", localAudioStats.rtt));
        }
    }

    private static class RemoteVideoStatsViewHolder extends RecyclerView.ViewHolder {

        private final TextView mUserName;
        private final TextView mResolution;
        private final TextView mLossRate;
        private final TextView mFrameRate;
        private final TextView mBitrate;
        private final TextView mQuality;
        private final TextView mRtt;

        public RemoteVideoStatsViewHolder(@NonNull View itemView) {
            super(itemView);
            mUserName = itemView.findViewById(R.id.stats_user_name);
            mResolution = itemView.findViewById(R.id.stats_resolution_value);
            mLossRate = itemView.findViewById(R.id.stats_loss_value);
            mFrameRate = itemView.findViewById(R.id.stats_frame_rate_value);
            mBitrate = itemView.findViewById(R.id.stats_bitrate_value);
            mQuality = itemView.findViewById(R.id.stats_quality_value);
            mRtt = itemView.findViewById(R.id.stats_delay_value);
        }

        public void bindRemoteStat(RemoteStreamStats remoteStats) {
            if (remoteStats == null || remoteStats.videoStats == null) {
                mUserName.setText("");
                mResolution.setText("0-0");
                mLossRate.setText("0(%)");
                mFrameRate.setText("0(fps)");
                mBitrate.setText("0(kb/s)");
                mQuality.setText(getNetworkStrByCode(NetworkQuality.NETWORK_QUALITY_UNKNOWN));
                mRtt.setText("0(ms)");
                return;
            }
            RemoteVideoStats remoteVideoStats = remoteStats.videoStats;
            mUserName.setText(VideoCallDataManager.ins().getUserNameByUserId(remoteStats.uid));
            mResolution.setText(String.format(Locale.ENGLISH, "%d*%d",
                    remoteVideoStats.width, remoteVideoStats.height));
            mLossRate.setText(String.format(Locale.ENGLISH, "%d(%%)", (int) (remoteVideoStats.videoLossRate * 100)));
            mFrameRate.setText(String.format(Locale.ENGLISH, "%d(fps)", remoteVideoStats.rendererOutputFrameRate));
            mBitrate.setText(String.format(Locale.ENGLISH, "%d(kb/s)", (int) remoteVideoStats.receivedKBitrate));
            mQuality.setText(getNetworkStrByCode(remoteStats.rxQuality));
            mRtt.setText(String.format(Locale.ENGLISH, "%d(ms)", remoteVideoStats.rtt));
        }
    }

    private static class RemoteAudioStatsViewHolder extends RecyclerView.ViewHolder {

        private final TextView mUserName;
        private final TextView mLossRate;
        private final TextView mBitrate;
        private final TextView mQuality;
        private final TextView mRtt;

        public RemoteAudioStatsViewHolder(@NonNull View itemView) {
            super(itemView);
            mUserName = itemView.findViewById(R.id.stats_user_name);
            mLossRate = itemView.findViewById(R.id.stats_loss_value);
            mBitrate = itemView.findViewById(R.id.stats_bitrate_value);
            mQuality = itemView.findViewById(R.id.stats_quality_value);
            mRtt = itemView.findViewById(R.id.stats_delay_value);
        }

        public void bindRemoteStat(@Nullable RemoteStreamStats remoteStats) {
            if (remoteStats == null || remoteStats.audioStats == null) {
                mUserName.setText("");
                mLossRate.setText("0(%)");
                mBitrate.setText("0(kb/s)");
                mQuality.setText(getNetworkStrByCode(NetworkQuality.NETWORK_QUALITY_UNKNOWN));
                mRtt.setText("0(ms)");
                return;
            }
            RemoteAudioStats remoteAudioStats = remoteStats.audioStats;
            mUserName.setText(VideoCallDataManager.ins().getUserNameByUserId(remoteStats.uid));
            mLossRate.setText(String.format(Locale.ENGLISH, "%d(%%)", (int) (remoteAudioStats.audioLossRate * 100)));
            mBitrate.setText(String.format(Locale.ENGLISH, "%d(kb/s)", (int) (remoteAudioStats.receivedKBitrate)));
            mQuality.setText((getNetworkStrByCode(remoteAudioStats.quality)));
            mRtt.setText(String.format(Locale.ENGLISH, "%d(ms)", remoteAudioStats.rtt));
        }
    }

    /**
     * 将网络状态映射到对应文案
     *
     * @param code 网络状态码
     * @return 文案
     */
    public static String getNetworkStrByCode(int code) {
        int resId;
        if (code == NetworkQuality.NETWORK_QUALITY_EXCELLENT) {
            resId = R.string.excellent;
        } else if (code == NetworkQuality.NETWORK_QUALITY_GOOD) {
            resId = R.string.good;
        } else if (code == NetworkQuality.NETWORK_QUALITY_POOR) {
            resId = R.string.good;
        } else if (code == NetworkQuality.NETWORK_QUALITY_BAD) {
            resId = R.string.stuck_stopped;
        } else if (code == NetworkQuality.NETWORK_QUALITY_VERY_BAD) {
            resId = R.string.stuck_stopped;
        } else {
            resId = R.string.stuck_stopped;
        }
        return AppUtil.getApplicationContext().getString(resId);
    }
}
