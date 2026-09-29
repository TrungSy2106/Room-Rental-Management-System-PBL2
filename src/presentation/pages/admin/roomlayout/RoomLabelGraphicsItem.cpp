#include "RoomLabelGraphicsItem.h"

#include <QFont>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <utility>

RoomLabelGraphicsItem::RoomLabelGraphicsItem(const RoomLabelRecord &label, QString roomType, int status, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , label(label)
    , roomType(std::move(roomType))
    , status(status)
{
    setPos(label.position);
    setAcceptedMouseButtons(Qt::NoButton);
    setToolTip(QStringLiteral("%1 | %2 | %3")
                   .arg(label.roomId, this->roomType, statusText()));
    setZValue(3.0);
}

QString RoomLabelGraphicsItem::roomId() const
{
    return label.roomId;
}

void RoomLabelGraphicsItem::setEditable(bool editable)
{
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    setAcceptedMouseButtons(editable ? Qt::LeftButton : Qt::NoButton);
    if (!editable) {
        setSelected(false);
    }
}

QRectF RoomLabelGraphicsItem::boundingRect() const
{
    return QRectF(-53.0, -42.0, 106.0, 84.0);
}

void RoomLabelGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    const QRectF marker(-48.0, -37.0, 96.0, 74.0);
    const bool selected = option->state.testFlag(QStyle::State_Selected);
    painter->setRenderHint(QPainter::Antialiasing);

    if (selected) {
        painter->setPen(QPen(QColor(0, 190, 140), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(marker.adjusted(-3, -3, 3, 3), 7.0, 7.0);
    }

    painter->setPen(QPen(QColor(190, 194, 196), 1.0));
    painter->setBrush(QColor(252, 252, 250));
    painter->drawRoundedRect(marker, 5.0, 5.0);
    painter->setPen(Qt::NoPen);
    painter->setBrush(statusColor());
    painter->drawEllipse(QPointF(0.0, marker.top() + 13.0), 5.0, 5.0);

    QFont roomFont = painter->font();
    roomFont.setBold(true);
    roomFont.setPointSize(11);
    painter->setFont(roomFont);
    painter->setPen(QColor(32, 38, 44));
    painter->drawText(QRectF(marker.left() + 6.0,
                             marker.top() + 22.0,
                             marker.width() - 12.0,
                             22.0),
                      Qt::AlignCenter,
                      label.roomId);

    QFont detailFont = painter->font();
    detailFont.setBold(false);
    detailFont.setPointSize(8);
    painter->setFont(detailFont);
    painter->setPen(QColor(83, 91, 99));
    painter->drawText(QRectF(marker.left() + 6.0,
                             marker.top() + 45.0,
                             marker.width() - 12.0,
                             18.0),
                      Qt::AlignCenter,
                      roomType);
}

QColor RoomLabelGraphicsItem::statusColor() const
{
    switch (status) {
    case 0:
        return QColor(49, 190, 126);
    case 1:
        return QColor(231, 76, 72);
    case 2:
        return QColor(245, 166, 35);
    case 3:
        return QColor(126, 91, 180);
    default:
        return QColor(120, 133, 145);
    }
}

QString RoomLabelGraphicsItem::statusText() const
{
    switch (status) {
    case 0:
        return QStringLiteral("Available");
    case 1:
        return QStringLiteral("Occupied");
    case 2:
        return QStringLiteral("Reserved");
    case 3:
        return QStringLiteral("Maintenance");
    default:
        return QStringLiteral("Unknown");
    }
}
