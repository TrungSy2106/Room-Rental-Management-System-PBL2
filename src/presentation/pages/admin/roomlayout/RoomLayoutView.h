#ifndef ROOMLAYOUTVIEW_H
#define ROOMLAYOUTVIEW_H

#include "FloorPlanDocument.h"
#include "GripItem.h"

#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QPointF>
#include <QVector>
#include <functional>

class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class QContextMenuEvent;
class QPen;

enum class FloorPlanTool {
    Select, DrawWall, PlaceRoomLabel,
    PlaceDoor, PlaceStair, PlaceText,
    PlaceHAxis, PlaceVAxis,
    MoveByPoints
};

enum class SnapType { Endpoint, Midpoint, Intersection, Perpendicular };

enum class ViewDragState { None, GripDrag, PotentialItemMove, ItemMove };

class RoomLayoutView : public QGraphicsView
{
public:
    explicit RoomLayoutView(QWidget *parent = nullptr);

    void resetView();
    void setEditorEnabled(bool enabled);
    void setTool(FloorPlanTool tool);
    void setOrthoEnabled(bool enabled);
    void setOsnapEnabled(bool enabled);
    void cancelTransient();

    void setWallCreatedHandler(std::function<void(QPointF, QPointF)>);
    void setRoomLabelPositionHandler(std::function<void(QPointF)>);
    void setSymbolPlacedHandler(std::function<void(SymbolType, QPointF)>);
    void setTextPositionHandler(std::function<void(QPointF)>);
    void setAxisCreatedHandler(std::function<void(AxisDirection, QPointF, QPointF)>);

    void setGripReleasedHandler(std::function<void(QString ownerId, GripOwnerType, bool isStart, QPointF newPos)>);
    void setItemsMovedHandler(std::function<void(QVector<QGraphicsItem*>, QPointF delta)>);

    void setRequestMoveHandler(std::function<void()>);

protected:
    void drawBackground(QPainter *, const QRectF &) override;
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    QPointF acquirePoint(QPointF rawScenePos, bool *snapped);
    QPointF acquirePointFrom(QPointF rawScenePos, QPointF refPoint, bool *snapped,
                             bool hasReference = true);
    QPointF constrainAxisGrip(const GripItem *grip, QPointF rawScenePos) const;
    QPointF constrainMoveDelta(QPointF delta) const;
    QVector<QGraphicsItem *> selectedMovableItems() const;
    void updateDragMode();
    void stopPanning();
    void updatePreviewLine(QGraphicsLineItem *&item, const QLineF &line,
                           const QPen &pen, qreal zValue);
    void updateWallPreview(QPoint vp);
    void updateAxisDrawPreview(QPoint vp);
    void showSnapMarker(QPointF pos, bool visible);

    bool editorEnabled = false;
    bool orthoEnabled  = false;
    bool osnapEnabled  = false;
    FloorPlanTool tool = FloorPlanTool::Select;

    bool    hasFirstPoint = false;
    QPointF firstPoint;
    QGraphicsLineItem       *previewItem     = nullptr;
    QGraphicsLineItem       *axisPreviewItem = nullptr;
    QGraphicsRectItem       *snapMarkerRect  = nullptr;
    QGraphicsSimpleTextItem *snapMarkerText  = nullptr;
    SnapType                 snapMarkerType  = SnapType::Endpoint;

    bool    isPanning       = false;
    QPoint lastPanPosition;

    ViewDragState             dragState        = ViewDragState::None;
    GripItem                 *activeGrip       = nullptr;
    QPoint                    dragStartViewPos;
    QPointF                   dragStartScenePos;
    QVector<QGraphicsItem *>  dragItems;
    QVector<QPointF>          dragItemsOldPos;

    bool                      moveHasBase      = false;
    QPointF                   moveBasePoint;
    QVector<QGraphicsItem *>  moveSelectedItems;
    QGraphicsLineItem        *moveGuideItem    = nullptr;

    std::function<void(QPointF, QPointF)>                        wallCreatedHandler;
    std::function<void(QPointF)>                                 roomLabelPositionHandler;
    std::function<void(SymbolType, QPointF)>                     symbolPlacedHandler;
    std::function<void(QPointF)>                                 textPositionHandler;
    std::function<void(AxisDirection, QPointF, QPointF)>         axisCreatedHandler;
    std::function<void(QString, GripOwnerType, bool, QPointF)>   gripReleasedHandler;
    std::function<void(QVector<QGraphicsItem*>, QPointF)>        itemsMovedHandler;
    std::function<void()>                                        requestMoveHandler;
};

#endif
