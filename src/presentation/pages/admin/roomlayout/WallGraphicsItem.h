#ifndef WALLGRAPHICSITEM_H
#define WALLGRAPHICSITEM_H

#include "FloorPlanDocument.h"

#include <QGraphicsLineItem>

class WallGraphicsItem : public QGraphicsLineItem
{
public:
    explicit WallGraphicsItem(const WallRecord &wall, QGraphicsItem *parent = nullptr);

    QString id() const;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void setEditable(bool editable);

private:
    QString wallId;
};

#endif
