#ifndef ROOMLAYOUTVIEW_H
#define ROOMLAYOUTVIEW_H

#include "FloorPlanDocument.h"

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

class RoomLayoutView : public QGraphicsView
{
public:
    explicit RoomLayoutView(QWidget *parent = nullptr);

    void resetView();
    void setEditorEnabled(bool enabled);
    void setTool(FloorPlanTool tool);
    void cancelTransient();

    void setWallCreatedHandler(std::function<void(QPointF, QPointF)>);
    void setRoomLabelPositionHandler(std::function<void(QPointF)>);
    void setSymbolPlacedHandler(std::function<void(SymbolType, QPointF)>);
    void setTextPositionHandler(std::function<void(QPointF)>);
    void setAxisCreatedHandler(std::function<void(AxisDirection, QPointF, QPointF)>);

protected:
    void drawBackground(QPainter *, const QRectF &) override;
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    void updateDragMode();
    void stopPanning();
    void updatePreviewLine(QGraphicsLineItem *&item, const QLineF &line,
                           const QPen &pen, qreal zValue);
    void updateWallPreview(QPoint vp);
    void updateAxisDrawPreview(QPoint vp);

    bool editorEnabled = false;
    FloorPlanTool tool = FloorPlanTool::Select;

    bool    hasFirstPoint = false;
    QPointF firstPoint;
    QGraphicsLineItem *previewItem     = nullptr;
    QGraphicsLineItem *axisPreviewItem = nullptr;

    bool    isPanning = false;
    QPoint lastPanPosition;

    std::function<void(QPointF, QPointF)>                    wallCreatedHandler;
    std::function<void(QPointF)>                             roomLabelPositionHandler;
    std::function<void(SymbolType, QPointF)>                 symbolPlacedHandler;
    std::function<void(QPointF)>                             textPositionHandler;
    std::function<void(AxisDirection, QPointF, QPointF)>     axisCreatedHandler;
};

#endif
