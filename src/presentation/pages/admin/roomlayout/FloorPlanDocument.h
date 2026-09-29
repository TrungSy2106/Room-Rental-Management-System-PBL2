#ifndef FLOORPLANDOCUMENT_H
#define FLOORPLANDOCUMENT_H

#include <QPointF>
#include <QString>
#include <QVector>

struct WallRecord       { QString id; int floor=1; QPointF start; QPointF end; };
struct RoomLabelRecord  { int floor=1; QString roomId; QPointF position; };
enum class SymbolType   { Door, Stair };
struct SymbolRecord     { QString id; int floor=1; SymbolType type=SymbolType::Door; QPointF position; qreal rotation = 0.0; };
struct TextRecord       { QString id; int floor=1; QPointF position; QString text; };
enum class AxisDirection{ Horizontal, Vertical };

struct AxisRecord {
    QString       id;
    int           floor     = 1;
    AxisDirection direction = AxisDirection::Horizontal;
    QPointF       start;
    QPointF       end;
    QString       label;
};

class FloorPlanDocument
{
public:
    const QVector<WallRecord>     &walls()      const;
    const QVector<RoomLabelRecord>&roomLabels() const;
    const QVector<SymbolRecord>   &symbols()    const;
    const QVector<TextRecord>     &texts()      const;
    const QVector<AxisRecord>     &axes()       const;
    void clear();

    void addWall   (const WallRecord &w);
    bool removeWall(const QString &id, WallRecord *out = nullptr);
    bool findWall  (const QString &id, WallRecord *out = nullptr) const;
    bool updateWall(const QString &id, QPointF newStart, QPointF newEnd);

    void setRoomLabel   (const RoomLabelRecord &l);
    bool removeRoomLabel(const QString &roomId, RoomLabelRecord *out = nullptr);
    bool findRoomLabel  (const QString &roomId, RoomLabelRecord *out = nullptr) const;
    bool updateRoomLabelPos(const QString &roomId, QPointF newPos);

    void addSymbol   (const SymbolRecord &s);
    bool removeSymbol(const QString &id, SymbolRecord *out = nullptr);
    bool findSymbol  (const QString &id, SymbolRecord *out = nullptr) const;
    bool updateSymbolPos(const QString &id, QPointF newPos);
    bool updateSymbolRotation(const QString &id, qreal newRotation);

    void addText   (const TextRecord &t);
    bool removeText(const QString &id, TextRecord *out = nullptr);
    bool findText  (const QString &id, TextRecord *out = nullptr) const;
    bool updateTextPos(const QString &id, QPointF newPos);

    void addAxis   (const AxisRecord &a);
    bool removeAxis(const QString &id, AxisRecord *out = nullptr);
    bool findAxis  (const QString &id, AxisRecord *out = nullptr) const;
    bool updateAxis(const QString &id, QPointF newStart, QPointF newEnd);

private:
    QVector<WallRecord>      wallRecords;
    QVector<RoomLabelRecord> labelRecords;
    QVector<SymbolRecord>    symbolRecords;
    QVector<TextRecord>      textRecords;
    QVector<AxisRecord>      axisRecords;
};

#endif
