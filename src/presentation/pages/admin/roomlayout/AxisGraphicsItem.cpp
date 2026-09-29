#include "AxisGraphicsItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>

AxisGraphicsItem::AxisGraphicsItem(const AxisRecord &axis, QGraphicsItem *parent)
    : QGraphicsLineItem(QLineF(axis.start, axis.end), parent)
    , record(axis)
{
    setZValue(1.0);
    setEditable(false);
    if (axis.direction == AxisDirection::Horizontal)
        setToolTip(QStringLiteral("H Axis: %1").arg(axis.label));
    else
        setToolTip(QStringLiteral("V Axis: %1").arg(axis.label));
}

QString AxisGraphicsItem::axisId() const { return record.id; }

void AxisGraphicsItem::setEditable(bool editable)
{
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    setAcceptedMouseButtons(editable ? Qt::LeftButton : Qt::NoButton);
    if (!editable) setSelected(false);
}

QPainterPath AxisGraphicsItem::shape() const
{
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    QPainterPathStroker stroker;
    stroker.setWidth(16.0);
    return stroker.createStroke(path);
}

void AxisGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    const bool sel = option->state.testFlag(QStyle::State_Selected);

    const QColor lineCol = sel ? QColor(0, 160, 115) : QColor(92, 126, 151);

    QPen dashedPen(lineCol, sel ? 1.5 : 0.8, Qt::CustomDashLine, Qt::FlatCap);
    dashedPen.setCosmetic(true);
    dashedPen.setDashPattern({8.0, 4.0, 2.0, 4.0});

    painter->setPen(dashedPen);
    painter->drawLine(line());
    
    painter->setPen(lineCol);
    QFont f; f.setPointSize(9); f.setBold(false);
    painter->setFont(f);
    
    QPointF txtPos = line().p1();
    if (record.direction == AxisDirection::Horizontal) {
        txtPos += QPointF(-10, -10);
    } else {
        txtPos += QPointF(10, -10);
    }
    painter->drawText(txtPos, record.label);
}
