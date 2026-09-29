#include "FloorPlanRepository.h"

#include <QFile>
#include <QSaveFile>
#include <QTextStream>
#include <utility>

namespace {
bool parseSectionHeader(const QString &line, const QString &name, int *count){
    const QStringList f = line.split(',');
    bool ok = false;
    if(f.size()!=2 || f[0].trimmed()!=name) return false;
    *count = f[1].trimmed().toInt(&ok); return ok && *count>=0; }

QString symName(SymbolType t){ return t==SymbolType::Door ? QStringLiteral("door") : QStringLiteral("stair"); }
bool parseSym(const QString &s, SymbolType *out){
    if(s==QStringLiteral("door")){ *out=SymbolType::Door; return true; }
    if(s==QStringLiteral("stair")){ *out=SymbolType::Stair; return true; } return false; }

QString axName(AxisDirection d){ return d==AxisDirection::Horizontal ? QStringLiteral("h") : QStringLiteral("v"); }
bool parseAx(const QString &s, AxisDirection *out){
    if(s==QStringLiteral("h")){ *out=AxisDirection::Horizontal; return true; }
    if(s==QStringLiteral("v")){ *out=AxisDirection::Vertical;   return true; } return false; }
}

FloorPlanRepository::FloorPlanRepository(QString path) : filePath(std::move(path)) {}

bool FloorPlanRepository::load(FloorPlanDocument *doc, bool *missing, QString *err) const
{
    if(!doc) return false;
    doc->clear();
    if(missing) *missing = false;
    QFile file(filePath);
    if(!file.exists()){ if(missing) *missing=true; return true; }
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){ if(err) *err=QStringLiteral("Cannot open %1").arg(filePath); return false; }
    QTextStream s(&file);
    if(s.readLine().trimmed()!=QStringLiteral("FLOOR_PLAN_V1")){ if(err) *err=QStringLiteral("Invalid header"); return false; }

    int n=0;
    if(!parseSectionHeader(s.readLine(),QStringLiteral("WALLS"),&n)){ if(err) *err=QStringLiteral("WALLS missing"); return false; }
    for(int i=0;i<n;++i){
        QStringList f=s.readLine().split(','); bool ok[6];
        if(f.size()!=6){ if(err) *err=QStringLiteral("Wall %1").arg(i+1); return false; }
        WallRecord w; w.id=f[0].trimmed(); w.floor=f[1].trimmed().toInt(&ok[0]);
        w.start.setX(f[2].trimmed().toDouble(&ok[1])); w.start.setY(f[3].trimmed().toDouble(&ok[2]));
        w.end.setX  (f[4].trimmed().toDouble(&ok[3])); w.end.setY  (f[5].trimmed().toDouble(&ok[4]));
        if(w.id.isEmpty()||!ok[0]||!ok[1]||!ok[2]||!ok[3]||!ok[4]||w.floor<1){ if(err) *err=QStringLiteral("Bad wall %1").arg(i+1); return false; }
        doc->addWall(w); }

    if(!parseSectionHeader(s.readLine(),QStringLiteral("LABELS"),&n)){ if(err) *err=QStringLiteral("LABELS missing"); return false; }
    for(int i=0;i<n;++i){
        QStringList f=s.readLine().split(','); bool ok[3];
        if(f.size()!=4){ if(err) *err=QStringLiteral("Label %1").arg(i+1); return false; }
        RoomLabelRecord l; l.floor=f[0].trimmed().toInt(&ok[0]); l.roomId=f[1].trimmed();
        l.position.setX(f[2].trimmed().toDouble(&ok[1])); l.position.setY(f[3].trimmed().toDouble(&ok[2]));
        if(!ok[0]||!ok[1]||!ok[2]||l.roomId.isEmpty()||l.floor<1){ if(err) *err=QStringLiteral("Bad label %1").arg(i+1); return false; }
        doc->setRoomLabel(l); }

    while(!s.atEnd()){
        const QString hdr = s.readLine().trimmed(); if(hdr.isEmpty()) continue;
        if(hdr.startsWith(QStringLiteral("SYMBOLS"))){
            if(!parseSectionHeader(hdr,QStringLiteral("SYMBOLS"),&n)) continue;
            for(int i=0;i<n;++i){
                QStringList f=s.readLine().split(','); bool ok[3]; SymbolType st;
                if(f.size()<5||!parseSym(f[2].trimmed(),&st)){ if(err) *err=QStringLiteral("Symbol %1").arg(i+1); return false; }
                SymbolRecord sym; sym.id=f[0].trimmed(); sym.floor=f[1].trimmed().toInt(&ok[0]); sym.type=st;
                sym.position.setX(f[3].trimmed().toDouble(&ok[1])); sym.position.setY(f[4].trimmed().toDouble(&ok[2]));
                sym.rotation = (f.size() > 5) ? f[5].trimmed().toDouble() : 0.0;
                if(sym.id.isEmpty()||!ok[0]||!ok[1]||!ok[2]||sym.floor<1) continue;
                doc->addSymbol(sym); }
        } else if(hdr.startsWith(QStringLiteral("TEXTS"))){
            if(!parseSectionHeader(hdr,QStringLiteral("TEXTS"),&n)) continue;
            for(int i=0;i<n;++i){
                QStringList f=s.readLine().split(','); bool ok[3];
                if(f.size()<5) continue;
                TextRecord t; t.id=f[0].trimmed(); t.floor=f[1].trimmed().toInt(&ok[0]);
                t.position.setX(f[2].trimmed().toDouble(&ok[1])); t.position.setY(f[3].trimmed().toDouble(&ok[2]));
                t.text=f.mid(4).join(QLatin1Char(','));
                if(t.id.isEmpty()||!ok[0]||!ok[1]||!ok[2]||t.floor<1||t.text.isEmpty()) continue;
                doc->addText(t); }
        } else if(hdr.startsWith(QStringLiteral("AXES"))){
            if(!parseSectionHeader(hdr,QStringLiteral("AXES"),&n)) continue;
            for(int i=0;i<n;++i){
                QStringList f=s.readLine().split(','); bool ok[5]; AxisDirection dir;
                if(f.size()<8||!parseAx(f[2].trimmed(),&dir)) continue;
                AxisRecord a; a.id=f[0].trimmed(); a.floor=f[1].trimmed().toInt(&ok[0]); a.direction=dir;
                a.start.setX(f[3].trimmed().toDouble(&ok[1])); a.start.setY(f[4].trimmed().toDouble(&ok[2]));
                a.end.setX  (f[5].trimmed().toDouble(&ok[3])); a.end.setY  (f[6].trimmed().toDouble(&ok[4]));
                a.label = f.mid(7).join(QLatin1Char(','));
                if(a.id.isEmpty()||!ok[0]||!ok[1]||!ok[2]||!ok[3]||!ok[4]||a.floor<1||a.label.isEmpty()) continue;
                doc->addAxis(a); }
        }
    }
    if (err) {
        err->clear();
    }
    return true;
}

