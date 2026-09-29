#ifndef GRIPITEM_H
#define GRIPITEM_H

#include <QGraphicsItem>
#include <QPointF>
#include <QString>

enum class GripRole { StartPoint, EndPoint };

enum class GripOwnerType { Wall, HAxis, VAxis };

class GripItem : public QGraphicsItem
{
public:
    GripItem(const QString &ownerId, GripOwnerType ownerType, GripRole role, const QPointF &otherEndpoint, QGraphicsItem *parent = nullptr);

    QString       ownerId()        const { return m_ownerId; }
    GripOwnerType ownerType()      const { return m_ownerType; }
    GripRole      role()           const { return m_role; }
    QPointF       otherEndpoint()  const { return m_other; }

    QRectF       boundingRect() const override;
    QPainterPath shape()        const override;
    void         paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget*) override;

private:
    QString       m_ownerId;
    GripOwnerType m_ownerType;
    GripRole      m_role;
    QPointF       m_other;
};

#endif
