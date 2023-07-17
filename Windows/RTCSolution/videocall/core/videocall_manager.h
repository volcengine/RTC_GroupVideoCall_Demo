#pragma once
#include <QObject>
#include <QCoreApplication>
#include <QEvent>
#include <QThread>
#include <QPointer>

#include <memory>
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/videocall_model.h"
#include "videocall/core/videocall_video_widget.h"

class VideoCallLoginWidget;
class VideoCallShareWidget;
class VideoCallMainPage;
class VideoCallSetting;
class ShareButtonBar;
class VideoCallData;

namespace videocall {
    static constexpr int kMaxShowWidgetNum = 12;

/** {zh}
 * 场景页面管理类
 * 负责不同UI界面之间的跳转和公用数据的更新
 * 处理RTC侧回调来的业务逻辑
 */

/** {en}
* Scene page management class
* Responsible for jumping between different UI interfaces and updating public data
* Handle the business logic from the RTC side callback
*/
class VideoCallManager : public QObject {
    Q_OBJECT

public:
    VideoCallManager() = default;
    ~VideoCallManager() = default;

    static VideoCallManager& instance();
    static void init();
    static void initTranslations();
    static void showLogin(QWidget* parent = nullptr);
    static void showTips(QWidget* parent = nullptr);
    static void showSetting(QWidget* parent = nullptr);
    static void showRealTimeData(QWidget* parent = nullptr);
    static void showShareWidget(QWidget* parent = nullptr);
    static void showShareControlBar();

    static int showCallExpDlg(QWidget* parent = nullptr);
    static void setLocalVideoWidget(const videocall::User& user, int idx);
    static void setRemoteVideoWidget(const videocall::User& user, int idx);
    static void setRemoteScreenVideoWidget(const videocall::User& user);

    static void initRoom();
    static void showRoom();
    static QWidget* currentWidget();
    static void hideRoom();
    static std::vector<std::shared_ptr<VideoCallVideoWidget>> getVideoList();
    static std::shared_ptr<VideoCallVideoWidget> getCurrentVideo();
    static std::shared_ptr<VideoCallVideoWidget> getScreenVideo();
    static void updateData();
    static void videoCallNotify();
    static void stopScreen();

protected:
    void customEvent(QEvent*) override;

signals:
    void sigReturnMainPage();

private:
    std::unique_ptr<VideoCallLoginWidget> login_widget_;
    std::unique_ptr<VideoCallSetting> setting_page_;
    std::unique_ptr<VideoCallShareWidget> share_widget_;
    std::unique_ptr<ShareButtonBar> share_button_bar_;
    std::unique_ptr<VideoCallMainPage> main_page_;
    std::vector<std::shared_ptr<VideoCallVideoWidget>> videos_;
    std::shared_ptr<VideoCallVideoWidget> screen_widget_;
    QPointer<VideoCallData> data_page_;
    QWidget* current_widget_ = nullptr;
    bool updating = false;
};

}  // namespace videocall
