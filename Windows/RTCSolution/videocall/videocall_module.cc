#include "core/application.h"
#include "core/navigator_interface.h"
#include "videocall_module.h"
#include "videocall/core/data_mgr.h"
#include "feature/data_mgr.h"
#include "scene_select_widget.h"
#include "videocall/core/videocall_manager.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/videocall_session.h"
#include "videocall/core/videocall_notify.h"

namespace vrd {
void VideoCallModule::addThis() {
    Application::getSingleton()
        .getComponent(VRD_UTIL_GET_COMPONENT_PARAM(vrd::INavigator))
        ->add("videocall",
            std::shared_ptr<IModule>((IModule*)(new VideoCallModule())));
    auto scenesList = vrd::DataMgr::instance().scenes_list();
    scenesList.push_back("videocall");
    vrd::DataMgr::instance().setScenesList(scenesList);
}

VideoCallModule::VideoCallModule() {
    videocall::VideoCallManager::init();
	connect(&videocall::VideoCallManager::instance(),
		&videocall::VideoCallManager::sigReturnMainPage, this, [=] {
			VideoCallRtcEngineWrap::unInit();
			turnToSceneSelectWidget();
		});
}

void VideoCallModule::turnToSceneSelectWidget() {
    VRD_FUNC_GET_COMPONET(vrd::INavigator)->go("scene_select");
    SceneSelectWidget::instance().show();
}

VideoCallModule::~VideoCallModule() {}

void VideoCallModule::open() {
    
    videocall::DataMgr::init();
    videocall::DataMgr::instance().setUserName(vrd::DataMgr::instance().user_name());
    videocall::DataMgr::instance().setUserID(vrd::DataMgr::instance().user_id());

    vrd::VideoCallSession::instance().initSceneConfig([]() {
        VideoCallRtcEngineWrap::init();
        videocall::VideoCallManager::showLogin();
        videocall::VideoCallManager::showTips();
        });
    VideoCallNotify::init();
}

void VideoCallModule::close() {
    VideoCallRtcEngineWrap::unInit();
	vrd::VideoCallSession::instance().exitScene();
}

void VideoCallModule::quit(bool) {
    VideoCallRtcEngineWrap::unInit();
    turnToSceneSelectWidget();
}

}  // namespace vrd
