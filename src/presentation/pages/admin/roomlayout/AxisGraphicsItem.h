#ifndef AXISGRAPHICSITEM_H
#define AXISGRAPHICSITEM_H

#include "FloorPlanDocument.h"
#include <QGraphicsLineItem>

class AxisGraphicsItem : public QGraphicsLineItem
{
public:
    explicit AxisGraphicsItem(const AxisRecord &axis, QGraphicsItem *parent = nullptr);

    QString axisId() const;
    void    setEditable(bool editable);

    QPainterPath shape() const override;
    void         paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget*) override;

private:
    AxisRecord record;
};

#endif