bool FloorPlanRepository::save(const FloorPlanDocument &doc, QString *err) const
{
    QSaveFile file(filePath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){ if(err) *err=QStringLiteral("Cannot write %1").arg(filePath); return false; }
    QTextStream s(&file);
    s << "FLOOR_PLAN_V1\n";

    s << "WALLS," << doc.walls().size() << '\n';
    for(const WallRecord &w:doc.walls())
        s<<w.id<<','<<w.floor<<','
         <<QString::number(w.start.x(),'f',3)<<','<<QString::number(w.start.y(),'f',3)<<','
         <<QString::number(w.end.x(),'f',3)<<','<<QString::number(w.end.y(),'f',3)<<'\n';

    s << "LABELS," << doc.roomLabels().size() << '\n';
    for(const RoomLabelRecord &l:doc.roomLabels())
        s<<l.floor<<','<<l.roomId<<','
         <<QString::number(l.position.x(),'f',3)<<','<<QString::number(l.position.y(),'f',3)<<'\n';

    s << "SYMBOLS," << doc.symbols().size() << '\n';
    for(const SymbolRecord &sym:doc.symbols())
        s<<sym.id<<','<<sym.floor<<','<<symName(sym.type)<<','
         <<QString::number(sym.position.x(),'f',3)<<','<<QString::number(sym.position.y(),'f',3)<<','
         <<QString::number(sym.rotation,'f',1)<<'\n';

    s << "TEXTS," << doc.texts().size() << '\n';
    for(const TextRecord &t:doc.texts())
        s<<t.id<<','<<t.floor<<','
         <<QString::number(t.position.x(),'f',3)<<','<<QString::number(t.position.y(),'f',3)<<','<<t.text<<'\n';

    s << "AXES," << doc.axes().size() << '\n';
    for(const AxisRecord &a:doc.axes())
        s<<a.id<<','<<a.floor<<','<<axName(a.direction)<<','
         <<QString::number(a.start.x(),'f',3)<<','<<QString::number(a.start.y(),'f',3)<<','
         <<QString::number(a.end.x(),'f',3)<<','<<QString::number(a.end.y(),'f',3)<<','<<a.label<<'\n';

    if(!file.commit()){ if(err) *err=QStringLiteral("Commit failed"); return false; }
    if (err) {
        err->clear();
    }
    return true;
}
