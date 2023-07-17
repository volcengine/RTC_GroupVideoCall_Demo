#include "videocall_session.h"
#include "core/Application.h"
#include "feature/data_mgr.h"
#include "core/util_tip.h"
#include "videocall/core/data_mgr.h"
#include "logger.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QDebug>

namespace vrd {

VideoCallSession& VideoCallSession::instance() {
	static VideoCallSession video_session;
	return video_session;
}

void VideoCallSession::setUserId(const std::string& uid) {
	base_->setUserId(uid);
}

void VideoCallSession::setToken(const std::string& token) {
	base_->setToken(token);
}

void VideoCallSession::setRoomId(const std::string& roomId) {
	base_->setRoomId(roomId);
}

std::string VideoCallSession::user_id() { 
	return base_->_userId(); 
}

void VideoCallSession::changeUserName(CSTRING_REF_PARAM name,
                                    CallBackFunction&& callback) {
	base_->changeUserName(name, std::move(callback));
}

void VideoCallSession::initSceneConfig(std::function<void(void)>&& callback) {
    base_->connectRTS("videocall", [this, callback]() {
        auto res_info = vrd::DataMgr::instance().rts_info();
        videocall::DataMgr::instance().setAppID(res_info.app_id);
        if (callback) {
            callback();
        }
    });
}

void VideoCallSession::exitScene() {
	base_->disconnectRTS();
}

void VideoCallSession::joinCall( const std::string& userId,
                                 const std::string& roomId, 
                                 std::function<void(int)>&& callback) {
	QJsonObject req;
	req["login_token"] = QString::fromStdString(base_->_token());
	req["user_id"] = QString::fromStdString(userId);
	req["room_id"] = QString::fromStdString(roomId);

	base_->_emitMessage("videocallJoinRoom", req, [=](const QJsonObject& rsp) {
        int code = rsp["code"].toInt();
        if (code != 200) {
            if (code == 406) {
                vrd::util::showToastInfo(QObject::tr("network_messsage_406").toStdString());
            }
            else {
                auto errorMessage = std::string(rsp["message"].toString().toUtf8());
                vrd::util::showToastInfo(errorMessage);
            }

            if (callback) {
                callback(code);
            }
            return;
        }
        videocall::DataMgr::instance().setRoomID(roomId);

        videocall::VideoCallRoom room;
        room.room_id = roomId;
        auto& response = rsp["response"].toObject();
        room.duration = response["duration"].toDouble();
        videocall::DataMgr::instance().setRoom(std::move(room));
        auto token = std::string(response["rtc_token"].toString().toUtf8());
        videocall::DataMgr::instance().setToken(token);

        std::vector<videocall::User> users;
        videocall::User self;
        self.is_camera_on = !videocall::DataMgr::instance().mute_video();
        self.is_mic_on = !videocall::DataMgr::instance().mute_audio();
        self.is_sharing = false;
        self.created_at = room.duration;
        self.user_id = videocall::DataMgr::instance().user_id();
        self.user_name = videocall::DataMgr::instance().user_name();
        users.push_back(std::move(self));
        videocall::DataMgr::instance().setUsers(std::move(users));

        if (callback) {
            callback(code);
        }
    });
}

void VideoCallSession::leaveCall(std::function<void(int)>&& callback) {
	QJsonObject req;
	req["login_token"] = QString::fromStdString(base_->_token());
	req["user_id"] = QString::fromStdString(videocall::DataMgr::instance().user_id());
    req["room_id"] = QString::fromStdString(videocall::DataMgr::instance().room_id());

	base_->_emitMessage("videocallLeaveRoom", req, [=](const QJsonObject& rsp) {
		int code = rsp["code"].toInt();
		if (code != 200) {
			callback(code);
			return;
		}
		callback(code);
	});
}

void VideoCallSession::startScreenShare(std::function<void(int)> callback) {
    QJsonObject req;
    req["login_token"] = QString::fromStdString(base_->_token());
    req["user_id"] = QString::fromStdString(videocall::DataMgr::instance().user_id());
    req["room_id"] = QString::fromStdString(videocall::DataMgr::instance().room_id());
    base_->_emitMessage("videocallStartShareScreen", req,
        [=](const QJsonObject& rsp) {
            int code = rsp["code"].toInt();
            if (code != 200) {
                callback(code);
                return;
            }
            callback(code);
        });
}

void VideoCallSession::stopScreenShare(std::function<void(int)> callback) {
	QJsonObject req;
	req["login_token"] = QString::fromStdString(base_->_token());
    req["user_id"] = QString::fromStdString(videocall::DataMgr::instance().user_id());
    req["room_id"] = QString::fromStdString(videocall::DataMgr::instance().room_id());
	base_->_emitMessage("videocallEndShareScreen", req,
		[=](const QJsonObject& rsp) {
			int code = rsp["code"].toInt();
			if (code != 200) {
				callback(code);
				return;
			}
			callback(code);
		});
}

void VideoCallSession::userReconnect(std::function<void(int)> callback) {
	QJsonObject req;
    req["login_token"] = QString::fromStdString(base_->_token());
    req["user_id"] = QString::fromStdString(videocall::DataMgr::instance().user_id());
    req["room_id"] = QString::fromStdString(videocall::DataMgr::instance().room_id());

	base_->_emitMessage("videocallReconnect", req, [=](const QJsonObject& rsp) {
		int code = rsp["code"].toInt();
		if (code == 200) {
            auto& response = rsp["response"].toObject();
            auto token = std::string(response["rtc_token"].toString().toUtf8());
            videocall::DataMgr::instance().setToken(token);
		}
		callback(code);
	});
}

void VideoCallSession::cleanUser(const std::string& userId,
									std::function<void(int)> callback) {
    QJsonObject req;
    req["login_token"] = QString::fromStdString(base_->_token());
    req["user_id"] = QString::fromStdString(videocall::DataMgr::instance().user_id());

    base_->_emitMessage("videocallClearUser", req, [=](const QJsonObject& rsp) {
        int code = rsp["code"].toInt();
        if (code != 200) {
            callback(code);
            return;
        }
        callback(code);
    });
}

void VideoCallSession::onCallEnd(std::function<void(int)>&& callback) {
	base_->_onNotify("videocallOnCloseRoom", [=](const QJsonObject& data) {
        auto roomId = std::string(data["room_id"].toString().toUtf8());

		if (callback && roomId == videocall::DataMgr::instance().room_id()) {
			callback(200);
		}
	});
}

VideoCallSession::VideoCallSession() {
  base_ = vrd::Application::getSingleton().getComponent(
      VRD_UTIL_GET_COMPONENT_PARAM(vrd::SessionBase));
}

}  // namespace vrd
