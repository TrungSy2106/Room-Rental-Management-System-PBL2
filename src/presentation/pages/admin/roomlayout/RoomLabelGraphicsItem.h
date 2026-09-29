#ifndef ROOMLABELGRAPHICSITEM_H
#define ROOMLABELGRAPHICSITEM_H

#include "FloorPlanDocument.h"

#include <QColor>
#include <QGraphicsItem>

class RoomLabelGraphicsItem : public QGraphicsItem
{
public:
    RoomLabelGraphicsItem(const RoomLabelRecord &label, QString roomType, int status, QGraphicsItem *parent = nullptr);

    QString roomId() const;
    void setEditable(bool editable);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QColor statusColor() const;
    QString statusText() const;

    RoomLabelRecord label;
    QString roomType;
    int status;
};

#endif
