#include "videocall_notify.h"
#include "videocall/core/videocall_session.h"

VideoCallNotify& VideoCallNotify::instance() {
    static VideoCallNotify notify;
    return notify;
}

void VideoCallNotify::init() { 
    instance()._init(); 
}

VideoCallNotify::VideoCallNotify() {

}

void VideoCallNotify::_init() {
    vrd::VideoCallSession::instance().onCallEnd([=](int code) {
        if (onCallEnd_) onCallEnd_(code);
    });
}


void VideoCallNotify::onCallEnd(std::function<void(int)>&& callback) {
    onCallEnd_ = std::move(callback);
}

void VideoCallNotify::offAll() {
    onCallEnd_ = nullptr;
}