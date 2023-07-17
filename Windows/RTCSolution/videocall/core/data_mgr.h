#pragma once
#include <algorithm>
#include <functional>
#include <mutex>

#include "core/rtc_engine_wrap.h"
#include "videocall_model.h"

namespace videocall {
#define PROPRETY(CLASS, MEMBER, UPPER_MEMBER)                        \
private:                                                             \
    CLASS MEMBER##_;                                                 \
                                                                      \
public:                                                              \
  CLASS MEMBER() const { return MEMBER##_; }                          \
  CLASS& ref_##MEMBER() { return MEMBER##_; }                         \
  void set##UPPER_MEMBER(const CLASS& MEMBER) { MEMBER##_ = MEMBER; } \
  void set##UPPER_MEMBER(CLASS&& MEMBER) { MEMBER##_ = std::move(MEMBER); }

/** {zh}
 * 场景需要的数据定义类，用于不同类之前的数据同步
 */

 /** {en}
  * The data definition class required by the scene, used for data synchronization between different classes
  */
class DataMgr {
public:
    static DataMgr& instance();
    static void init();

    PROPRETY(StreamInfo, local_stream_info, LocalStreamInfo)
    PROPRETY(std::vector<StreamInfo>, remote_stream_infos, RemoteStreamInfos)
    PROPRETY(VideoCallSettingModel, setting, Setting)
    PROPRETY(std::string, user_name, UserName)
    PROPRETY(bool, mute_audio, MuteAudio)
    PROPRETY(bool, mute_video, MuteVideo)
    PROPRETY(int, share_quality_index, ShareQualityIndex)
    PROPRETY(bool, share_screen, ShareScreen) //only for local share state

    PROPRETY(std::string, high_light, HighLight)
    PROPRETY(std::vector<AudioVolumeInfoWrap>, remote_volumes, RemoteVolumes)
    PROPRETY(std::vector<AudioVolumeInfoWrap>, local_volumes, LocalVolumes)

    PROPRETY(std::string, app_id, AppID)
    PROPRETY(std::string, user_id, UserID)
    PROPRETY(std::string, room_id, RoomID)

    PROPRETY(VideoCallRoom, room, Room)
    PROPRETY(std::vector<User>, users, Users)
    PROPRETY(std::string, token, Token)

protected:
    DataMgr() = default;
    ~DataMgr() = default;

public:
    mutable std::mutex _mutex;
};

#undef PROPRETY

}  // namespace videocall