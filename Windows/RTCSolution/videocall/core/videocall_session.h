#pragma once

#include <memory>
#include <vector>

#include "core/session_base.h"
#include "videocall_model.h"

namespace vrd {

/** {zh}
 * 需要和服务端通信的相关接口和通知定义
 */

 /** {en}
  * Related interfaces and notification definitions that need to communicate with the server
  */
class VideoCallSession {
public:
    static VideoCallSession& instance();

    void setUserId(const std::string& uid);
    void setToken(const std::string& token);
    void setRoomId(const std::string& roomId);
    std::string user_id();

    // {zh} ----------------------------------接口----------------------------------
    // {en} ----------------------------------interface----------------------------------
    void changeUserName(CSTRING_REF_PARAM name, CallBackFunction&& callback);

    void initSceneConfig(std::function<void(void)>&& callback);
    void exitScene();

    void joinCall(const std::string& userId,
                    const std::string& roomId,
                    std::function<void(int)>&& callback);
    void leaveCall(std::function<void(int)>&& callback);
    void startScreenShare(std::function<void(int)> callback);
    void stopScreenShare(std::function<void(int)> callback);
    void userReconnect(std::function<void(int)> callback);
    void cleanUser(const std::string& userId, std::function<void(int)> callback);

    // {zh} ----------------------------------通知----------------------------------
    // {en} ----------------------------------notify----------------------------------
    void onCallEnd(std::function<void(int)>&& callback);

private:
    VideoCallSession();
    ~VideoCallSession() = default;

private:
    std::shared_ptr<SessionBase> base_;
};

}  // namespace vrd