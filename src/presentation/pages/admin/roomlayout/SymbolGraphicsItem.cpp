#include "SymbolGraphicsItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>

namespace {
constexpr qreal DoorWidth  = 28.0;
constexpr qreal StairW     = 54.0;
constexpr qreal StairH     = 70.0;
}

SymbolGraphicsItem::SymbolGraphicsItem(const SymbolRecord &sym, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , sym(sym)
{
    setPos(sym.position);
    setRotation(sym.rotation);
    setZValue(6.0);
    setEditable(false);
    setToolTip(sym.type == SymbolType::Door
                   ? QStringLiteral("Door")
                   : QStringLiteral("Stair"));
}

QString SymbolGraphicsItem::symbolId() const  { return sym.id; }
SymbolType SymbolGraphicsItem::symbolType() const { return sym.type; }

void SymbolGraphicsItem::setEditable(bool editable)
{
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    setAcceptedMouseButtons(editable ? Qt::LeftButton : Qt::NoButton);
    if (!editable) setSelected(false);
}

QRectF SymbolGraphicsItem::boundingRect() const
{
    if (sym.type == SymbolType::Door) {
        const qreal hw = DoorWidth / 2.0;
        return QRectF(-hw - 2.0, -DoorWidth - 6.0, DoorWidth + 4.0, DoorWidth + 12.0);
    }
    return QRectF(-StairW / 2.0 - 4.0, -StairH / 2.0 - 4.0, StairW + 8.0, StairH + 8.0);
}

QPainterPath SymbolGraphicsItem::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(12.0);
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void SymbolGraphicsItem::paintDoor(QPainter *painter, bool selected) const
{
    constexpr qreal DW = DoorWidth;
    constexpr qreal hw = DW / 2.0;
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#f7f9fb"));
    painter->drawRect(QRectF(-hw, -5.0, DW, 10.0));
    
    const QColor outlineCol = selected ? QColor(0, 155, 110) : QColor(70, 70, 70);
    const QColor panelCol   = selected ? QColor(0, 155, 110) : QColor(40, 70, 90);
    const QColor arcCol     = selected ? QColor(0, 155, 110) : QColor(120, 140, 150);
    
    painter->setPen(QPen(outlineCol, 1.0));
    painter->drawLine(QLineF(-hw, -4.0, -hw, 4.0));
    painter->drawLine(QLineF( hw, -4.0,  hw, 4.0));
    
    painter->setPen(QPen(panelCol, 2.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(QLineF(-hw, -4.0, -hw, -4.0 - DW));
    
    QPainterPath arcPath;
    arcPath.moveTo(hw, -4.0);
    arcPath.arcTo(QRectF(-hw - DW, -4.0 - DW, 2.0 * DW, 2.0 * DW), 0.0, 90.0);
    painter->setPen(QPen(arcCol, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(arcPath);
}

void SymbolGraphicsItem::paintStair(QPainter *painter, bool selected) const
{
    constexpr qreal hw = StairW / 2.0;
    constexpr qreal hh = StairH / 2.0;
    constexpr int   Treads = 6;

    const QColor baseColor = selected ? QColor(0, 155, 110) : QColor(50, 80, 100);
    const QColor fillColor(selected ? QColor(180, 240, 225) : QColor(226, 235, 240));
    const QPen   pen(baseColor, 1.5);

    painter->setPen(pen);
    painter->setBrush(fillColor);
    painter->drawRect(QRectF(-hw, -hh, StairW, StairH));

    for (int k = 1; k < Treads; ++k) {
        const qreal y = -hh + k * StairH / Treads;
        painter->drawLine(QLineF(-hw, y, hw, y));
    }

    const QPen arrowPen(baseColor, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(arrowPen);
    painter->setBrush(Qt::NoBrush);
    const qreal ax = 0.0;
    const qreal ayTop = -hh + 8.0;
    const qreal ayBot = hh - 8.0;
    painter->drawLine(QLineF(ax, ayBot, ax, ayTop));
    painter->drawLine(QLineF(ax, ayTop, ax - 7.0, ayTop + 10.0));
    painter->drawLine(QLineF(ax, ayTop, ax + 7.0, ayTop + 10.0));
}

void SymbolGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    const bool selected = option->state.testFlag(QStyle::State_Selected);
    if (sym.type == SymbolType::Door) {
        paintDoor(painter, selected);
    } else {
        paintStair(painter, selected);
    }
}
