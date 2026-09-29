#include "GripItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

GripItem::GripItem(const QString &ownerId, GripOwnerType ownerType, GripRole role, const QPointF &otherEndpoint, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_ownerId(ownerId), m_ownerType(ownerType), m_role(role), m_other(otherEndpoint)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptedMouseButtons(Qt::NoButton);
    setZValue(50.0);
}

QRectF GripItem::boundingRect() const
{
    return QRectF(-5.0, -5.0, 10.0, 10.0);
}

QPainterPath GripItem::shape() const
{
    QPainterPath p;
    p.addRect(boundingRect());
    return p;
}

void GripItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(0, 140, 255), 1.5));
    painter->setBrush(QColor(255, 255, 255, 230));
    painter->drawRect(QRectF(-4.5, -4.5, 9.0, 9.0));
}
