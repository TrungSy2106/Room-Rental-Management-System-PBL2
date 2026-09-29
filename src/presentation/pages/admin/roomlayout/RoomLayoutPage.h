#ifndef ROOMLAYOUTPAGE_H
#define ROOMLAYOUTPAGE_H

#include "FloorPlanDocument.h"
#include "RoomLayoutView.h"

#include <QColor>
#include <QVector>
#include <QWidget>

class QGraphicsScene;

namespace Ui { class RoomLayoutPage; }

class RoomLayoutPage : public QWidget
{
public:
    explicit RoomLayoutPage(QWidget *parent = nullptr);
    ~RoomLayoutPage();
    void reloadRooms();
    void setViewOnlyMode();

private:
    QWidget *createLegendItem(const QColor &color, const QString &text);
    void loadFloorPlan();
    int currentFloor() const;
    void refreshFloorSelector(int selectedFloor);
    void refreshRoomSelector();
    void renderFloor(int floor, bool resetView = false);
    void setEditMode(bool enabled);
    void setTool(FloorPlanTool tool);
    void addWall(const QPointF &start, const QPointF &end);
    void placeRoomLabel(const QPointF &position);
    void placeSymbol(SymbolType type, const QPointF &position);
    void placeTextAnnotation(const QPointF &position);
    void placeAxis(AxisDirection dir, const QPointF &start, const QPointF &end);
    void addFloor();
    void saveLayout();
    QString layoutDataFilePath() const;
    void updateEditorStatus();
    void updateSelectionActions();
    void updateItemEditability();

    Ui::RoomLayoutPage *ui;
    QGraphicsScene     *scene = nullptr;

    QVector<int>        floors;
    FloorPlanDocument   document;
    FloorPlanTool       currentTool    = FloorPlanTool::Select;
    bool editMode      = false;
    bool documentLoaded= false;
};

#endif
