#pragma once

#include <QWidget>

class QHBoxLayout;
class QSpacerItem;

namespace Ui {
class FocusVideoView;
}

/** {zh}
 * 包括共享内容的视频渲染区域类，左边是共享内容，右边是竖着排列的用户视频
 */

/** {en}
* The video rendering area class including shared content, 
* the left side is the shared content, and the right side is the user's video arranged vertically
*/
class FocusVideoView : public QWidget {
  Q_OBJECT

 public:
  explicit FocusVideoView(QWidget *parent = nullptr);
  ~FocusVideoView();
  void init();
  void showWidget(int cnt);
  void wheelEvent(QWheelEvent *) override;

 protected:
  void paintEvent(QPaintEvent *) override;

 private:
  Ui::FocusVideoView* ui;
  int cnt_ = 0;
};
