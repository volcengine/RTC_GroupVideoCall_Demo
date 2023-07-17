#include "popup_arrow_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QVBoxLayout>

PopupArrowWidget::PopupArrowWidget(QWidget* parent)
    : QWidget(parent) {

    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    this->setLayout(main_layout);
}

void PopupArrowWidget::addCustomWidget(QWidget* widget) {
    layout()->addWidget(widget);
}

void PopupArrowWidget::setPopupPosition() {
    if (auto parentWidget = this->parentWidget()) {
        auto parentPoint = parentWidget->mapToGlobal(QPoint(0,0));
        switch (arrowPosition_) {
        case ArrowPosition::left: {
            move(QPoint(parentPoint.x() + parentWidget->width(), 
                parentPoint.y() + parentWidget->height() / 2 - height() / 2));
        }
        break;
        case ArrowPosition::right: {
            move(QPoint(parentPoint.x() - width(), parentPoint.y() + parentWidget->height() / 2 - height() / 2));
        }
        break;
        case ArrowPosition::top: {
            move(QPoint(parentPoint.x() + parentWidget->width() / 2 - width() / 2, 
                parentPoint.y() + parentWidget->height()));
        }
        break;
        case ArrowPosition::bottom: {
            move(QPoint(parentPoint.x() + parentWidget->width() / 2 - width() / 2, 
                parentPoint.y() - height() - 10));
        }
        break;
        }
    }
}

void PopupArrowWidget::setArrowPosition(ArrowPosition position) {
    arrowPosition_ = position;
}

void PopupArrowWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    const auto rect = this->rect();
    auto width = rect.width();
    auto height = rect.height();

    QPainterPath path;
    QPolygonF polygon;
    QRect centerRect;
    switch (arrowPosition_) {
    case ArrowPosition::left: {
        polygon.append(QPointF(ArrowRectWidth, (height - ArrowRectHeight) / 2));
        polygon.append(QPointF(0, (height) / 2));
        polygon.append(QPointF(ArrowRectWidth, (height) / 2 + ArrowRectHeight / 2));
        centerRect = QRect(ArrowRectWidth - 5, 0, (width - ArrowRectWidth), height);
    }
    break;
    case ArrowPosition::right: {
        polygon.append(QPointF(width - ArrowRectWidth, (height - ArrowRectHeight) / 2));
        polygon.append(QPointF(width, (height) / 2));
        polygon.append(QPointF(width - ArrowRectWidth, (height) / 2 + ArrowRectHeight / 2));
        centerRect = QRect(0, 0, (width - ArrowRectWidth), height);
    }
    break;
    case ArrowPosition::top: {
        polygon.append(QPointF((width - ArrowRectWidth) / 2, ArrowRectHeight));
        polygon.append(QPointF((width) / 2, 0));
        polygon.append(QPointF(width / 2 + ArrowRectWidth / 2, ArrowRectHeight));
        centerRect = QRect(0, ArrowRectHeight, width, height - ArrowRectHeight);
    }
    break;
    case ArrowPosition::bottom: {
        polygon.append(QPointF((width - ArrowRectWidth) / 2, height - ArrowRectHeight));
        polygon.append(QPointF((width) / 2, height));
        polygon.append(QPointF(width / 2 + ArrowRectWidth / 2, height - ArrowRectHeight));
        centerRect = QRect(0, 0, width, height - ArrowRectHeight);
    }
    }

    painter.setBrush(backgroundColor);
    painter.setPen(backgroundColor);
    path.addPolygon(polygon);
    path.addRoundedRect(centerRect, 6, 6);
    painter.setClipPath(path);
    painter.drawPolygon(polygon);
    painter.drawRoundedRect(centerRect, 6, 6);
}

void PopupArrowWidget::showEvent(QShowEvent* event) {
    emit widgetVisiblilityChanged(true);
    QWidget::showEvent(event);
}

void PopupArrowWidget::hideEvent(QHideEvent* event) {
    emit widgetVisiblilityChanged(false);
    QWidget::hideEvent(event);
}
