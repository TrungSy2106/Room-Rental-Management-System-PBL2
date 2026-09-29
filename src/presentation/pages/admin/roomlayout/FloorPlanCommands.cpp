#include "FloorPlanCommands.h"
#include <utility>

FloorPlanCommand::FloorPlanCommand(FloorPlanDocument *document,
                                   std::function<void()> refresh)
    : doc(document), refreshCallback(std::move(refresh))
{
}

void FloorPlanCommand::refreshView() const
{
    if (refreshCallback)
        refreshCallback();
}

DrawWallCommand::DrawWallCommand(FloorPlanDocument *d, WallRecord w, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), wall(std::move(w)) {}
void DrawWallCommand::undo()
{
    doc->removeWall(wall.id);
    refreshView();
}
void DrawWallCommand::redo()
{
    doc->addWall(wall);
    refreshView();
}

PlaceRoomLabelCommand::PlaceRoomLabelCommand(FloorPlanDocument *d, RoomLabelRecord l, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), label(std::move(l))
{
    hadPrev = doc->findRoomLabel(this->label.roomId, &previous);
}
void PlaceRoomLabelCommand::undo()
{
    if (hadPrev)
        doc->setRoomLabel(previous);
    else
        doc->removeRoomLabel(label.roomId);
    refreshView();
}
void PlaceRoomLabelCommand::redo()
{
    doc->setRoomLabel(label);
    refreshView();
}

UnassignRoomLabelCommand::UnassignRoomLabelCommand(FloorPlanDocument *d, RoomLabelRecord l, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), label(std::move(l)) {}
void UnassignRoomLabelCommand::undo()
{
    doc->setRoomLabel(label);
    refreshView();
}
void UnassignRoomLabelCommand::redo()
{
    doc->removeRoomLabel(label.roomId);
    refreshView();
}

PlaceSymbolCommand::PlaceSymbolCommand(FloorPlanDocument *d, SymbolRecord s, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), sym(std::move(s)) {}
void PlaceSymbolCommand::undo()
{
    doc->removeSymbol(sym.id);
    refreshView();
}
void PlaceSymbolCommand::redo()
{
    doc->addSymbol(sym);
    refreshView();
}

PlaceTextAnnotationCommand::PlaceTextAnnotationCommand(FloorPlanDocument *d, TextRecord t, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), txt(std::move(t)) {}
void PlaceTextAnnotationCommand::undo()
{
    doc->removeText(txt.id);
    refreshView();
}
void PlaceTextAnnotationCommand::redo()
{
    doc->addText(txt);
    refreshView();
}

PlaceAxisCommand::PlaceAxisCommand(FloorPlanDocument *d, AxisRecord a, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), axis(std::move(a)) {}
void PlaceAxisCommand::undo()
{
    doc->removeAxis(axis.id);
    refreshView();
}
void PlaceAxisCommand::redo()
{
    doc->addAxis(axis);
    refreshView();
}

EditWallCommand::EditWallCommand(FloorPlanDocument *d, QString id_, QPointF os, QPointF oe, QPointF ns, QPointF ne, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), id(std::move(id_)), oldS(os), oldE(oe), newS(ns), newE(ne) {}
void EditWallCommand::undo()
{
    doc->updateWall(id, oldS, oldE);
    refreshView();
}
void EditWallCommand::redo()
{
    doc->updateWall(id, newS, newE);
    refreshView();
}

EditAxisCommand::EditAxisCommand(FloorPlanDocument *d, QString id_, QPointF os, QPointF oe, QPointF ns, QPointF ne, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), id(std::move(id_)), oldS(os), oldE(oe), newS(ns), newE(ne) {}
void EditAxisCommand::undo()
{
    doc->updateAxis(id, oldS, oldE);
    refreshView();
}
void EditAxisCommand::redo()
{
    doc->updateAxis(id, newS, newE);
    refreshView();
}

