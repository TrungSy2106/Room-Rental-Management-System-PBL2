#ifndef TEXTANNOTATIONITEM_H
#define TEXTANNOTATIONITEM_H

#include "FloorPlanDocument.h"

#include <QGraphicsItem>

class TextAnnotationItem : public QGraphicsItem
{
public:
    explicit TextAnnotationItem(const TextRecord &txt, QGraphicsItem *parent = nullptr);

    QString textId()  const;
    void    setEditable(bool editable);

    QRectF boundingRect() const override;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    TextRecord txt;
    QRectF     cachedRect;
};

#endif
