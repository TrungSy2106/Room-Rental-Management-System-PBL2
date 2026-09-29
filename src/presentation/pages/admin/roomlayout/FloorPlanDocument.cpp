#include "FloorPlanDocument.h"

const QVector<WallRecord> &FloorPlanDocument::walls() const { return wallRecords; }
const QVector<RoomLabelRecord> &FloorPlanDocument::roomLabels() const { return labelRecords; }
const QVector<SymbolRecord> &FloorPlanDocument::symbols() const { return symbolRecords; }
const QVector<TextRecord> &FloorPlanDocument::texts() const { return textRecords; }
const QVector<AxisRecord> &FloorPlanDocument::axes() const { return axisRecords; }

void FloorPlanDocument::clear()
{
    wallRecords.clear();
    labelRecords.clear();
    symbolRecords.clear();
    textRecords.clear();
    axisRecords.clear();
}

void FloorPlanDocument::addWall(const WallRecord &w)
{
    if (!findWall(w.id))
        wallRecords.append(w);
}
bool FloorPlanDocument::removeWall(const QString &id, WallRecord *out)
{
    for (int i = 0; i < wallRecords.size(); ++i)
    {
        if (wallRecords[i].id != id)
            continue;
        if (out)
            *out = wallRecords[i];
        wallRecords.removeAt(i);
        return true;
    }
    return false;
}
bool FloorPlanDocument::findWall(const QString &id, WallRecord *out) const
{
    for (const WallRecord &r : wallRecords)
    {
        if (r.id == id)
        {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateWall(const QString &id, QPointF s, QPointF e)
{
    for (WallRecord &r : wallRecords)
    {
        if (r.id == id)
        {
            r.start = s;
            r.end = e;
            return true;
        }
    }
    return false;
}

void FloorPlanDocument::setRoomLabel(const RoomLabelRecord &l)
{
    for (RoomLabelRecord &r : labelRecords)
    {
        if (r.roomId == l.roomId)
        {
            r = l;
            return;
        }
    }
    labelRecords.append(l);
}
bool FloorPlanDocument::removeRoomLabel(const QString &roomId, RoomLabelRecord *out)
{
    for (int i = 0; i < labelRecords.size(); ++i)
    {
        if (labelRecords[i].roomId != roomId)
            continue;
        if (out)
            *out = labelRecords[i];
        labelRecords.removeAt(i);
        return true;
    }
    return false;
}
bool FloorPlanDocument::findRoomLabel(const QString &roomId, RoomLabelRecord *out) const
{
    for (const RoomLabelRecord &r : labelRecords)
    {
        if (r.roomId == roomId)
        {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateRoomLabelPos(const QString &roomId, QPointF pos)
{
    for (RoomLabelRecord &r : labelRecords)
    {
        if (r.roomId == roomId)
        {
            r.position = pos;
            return true;
        }
    }
    return false;
}

void FloorPlanDocument::addSymbol(const SymbolRecord &s)
{
    if (!findSymbol(s.id))
        symbolRecords.append(s);
}
bool FloorPlanDocument::removeSymbol(const QString &id, SymbolRecord *out)
{
    for (int i = 0; i < symbolRecords.size(); ++i)
    {
        if (symbolRecords[i].id != id)
            continue;
        if (out)
            *out = symbolRecords[i];
        symbolRecords.removeAt(i);
        return true;
    }
    return false;
}
bool FloorPlanDocument::findSymbol(const QString &id, SymbolRecord *out) const
{
    for (const SymbolRecord &r : symbolRecords)
    {
        if (r.id == id)
        {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateSymbolPos(const QString &id, QPointF pos)
{
    for (SymbolRecord &r : symbolRecords)
    {
        if (r.id == id)
        {
            r.position = pos;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateSymbolRotation(const QString &id, qreal newRot)
{
    for (SymbolRecord &r : symbolRecords)
    {
        if (r.id == id)
        {
            r.rotation = newRot;
            return true;
        }
    }
    return false;
}

void FloorPlanDocument::addText(const TextRecord &t)
{
    if (!findText(t.id))
        textRecords.append(t);
}
bool FloorPlanDocument::removeText(const QString &id, TextRecord *out)
{
    for (int i = 0; i < textRecords.size(); ++i)
    {
        if (textRecords[i].id != id)
            continue;
        if (out)
            *out = textRecords[i];
        textRecords.removeAt(i);
        return true;
    }
    return false;
}
bool FloorPlanDocument::findText(const QString &id, TextRecord *out) const
{
    for (const TextRecord &r : textRecords)
    {
        if (r.id == id)
        {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateTextPos(const QString &id, QPointF pos)
{
    for (TextRecord &r : textRecords)
    {
        if (r.id == id)
        {
            r.position = pos;
            return true;
        }
    }
    return false;
}

void FloorPlanDocument::addAxis(const AxisRecord &a)
{
    if (!findAxis(a.id))
        axisRecords.append(a);
}
bool FloorPlanDocument::removeAxis(const QString &id, AxisRecord *out)
{
    for (int i = 0; i < axisRecords.size(); ++i)
    {
        if (axisRecords[i].id != id)
            continue;
        if (out)
            *out = axisRecords[i];
        axisRecords.removeAt(i);
        return true;
    }
    return false;
}
bool FloorPlanDocument::findAxis(const QString &id, AxisRecord *out) const
{
    for (const AxisRecord &r : axisRecords)
    {
        if (r.id == id)
        {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}
bool FloorPlanDocument::updateAxis(const QString &id, QPointF s, QPointF e)
{
    for (AxisRecord &r : axisRecords)
    {
        if (r.id == id)
        {
            r.start = s;
            r.end = e;
            return true;
        }
    }
    return false;
}