MoveItemsCommand::MoveItemsCommand(FloorPlanDocument *d, QVector<ItemMoveRecord> m, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), moves(std::move(m)) {}
void MoveItemsCommand::undo()
{
    for (const ItemMoveRecord &m : moves)
    {
        switch (m.type)
        {
        case ItemMoveRecord::Type::Wall:
            doc->updateWall(m.id, m.oldS, m.oldE);
            break;
        case ItemMoveRecord::Type::Axis:
            doc->updateAxis(m.id, m.oldS, m.oldE);
            break;
        case ItemMoveRecord::Type::Symbol:
            doc->updateSymbolPos(m.id, m.oldPos);
            break;
        case ItemMoveRecord::Type::Text:
            doc->updateTextPos(m.id, m.oldPos);
            break;
        case ItemMoveRecord::Type::RoomLabel:
            doc->updateRoomLabelPos(m.id, m.oldPos);
            break;
        }
    }
    refreshView();
}
void MoveItemsCommand::redo()
{
    for (const ItemMoveRecord &m : moves)
    {
        switch (m.type)
        {
        case ItemMoveRecord::Type::Wall:
            doc->updateWall(m.id, m.newS, m.newE);
            break;
        case ItemMoveRecord::Type::Axis:
            doc->updateAxis(m.id, m.newS, m.newE);
            break;
        case ItemMoveRecord::Type::Symbol:
            doc->updateSymbolPos(m.id, m.newPos);
            break;
        case ItemMoveRecord::Type::Text:
            doc->updateTextPos(m.id, m.newPos);
            break;
        case ItemMoveRecord::Type::RoomLabel:
            doc->updateRoomLabelPos(m.id, m.newPos);
            break;
        }
    }
    refreshView();
}

RotateSymbolsCommand::RotateSymbolsCommand(FloorPlanDocument *d, QVector<QPair<QString, qreal>> c, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), changes(std::move(c))
{
    for (const auto &pair : changes)
    {
        SymbolRecord sym;
        oldRotations.append(doc->findSymbol(pair.first, &sym) ? sym.rotation : 0.0);
    }
}
void RotateSymbolsCommand::undo()
{
    for (int i = 0; i < changes.size(); ++i)
        doc->updateSymbolRotation(changes[i].first, oldRotations[i]);
    refreshView();
}
void RotateSymbolsCommand::redo()
{
    for (const auto &pair : changes)
        doc->updateSymbolRotation(pair.first, pair.second);
    refreshView();
}

DeleteEditablesCommand::DeleteEditablesCommand(FloorPlanDocument *d, QVector<WallRecord> w, QVector<SymbolRecord> s, QVector<TextRecord> t, QVector<AxisRecord> a, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), walls(std::move(w)), syms(std::move(s)), texts(std::move(t)), axes(std::move(a)) {}
void DeleteEditablesCommand::undo()
{
    for (const WallRecord &w : walls)
        doc->addWall(w);
    for (const SymbolRecord &s : syms)
        doc->addSymbol(s);
    for (const TextRecord &t : texts)
        doc->addText(t);
    for (const AxisRecord &a : axes)
        doc->addAxis(a);
    refreshView();
}
void DeleteEditablesCommand::redo()
{
    for (const WallRecord &w : walls)
        doc->removeWall(w.id);
    for (const SymbolRecord &s : syms)
        doc->removeSymbol(s.id);
    for (const TextRecord &t : texts)
        doc->removeText(t.id);
    for (const AxisRecord &a : axes)
        doc->removeAxis(a.id);
    refreshView();
}

PasteEditablesCommand::PasteEditablesCommand(FloorPlanDocument *d, QVector<WallRecord> w, QVector<SymbolRecord> s, QVector<TextRecord> t, QVector<AxisRecord> a, std::function<void()> r)
    : FloorPlanCommand(d, std::move(r)), walls(std::move(w)), symbols(std::move(s)), texts(std::move(t)), axes(std::move(a)) {}
void PasteEditablesCommand::undo()
{
    for (const WallRecord &wall : walls)
        doc->removeWall(wall.id);
    for (const SymbolRecord &symbol : symbols)
        doc->removeSymbol(symbol.id);
    for (const TextRecord &text : texts)
        doc->removeText(text.id);
    for (const AxisRecord &axis : axes)
        doc->removeAxis(axis.id);
    refreshView();
}
void PasteEditablesCommand::redo()
{
    for (const WallRecord &wall : walls)
        doc->addWall(wall);
    for (const SymbolRecord &symbol : symbols)
        doc->addSymbol(symbol);
    for (const TextRecord &text : texts)
        doc->addText(text);
    for (const AxisRecord &axis : axes)
        doc->addAxis(axis);
    refreshView();
}
