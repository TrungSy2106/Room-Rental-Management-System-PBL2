#include "RoomLayoutView.h"

#include "WallGraphicsItem.h"

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>
#include <cmath>

namespace {
constexpr int MoveThreshold = 5;

bool isAxisTool(FloorPlanTool tool)
{
    return tool == FloorPlanTool::PlaceHAxis || tool == FloorPlanTool::PlaceVAxis;
}

bool usesCrossCursor(FloorPlanTool tool)
{
    return tool == FloorPlanTool::DrawWall || tool == FloorPlanTool::PlaceDoor
           || tool == FloorPlanTool::PlaceStair || tool == FloorPlanTool::PlaceText
           || isAxisTool(tool);
}

template <typename Item>
void deleteSceneItem(Item *&item)
{
    if (!item) return;
    if (item->scene()) item->scene()->removeItem(item);
    delete item;
    item = nullptr;
}
}

RoomLayoutView::RoomLayoutView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void RoomLayoutView::resetView()
{
    resetTransform();
    if (scene()) fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void RoomLayoutView::setEditorEnabled(bool enabled)
{
    editorEnabled = enabled;
    updateDragMode();
}

void RoomLayoutView::setTool(FloorPlanTool newTool)
{
    if (tool != newTool) cancelTransient();
    tool = newTool;
    updateDragMode();
    setCursor(editorEnabled && usesCrossCursor(tool) ? Qt::CrossCursor : Qt::ArrowCursor);
}

void RoomLayoutView::setSnapPoints(const QVector<SnapCandidate> &pts) { snapCandidates = pts; }
void RoomLayoutView::setWallSegments(const QVector<QPair<QPointF,QPointF>> &segs) { wallSegs = segs; }

void RoomLayoutView::cancelTransient()
{
    stopPanning();
    hasFirstPoint = false;
    deleteSceneItem(previewItem);
    deleteSceneItem(axisPreviewItem);
    deleteSceneItem(snapMarkerRect);
    deleteSceneItem(snapMarkerText);
    dragState = ViewDragState::None;
    dragItems.clear(); dragItemsOldPos.clear();
}

void RoomLayoutView::updateDragMode()
{
    setDragMode(editorEnabled && tool == FloorPlanTool::Select
                    ? QGraphicsView::RubberBandDrag
                    : QGraphicsView::NoDrag);
}

void RoomLayoutView::stopPanning()
{
    if (!isPanning) return;
    isPanning = false;
    if (viewport()->mouseGrabber() == viewport()) viewport()->releaseMouse();
}

QVector<QGraphicsItem *> RoomLayoutView::selectedMovableItems() const
{
    QVector<QGraphicsItem *> items;
    if (!scene()) return items;
    for (QGraphicsItem *item : scene()->selectedItems())
        items.append(item);
    return items;
}

void RoomLayoutView::updatePreviewLine(QGraphicsLineItem *&item, const QLineF &line,
                                       const QPen &pen, qreal zValue)
{
    if (!item) {
        item = scene()->addLine(line, pen);
        item->setZValue(zValue);
    } else {
        item->setLine(line);
    }
}

void RoomLayoutView::setWallCreatedHandler     (std::function<void(QPointF,QPointF)> f) { wallCreatedHandler=std::move(f); }
void RoomLayoutView::setRoomLabelPositionHandler(std::function<void(QPointF)> f)        { roomLabelPositionHandler=std::move(f); }
void RoomLayoutView::setSymbolPlacedHandler    (std::function<void(SymbolType,QPointF)> f){ symbolPlacedHandler=std::move(f); }
void RoomLayoutView::setTextPositionHandler    (std::function<void(QPointF)> f)         { textPositionHandler=std::move(f); }
void RoomLayoutView::setAxisCreatedHandler     (std::function<void(AxisDirection,QPointF,QPointF)> f){ axisCreatedHandler=std::move(f); }
void RoomLayoutView::setItemsMovedHandler      (std::function<void(QVector<QGraphicsItem*>,QPointF)> f){ itemsMovedHandler=std::move(f); }

void RoomLayoutView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    if (!scene()) return;
    const qreal step = 40.0;
    QPen gridPen(QColor(170, 180, 190, 115), 0.0, Qt::CustomDashLine, Qt::FlatCap);
    gridPen.setCosmetic(true);
    gridPen.setDashPattern({1.5, 3.5});
    painter->setPen(gridPen);
    qreal x = std::floor(rect.left()/step)*step;
    for (; x <= rect.right(); x += step) painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    qreal y = std::floor(rect.top()/step)*step;
    for (; y <= rect.bottom(); y += step) painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

void RoomLayoutView::wheelEvent(QWheelEvent *event)
{
    const qreal factor = event->angleDelta().y() > 0 ? 1.15 : (1.0/1.15);
    const qreal cur = transform().m11();
    if ((cur > 8.0 && factor > 1.0) || (cur < 0.05 && factor < 1.0)) return;
    scale(factor, factor);
    event->accept();
}

void RoomLayoutView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        isPanning = true;
        lastPanPosition = event->pos();
        viewport()->grabMouse();
        setCursor(Qt::ClosedHandCursor);
        event->accept(); return;
    }
    stopPanning();
    if (!editorEnabled || event->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(event); return;
    }

    const QPointF scenePos = mapToScene(event->pos());

    if (tool == FloorPlanTool::DrawWall) {
        if (!hasFirstPoint) {
            firstPoint = scenePos; hasFirstPoint = true;
            showSnapMarker(scenePos, true);
        } else {
            const QPointF start = firstPoint;
            cancelTransient();
            if (scenePos != start && wallCreatedHandler)
                wallCreatedHandler(start, scenePos);
        }
        event->accept(); return;
    }

    if (isAxisTool(tool)) {
        if (!hasFirstPoint) {
            firstPoint = scenePos; hasFirstPoint = true;
            showSnapMarker(scenePos, true);
        } else {
            QPointF locked = (tool == FloorPlanTool::PlaceHAxis)
                             ? QPointF(scenePos.x(), firstPoint.y())
                             : QPointF(firstPoint.x(), scenePos.y());
            if (locked != firstPoint && axisCreatedHandler) {
                const AxisDirection dir = (tool==FloorPlanTool::PlaceHAxis)
                                          ? AxisDirection::Horizontal
                                          : AxisDirection::Vertical;
                const QPointF start = firstPoint;
                cancelTransient();
                axisCreatedHandler(dir, start, locked);
            } else {
                cancelTransient();
            }
        }
        event->accept(); return;
    }

    if (tool == FloorPlanTool::PlaceRoomLabel) {
        if (roomLabelPositionHandler) roomLabelPositionHandler(scenePos);
        event->accept(); return;
    }
    if (tool == FloorPlanTool::PlaceDoor || tool == FloorPlanTool::PlaceStair) {
        if (symbolPlacedHandler)
            symbolPlacedHandler(tool==FloorPlanTool::PlaceDoor ? SymbolType::Door:SymbolType::Stair, scenePos);
        event->accept(); return;
    }
    if (tool == FloorPlanTool::PlaceText) {
        if (textPositionHandler) textPositionHandler(scenePos);
        event->accept(); return;
    }

    if (tool == FloorPlanTool::Select) {
        QGraphicsItem *hit = itemAt(event->pos());
        if (hit) {
            setDragMode(QGraphicsView::NoDrag);
        } else {
            setDragMode(QGraphicsView::RubberBandDrag);
        }

        QGraphicsView::mousePressEvent(event);

        dragItems = selectedMovableItems();
        if (!dragItems.isEmpty()) {
            dragItemsOldPos.clear();
            for (auto *item : dragItems) dragItemsOldPos << item->pos();
            dragStartViewPos  = event->pos();
            dragStartScenePos = scenePos;
            dragState = ViewDragState::PotentialItemMove;
        }
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void RoomLayoutView::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning) {
        const QPoint delta = event->pos() - lastPanPosition;
        lastPanPosition = event->pos();
        const qreal scaleX = transform().m11();
        const qreal scaleY = transform().m22();
        if (!qFuzzyIsNull(scaleX) && !qFuzzyIsNull(scaleY))
            translate(delta.x() / scaleX, delta.y() / scaleY);
        event->accept(); return;
    }

    if (editorEnabled && tool == FloorPlanTool::DrawWall) {
        updateWallPreview(event->pos()); event->accept(); return;
    }

    if (editorEnabled && isAxisTool(tool)) {
        updateAxisDrawPreview(event->pos()); event->accept(); return;
    }

    if (dragState == ViewDragState::PotentialItemMove) {
        if ((event->pos()-dragStartViewPos).manhattanLength() > MoveThreshold)
            dragState = ViewDragState::ItemMove;
    }

    if (dragState == ViewDragState::ItemMove) {
        const QPointF delta = mapToScene(event->pos()) - dragStartScenePos;
        for (int i = 0; i < dragItems.size(); ++i)
            dragItems[i]->setPos(dragItemsOldPos[i] + delta);
        event->accept(); return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void RoomLayoutView::mouseReleaseEvent(QMouseEvent *event)
{
    updateDragMode();

    if (isPanning && (event->button()==Qt::MiddleButton||event->button()==Qt::RightButton)) {
        stopPanning();
        setCursor(tool==FloorPlanTool::DrawWall||isAxisTool(tool)
                      ? Qt::CrossCursor : Qt::ArrowCursor);
        event->accept(); return;
    }

    if (event->button() == Qt::LeftButton) {
        if (dragState == ViewDragState::ItemMove) {
            const QPointF delta = mapToScene(event->pos()) - dragStartScenePos;
            for (int i = 0; i < dragItems.size(); ++i)
                dragItems[i]->setPos(dragItemsOldPos[i]);

            if (delta.manhattanLength() > 0.5 && itemsMovedHandler)
                itemsMovedHandler(dragItems, delta);

            dragItems.clear(); dragItemsOldPos.clear();
            dragState = ViewDragState::None;
            event->accept(); return;
        }

        if (dragState == ViewDragState::PotentialItemMove) {
            dragItems.clear(); dragItemsOldPos.clear();
            dragState = ViewDragState::None;
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void RoomLayoutView::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
}

void RoomLayoutView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
}

void RoomLayoutView::updateWallPreview(QPoint vp)
{
    if (!scene() || !hasFirstPoint) return;
    const QPointF pt = mapToScene(vp);
    showSnapMarker(pt, true);
    const QLineF line(firstPoint, pt);
    QPen pen(QColor(0,145,105),2.0,Qt::DashLine); pen.setCosmetic(true);
    updatePreviewLine(previewItem, line, pen, 30.0);
}

void RoomLayoutView::updateAxisDrawPreview(QPoint vp)
{
    if (!scene() || !hasFirstPoint) return;
    const QPointF pt = mapToScene(vp);
    QPointF locked = (tool == FloorPlanTool::PlaceHAxis)
                     ? QPointF(pt.x(), firstPoint.y())
                     : QPointF(firstPoint.x(), pt.y());
    showSnapMarker(locked, true);
    const QLineF line(firstPoint, locked);
    QPen pen(QColor(60,90,200),2.0,Qt::CustomDashLine);
    pen.setDashPattern({8.0,4.0}); pen.setCosmetic(true);
    updatePreviewLine(axisPreviewItem, line, pen, 29.0);
}

void RoomLayoutView::showSnapMarker(QPointF pos, bool visible)
{
    if (!scene()) return;
    const QPen markerPen(QColor(0,200,150), 1.5);
    const qreal S = 5.0;

    if (visible) {
        if (!snapMarkerRect) {
            snapMarkerRect = scene()->addRect(QRectF(-S,-S,2*S,2*S), markerPen, Qt::NoBrush);
            snapMarkerRect->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            snapMarkerRect->setZValue(31.0);
            snapMarkerText = new QGraphicsSimpleTextItem();
            snapMarkerText->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            snapMarkerText->setZValue(32.0);
            QFont f; f.setPointSize(7); snapMarkerText->setFont(f);
            snapMarkerText->setBrush(QColor(0,200,150));
            snapMarkerText->setText(QStringLiteral("\u25a0"));
            scene()->addItem(snapMarkerText);
        }
        snapMarkerRect->setPos(pos);
        snapMarkerRect->setVisible(true);
        snapMarkerText->setPos(QPointF(pos.x() + S + 1, pos.y() - S - 1));
        snapMarkerText->setVisible(true);
    } else {
        if (snapMarkerRect) snapMarkerRect->setVisible(false);
        if (snapMarkerText) snapMarkerText->setVisible(false);
    }
}
