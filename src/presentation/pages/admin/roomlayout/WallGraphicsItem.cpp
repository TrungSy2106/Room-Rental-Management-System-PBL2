#include "WallGraphicsItem.h"

#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>

WallGraphicsItem::WallGraphicsItem(const WallRecord &wall, QGraphicsItem *parent)
    : QGraphicsLineItem(QLineF(wall.start, wall.end), parent)
    , wallId(wall.id)
{
    setZValue(5.0);
    setEditable(false);
}

QString WallGraphicsItem::id() const
{
    return wallId;
}

QPainterPath WallGraphicsItem::shape() const
{
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    QPainterPathStroker stroker;
    stroker.setWidth(16.0);
    return stroker.createStroke(path);
}

void WallGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    const bool selected = option->state.testFlag(QStyle::State_Selected);
    painter->setRenderHint(QPainter::Antialiasing);
    
    const QLineF centerLine = line();
    if (qFuzzyIsNull(centerLine.length())) {
        return;
    }

    constexpr qreal HalfWallWidth = 4.0;
    QLineF normal = centerLine.normalVector();
    normal.setLength(HalfWallWidth);
    const QPointF offset = normal.p2() - normal.p1();
    const QLineF firstEdge(centerLine.p1() + offset, centerLine.p2() + offset);
    const QLineF secondEdge(centerLine.p1() - offset, centerLine.p2() - offset);

    const QColor wallColor = selected ? QColor(0, 155, 110) : QColor(55, 67, 77);
    QPen edgePen(wallColor, selected ? 1.8 : 1.15, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    edgePen.setCosmetic(true);
    painter->setPen(edgePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(firstEdge);
    painter->drawLine(secondEdge);
    painter->drawLine(firstEdge.p1(), secondEdge.p1());
    painter->drawLine(firstEdge.p2(), secondEdge.p2());

    if (selected) {
        QPen centerPen(QColor(0, 155, 110, 150), 1.0, Qt::DashLine, Qt::FlatCap);
        centerPen.setCosmetic(true);
        painter->setPen(centerPen);
        painter->drawLine(centerLine);
    }
}

void WallGraphicsItem::setEditable(bool editable)
{
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    setAcceptedMouseButtons(editable ? Qt::LeftButton : Qt::NoButton);
    if (!editable) {
        setSelected(false);
    }
}
