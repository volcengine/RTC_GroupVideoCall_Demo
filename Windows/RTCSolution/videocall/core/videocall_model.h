#pragma once
#include <unordered_map>
#include <vector>
#include <string>

namespace videocall {
    struct VideoResolution {
        int width = 640;
        int height = 360;
    };

    enum AudioQuality {
        kAudioQualityFluent = 0,
        kAudioQualityStandard = 1,
        kAudioQualityHD = 2,
    };

    struct VideoConfiger {
        VideoResolution resolution;
        int fps = 15;
        int kbps = 64;
    };

    struct VideoCallSettingModel {
        VideoConfiger camera{ {1280, 720}, 15, 500 };
        AudioQuality audio_quality{ kAudioQualityStandard };
        bool enable_camera_mirror = true;
    };

    struct User {
        std::string user_id;
        std::string user_name;
        // {zh} 加入通话的时间
        // {en} UTC/GMT join call time
        int64_t created_at{ 0 };
        bool is_sharing{ false };
        bool is_mic_on{ false };
        bool is_camera_on{ false };
        int audio_volume{ 0 };
    };

    struct VideoCallRoom {
      std::string room_id;
      std::string screen_shared_uid;
      // {zh} 通话持续时间
      // {en} UTC/GMT, call duration time
      int64_t duration;
    };

    struct StreamInfo {
        std::string user_id;
        std::string user_name;
        // {zh} 分辨率的宽度值
        // {en} Resolution width value
        int width;
        // {zh} 分辨率的高度值
        // {en} Resolution height value
        int height;
        // {zh} 视频帧率
        // {en} video frame rate
        int video_fps;
        // {zh} 视频码率
        // {en} Video bit rate
        int video_kbitrate;
        // {zh} 音频码率
        // {en} audio bit rate
        int audio_kbitrate;
        // {zh} 音频时延
        // {en} audio delay
        int audio_delay;
        // {zh} 视频时延
        // {en} video delay
        int video_delay;
        // {zh} 音频丢包率
        // {en} audio packet loss rate
        float audio_loss_rate;
        // {zh} 视频丢包率
        // {en} Video packet loss rate
        float video_loss_rate;
        // {zh} 网络质量
        // {en} network quality
        int natwork_quality;
    };
}


