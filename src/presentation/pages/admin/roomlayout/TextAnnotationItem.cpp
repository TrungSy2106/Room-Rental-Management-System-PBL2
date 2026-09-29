#include "TextAnnotationItem.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
QFont annotationFont()
{
    QFont f;
    f.setPointSize(11);
    f.setItalic(true);
    f.setBold(false);
    return f;
}
}

TextAnnotationItem::TextAnnotationItem(const TextRecord &txt, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , txt(txt)
{
    setPos(txt.position);
    setZValue(4.5);
    setEditable(false);
    setToolTip(txt.text);

    const QFontMetricsF fm(annotationFont());
    const qreal w = fm.horizontalAdvance(txt.text) + 10.0;
    const qreal h = fm.height() + 6.0;
    cachedRect = QRectF(-w / 2.0, -h / 2.0, w, h);
}

QString TextAnnotationItem::textId() const { return txt.id; }

void TextAnnotationItem::setEditable(bool editable)
{
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    setAcceptedMouseButtons(editable ? Qt::LeftButton : Qt::NoButton);
    if (!editable) setSelected(false);
}

QRectF TextAnnotationItem::boundingRect() const
{
    return cachedRect.adjusted(-3.0, -3.0, 3.0, 3.0);
}

void TextAnnotationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    const bool selected = option->state.testFlag(QStyle::State_Selected);

    if (selected) {
        painter->setPen(QPen(QColor(0, 190, 140), 1.5, Qt::DashLine));
        painter->setBrush(QColor(0, 190, 140, 25));
        painter->drawRoundedRect(cachedRect, 4.0, 4.0);
    }

    painter->setFont(annotationFont());
    painter->setPen(selected ? QColor(0, 130, 95) : QColor(39, 99, 151));
    painter->drawText(cachedRect, Qt::AlignCenter, txt.text);

    const qreal cx = cachedRect.center().x();
    const qreal cy = cachedRect.bottom() + 2.0;
    painter->setPen(QPen(selected ? QColor(0, 190, 140) : QColor(130, 150, 170), 1.2));
    painter->drawLine(QLineF(cx - 5.0, cy, cx + 5.0, cy));
}
