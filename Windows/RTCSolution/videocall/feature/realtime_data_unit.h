#pragma once

#include <QWidget>
#include "videocall/core/data_mgr.h"

namespace Ui {
    class realTimeDataUnit;
}

/** {zh}
 * 用于通话数据页面中每个用户的音频或视频数据展示单元
 * 视频数据包括分辨率，帧率，码率，延迟，丢包率，网络状态
 * 音频数据包括码率，延迟，丢包率，网络状态
 */

/** {en}
* Audio or video data display unit for each user in the call statistics page
* Video data includes resolution, frame rate, bit rate, delay, packet loss rate, network status
* Audio data includes bit rate, delay, packet loss rate, network status
*/
class realTimeDataUnit : public QWidget {
    Q_OBJECT

public:
    explicit realTimeDataUnit(QWidget* parent = nullptr);
    ~realTimeDataUnit();
    void updateInfo(const videocall::StreamInfo& info, bool isVideoInfo);

private:
    Ui::realTimeDataUnit* ui;
};
