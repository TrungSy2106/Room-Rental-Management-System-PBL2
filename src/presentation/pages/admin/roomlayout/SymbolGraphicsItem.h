#ifndef SYMBOLGRAPHICSITEM_H
#define SYMBOLGRAPHICSITEM_H

#include "FloorPlanDocument.h"

#include <QGraphicsItem>

class SymbolGraphicsItem : public QGraphicsItem
{
public:
    explicit SymbolGraphicsItem(const SymbolRecord &sym, QGraphicsItem *parent = nullptr);

    QString    symbolId()  const;
    SymbolType symbolType() const;
    void       setEditable(bool editable);

    QRectF       boundingRect() const override;
    QPainterPath shape()      const override;
    void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    void paintDoor(QPainter *painter, bool selected) const;
    void paintStair(QPainter *painter, bool selected) const;

    SymbolRecord sym;
};

#endif
