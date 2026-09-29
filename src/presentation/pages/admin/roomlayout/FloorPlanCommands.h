#ifndef FLOORPLANCOMMANDS_H
#define FLOORPLANCOMMANDS_H

#include "CommandStack.h"
#include "FloorPlanDocument.h"
#include "GripItem.h"

#include <QGraphicsItem>
#include <functional>

class FloorPlanCommand : public ICommand
{
protected:
    FloorPlanCommand(FloorPlanDocument *document, std::function<void()> refreshCallback);
    void refreshView() const;

    FloorPlanDocument *doc;

private:
    std::function<void()> refreshCallback;
};

class DrawWallCommand : public FloorPlanCommand
{
public:
    DrawWallCommand(FloorPlanDocument *, WallRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return "Draw wall"; }

private:
    WallRecord wall;
};

class PlaceRoomLabelCommand : public FloorPlanCommand
{
public:
    PlaceRoomLabelCommand(FloorPlanDocument *, RoomLabelRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return hadPrev ? "Move room label" : "Place room label"; }

private:
    RoomLabelRecord label, previous;
    bool hadPrev = false;
};

class UnassignRoomLabelCommand : public FloorPlanCommand
{
public:
    UnassignRoomLabelCommand(FloorPlanDocument *, RoomLabelRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return "Unassign room"; }

private:
    RoomLabelRecord label;
};

class PlaceSymbolCommand : public FloorPlanCommand
{
public:
    PlaceSymbolCommand(FloorPlanDocument *, SymbolRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return sym.type == SymbolType::Door ? "Place door" : "Place stair"; }

private:
    SymbolRecord sym;
};

class PlaceTextAnnotationCommand : public FloorPlanCommand
{
public:
    PlaceTextAnnotationCommand(FloorPlanDocument *, TextRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return "Place text"; }

private:
    TextRecord txt;
};

class PlaceAxisCommand : public FloorPlanCommand
{
public:
    PlaceAxisCommand(FloorPlanDocument *, AxisRecord, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return axis.direction == AxisDirection::Horizontal ? "Place H axis" : "Place V axis"; }

private:
    AxisRecord axis;
};

class EditWallCommand : public FloorPlanCommand
{
public:
    EditWallCommand(FloorPlanDocument *, QString id, QPointF oldS, QPointF oldE, QPointF newS, QPointF newE, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return "Edit wall"; }

private:
    QString id;
    QPointF oldS, oldE, newS, newE;
};

class EditAxisCommand : public FloorPlanCommand
{
public:
    EditAxisCommand(FloorPlanDocument *, QString id, QPointF oldS, QPointF oldE, QPointF newS, QPointF newE, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return "Edit axis"; }

private:
    QString id;
    QPointF oldS, oldE, newS, newE;
};

struct ItemMoveRecord
{
    enum class Type
    {
        Wall,
        Axis,
        Symbol,
        Text,
        RoomLabel
    };
    Type type;
    QString id;
    QPointF oldS, oldE;
    QPointF newS, newE;
    QPointF oldPos;
    QPointF newPos;
};

class MoveItemsCommand : public FloorPlanCommand
{
public:
    MoveItemsCommand(FloorPlanDocument *, QVector<ItemMoveRecord>, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return moves.size() == 1 ? "Move item" : "Move items"; }

private:
    QVector<ItemMoveRecord> moves;
};

class RotateSymbolsCommand : public FloorPlanCommand
{
public:
    RotateSymbolsCommand(FloorPlanDocument *, QVector<QPair<QString, qreal>> changes, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return changes.size() == 1 ? "Rotate symbol" : "Rotate symbols"; }

private:
    QVector<QPair<QString, qreal>> changes;
    QVector<qreal> oldRotations;
};

class DeleteEditablesCommand : public FloorPlanCommand
{
public:
    DeleteEditablesCommand(FloorPlanDocument *, QVector<WallRecord>, QVector<SymbolRecord>, QVector<TextRecord>, QVector<AxisRecord>, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return (walls.size() + syms.size() + texts.size() + axes.size()) == 1 ? "Delete item" : "Delete items"; }

private:
    QVector<WallRecord> walls;
    QVector<SymbolRecord> syms;
    QVector<TextRecord> texts;
    QVector<AxisRecord> axes;
};

class PasteEditablesCommand : public FloorPlanCommand
{
public:
    PasteEditablesCommand(FloorPlanDocument *, QVector<WallRecord>, QVector<SymbolRecord>, QVector<TextRecord>, QVector<AxisRecord>, std::function<void()>);
    void undo() override;
    void redo() override;
    std::string description() const override { return (walls.size() + symbols.size() + texts.size() + axes.size()) == 1 ? "Paste item" : "Paste items"; }

private:
    QVector<WallRecord> walls;
    QVector<SymbolRecord> symbols;
    QVector<TextRecord> texts;
    QVector<AxisRecord> axes;
};

#endif
