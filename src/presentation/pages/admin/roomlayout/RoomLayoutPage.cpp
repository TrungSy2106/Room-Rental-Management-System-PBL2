#include "RoomLayoutPage.h"
#include "ui_RoomLayoutPage.h"

#include "AxisGraphicsItem.h"
#include "FloorPlanRepository.h"
#include "Room.h"
#include "RoomLabelGraphicsItem.h"
#include "SymbolGraphicsItem.h"
#include "TextAnnotationItem.h"
#include "WallGraphicsItem.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSet>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>
#include <memory>
#include <utility>

namespace {
constexpr qreal SceneW = 1140.0, SceneH = 700.0;
const QString DemoDataFile = QStringLiteral("FloorPlanGeometry.txt");

enum class SceneItemKind { None, Wall, RoomLabel, Symbol, Text, Axis };

struct SceneItemInfo {
    SceneItemKind kind = SceneItemKind::None;
    QString id;
};

SceneItemInfo inspectSceneItem(QGraphicsItem *item)
{
    if (auto *wall = dynamic_cast<WallGraphicsItem *>(item))
        return {SceneItemKind::Wall, wall->id()};
    if (auto *label = dynamic_cast<RoomLabelGraphicsItem *>(item))
        return {SceneItemKind::RoomLabel, label->roomId()};
    if (auto *symbol = dynamic_cast<SymbolGraphicsItem *>(item))
        return {SceneItemKind::Symbol, symbol->symbolId()};
    if (auto *text = dynamic_cast<TextAnnotationItem *>(item))
        return {SceneItemKind::Text, text->textId()};
    if (auto *axis = dynamic_cast<AxisGraphicsItem *>(item))
        return {SceneItemKind::Axis, axis->axisId()};
    return {};
}

void setSceneItemEditable(QGraphicsItem *item, bool editable)
{
    switch (inspectSceneItem(item).kind) {
    case SceneItemKind::Wall:      static_cast<WallGraphicsItem *>(item)->setEditable(editable); break;
    case SceneItemKind::RoomLabel: static_cast<RoomLabelGraphicsItem *>(item)->setEditable(editable); break;
    case SceneItemKind::Symbol:    static_cast<SymbolGraphicsItem *>(item)->setEditable(editable); break;
    case SceneItemKind::Text:      static_cast<TextAnnotationItem *>(item)->setEditable(editable); break;
    case SceneItemKind::Axis:      static_cast<AxisGraphicsItem *>(item)->setEditable(editable); break;
    default: break;
    }
}
}

RoomLayoutPage::RoomLayoutPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RoomLayoutPage)
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    ui->view->setScene(scene);

    auto *legendLayout = new QHBoxLayout(ui->legendItemsContainer);
    legendLayout->setContentsMargins(0,0,0,0); legendLayout->setSpacing(18);
    legendLayout->addWidget(createLegendItem(QColor(49,190,126),  QStringLiteral("Available")));
    legendLayout->addWidget(createLegendItem(QColor(231,76,72),   QStringLiteral("Occupied")));
    legendLayout->addWidget(createLegendItem(QColor(245,166,35),  QStringLiteral("Reserved")));
    legendLayout->addWidget(createLegendItem(QColor(126,91,180),  QStringLiteral("Maintenance")));

    auto *toolGroup = new QButtonGroup(this); toolGroup->setExclusive(true);
    for (QPushButton *b : {ui->selectButton, ui->drawWallButton, ui->doorButton,
                            ui->stairButton, ui->textButton, ui->hAxisButton,
                            ui->vAxisButton, ui->placeRoomButton}) {
        b->setCheckable(true); toolGroup->addButton(b);
    }
    ui->selectButton->setChecked(true);

    connect(ui->resetBtn,       &QPushButton::clicked, ui->view, &RoomLayoutView::resetView);
    connect(ui->addFloorButton, &QPushButton::clicked, this, &RoomLayoutPage::addFloor);
    connect(ui->editModeButton, &QPushButton::toggled, this, &RoomLayoutPage::setEditMode);
    connect(ui->selectButton,   &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::Select);});
    connect(ui->drawWallButton, &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::DrawWall);});
    connect(ui->doorButton,     &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceDoor);});
    connect(ui->stairButton,    &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceStair);});
    connect(ui->textButton,     &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceText);});
    connect(ui->hAxisButton,    &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceHAxis);});
    connect(ui->vAxisButton,    &QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceVAxis);});
    connect(ui->placeRoomButton,&QPushButton::clicked, this, [this](){setTool(FloorPlanTool::PlaceRoomLabel);});
    connect(ui->deleteButton,        &QPushButton::clicked, this, &RoomLayoutPage::deleteSelectedItems);
    connect(ui->unassignRoomButton,  &QPushButton::clicked, this, &RoomLayoutPage::unassignSelectedRoomLabel);
    connect(ui->saveButton,  &QPushButton::clicked, this, &RoomLayoutPage::saveLayout);
    connect(ui->floorSelector, &QComboBox::currentIndexChanged, this, [this](int){
        renderFloor(currentFloor(), true);});
    connect(scene, &QGraphicsScene::selectionChanged,
            this, &RoomLayoutPage::updateSelectionActions);

    ui->view->setWallCreatedHandler([this](QPointF s, QPointF e){ addWall(s, e); });
    ui->view->setRoomLabelPositionHandler([this](QPointF p){ placeRoomLabel(p); });
    ui->view->setSymbolPlacedHandler([this](SymbolType t, QPointF p){ placeSymbol(t, p); });
    ui->view->setTextPositionHandler([this](QPointF p){ placeTextAnnotation(p); });
    ui->view->setAxisCreatedHandler([this](AxisDirection d, QPointF s, QPointF e){ placeAxis(d, s, e); });

    const auto sc=[this](QKeySequence seq, std::function<void()> fn){
        auto *s=new QShortcut(seq,this); s->setContext(Qt::WidgetWithChildrenShortcut);
        connect(s, &QShortcut::activated, this, [this, fn=std::move(fn)](){ if(editMode) fn(); });};
    sc(QKeySequence(Qt::Key_Delete), [this](){ deleteSelectedItems(); });
    sc(QKeySequence(Qt::CTRL | Qt::Key_A), [this](){
        for(auto *item : scene->items()){
            if(inspectSceneItem(item).kind != SceneItemKind::None
               && inspectSceneItem(item).kind != SceneItemKind::RoomLabel)
                item->setSelected(true);
        }
    });
    sc(QKeySequence(Qt::Key_Escape), [this](){
        ui->view->cancelTransient(); scene->clearSelection();
        if(currentTool != FloorPlanTool::Select) setTool(FloorPlanTool::Select);});

    ui->deleteButton->setEnabled(false); ui->unassignRoomButton->setEnabled(false);
    reloadRooms();
}

RoomLayoutPage::~RoomLayoutPage()
{
    delete ui;
}

void RoomLayoutPage::reloadRooms()
{
    const int prev = currentFloor();
    if (!documentLoaded) loadFloorPlan();
    QSet<int> disc;
    for(const WallRecord     &w:document.walls())      disc.insert(w.floor);
    for(const RoomLabelRecord&l:document.roomLabels()) disc.insert(l.floor);
    for(const SymbolRecord   &s:document.symbols())    disc.insert(s.floor);
    for(const TextRecord     &t:document.texts())      disc.insert(t.floor);
    for(const AxisRecord     &a:document.axes())       disc.insert(a.floor);
    floors=disc.values();
    if(floors.isEmpty()) floors.append(1);
    std::sort(floors.begin(),floors.end());
    refreshFloorSelector(prev);
    renderFloor(currentFloor(), true);
}

void RoomLayoutPage::loadFloorPlan()
{
    bool miss=false; QString err;
    FloorPlanDocument loaded;
    if (!FloorPlanRepository(layoutDataFilePath()).load(&loaded, &miss, &err)) {
        QMessageBox::warning(this, QStringLiteral("Load"), err);
        document.clear();
    } else if (miss) {
        bool demoMissing = false;
        if (!FloorPlanRepository(DemoDataFile).load(
                &loaded, &demoMissing, &err) || demoMissing) {
            if (!err.isEmpty()) {
                QMessageBox::warning(this, QStringLiteral("Load"), err);
            }
            loaded.clear();
        }
        document = loaded;
    } else {
        document = loaded;
    }
    documentLoaded=true;
}

int RoomLayoutPage::currentFloor() const
{
    return ui->floorSelector->currentData().toInt();
}

void RoomLayoutPage::refreshFloorSelector(int sel)
{
    const QSignalBlocker b(ui->floorSelector); ui->floorSelector->clear();
    for(int f:floors) ui->floorSelector->addItem(QStringLiteral("Floor %1").arg(f),f);
    const int idx=ui->floorSelector->findData(sel);
    ui->floorSelector->setCurrentIndex(idx>=0?idx:0);
}

void RoomLayoutPage::refreshRoomSelector()
{
    QSet<QString> placed;
    for(const RoomLabelRecord &l:document.roomLabels()) placed.insert(l.roomId);
    const QString prev=ui->roomSelector->currentData().toString();
    ui->roomSelector->clear();
    LinkedList<Room>::Node *cur=Room::roomList.begin();
    while (cur) {
        const QString id = QString::fromStdString(cur->data.getID());
        if (!placed.contains(id)) {
            ui->roomSelector->addItem(id, id);
        }
        cur = cur->next;
    }
    const int idx=ui->roomSelector->findData(prev);
    ui->roomSelector->setCurrentIndex(idx>=0?idx:0);
}

void RoomLayoutPage::renderFloor(int floor, bool doResetView)
{
    ui->view->cancelTransient();
    {
        const QSignalBlocker blocker(scene);
        scene->clear();
    }
    scene->setSceneRect(0.0,0.0,SceneW,SceneH);

    QHash<QString,const Room*> rById;
    LinkedList<Room>::Node *cur=Room::roomList.begin();
    while(cur){ rById.insert(QString::fromStdString(cur->data.getID()),&cur->data); cur=cur->next; }

    for(const AxisRecord &a:document.axes()){
        if(a.floor!=floor) continue;
        auto *item=new AxisGraphicsItem(a); item->setEditable(editMode&&currentTool==FloorPlanTool::Select);
        scene->addItem(item);
    }

    for(const WallRecord &w:document.walls()){
        if(w.floor!=floor) continue;
        auto *item=new WallGraphicsItem(w); item->setEditable(editMode&&currentTool==FloorPlanTool::Select);
        scene->addItem(item);
    }

    int cnt=0;
    for(const RoomLabelRecord &l:document.roomLabels()){
        if(l.floor!=floor||!rById.contains(l.roomId)) continue;
        const Room *r=rById.value(l.roomId);
        const QString rt=r->getRoomType()?QString::fromStdString(r->getRoomType()->getName()):QStringLiteral("?");
        auto *li=new RoomLabelGraphicsItem(l,rt,r->getStatus());
        li->setEditable(editMode&&currentTool==FloorPlanTool::Select);
        scene->addItem(li); ++cnt; }

    for(const SymbolRecord &s:document.symbols()){
        if(s.floor!=floor) continue;
        auto *item=new SymbolGraphicsItem(s); item->setEditable(editMode&&currentTool==FloorPlanTool::Select);
        scene->addItem(item); }

    for(const TextRecord &t:document.texts()){
        if(t.floor!=floor) continue;
        auto *item=new TextAnnotationItem(t); item->setEditable(editMode&&currentTool==FloorPlanTool::Select);
        scene->addItem(item); }

    ui->roomCountLabel->setText(QStringLiteral("%1 rooms").arg(cnt));
    ui->deleteButton->setEnabled(false); ui->unassignRoomButton->setEnabled(false);
    refreshRoomSelector(); updateEditorStatus();
    if(doResetView) QTimer::singleShot(0, ui->view, [this](){ ui->view->resetView(); });
}

void RoomLayoutPage::setEditMode(bool enabled)
{
    editMode=enabled; ui->editorControls->setVisible(enabled); ui->editorStatusLabel->setVisible(enabled);
    ui->editModeButton->setText(enabled?QStringLiteral("Finish Editing"):QStringLiteral("Edit Layout"));
    ui->view->setEditorEnabled(enabled); setTool(FloorPlanTool::Select);
    ui->view->setFocus(); updateEditorStatus();
}

void RoomLayoutPage::setTool(FloorPlanTool tool)
{
    if(!editMode&&tool!=FloorPlanTool::Select) return;
    currentTool=tool;
    ui->selectButton->setChecked   (tool==FloorPlanTool::Select);
    ui->drawWallButton->setChecked (tool==FloorPlanTool::DrawWall);
    ui->doorButton->setChecked     (tool==FloorPlanTool::PlaceDoor);
    ui->stairButton->setChecked    (tool==FloorPlanTool::PlaceStair);
    ui->textButton->setChecked     (tool==FloorPlanTool::PlaceText);
    ui->hAxisButton->setChecked    (tool==FloorPlanTool::PlaceHAxis);
    ui->vAxisButton->setChecked    (tool==FloorPlanTool::PlaceVAxis);
    ui->placeRoomButton->setChecked(tool==FloorPlanTool::PlaceRoomLabel);
    ui->view->setTool(tool); updateItemEditability(); updateEditorStatus(); ui->view->setFocus();
}

void RoomLayoutPage::addWall(const QPointF &s, const QPointF &e)
{
    WallRecord w; w.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    w.floor=currentFloor(); w.start=s; w.end=e;
    document.addWall(w);
    renderFloor(currentFloor());
}

void RoomLayoutPage::placeRoomLabel(const QPointF &pos)
{
    const QString id=ui->roomSelector->currentData().toString();
    if(id.isEmpty()){ QMessageBox::information(this,QStringLiteral("Assign Room"),QStringLiteral("No unassigned room.")); return; }
    RoomLabelRecord l; l.floor=currentFloor(); l.roomId=id; l.position=pos;
    document.setRoomLabel(l);
    renderFloor(currentFloor());
    setTool(FloorPlanTool::Select);
}

void RoomLayoutPage::placeSymbol(SymbolType type, const QPointF &pos)
{
    SymbolRecord s; s.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.floor=currentFloor(); s.type=type; s.position=pos;
    document.addSymbol(s);
    renderFloor(currentFloor());
}

void RoomLayoutPage::placeTextAnnotation(const QPointF &pos)
{
    bool ok=false;
    const QString txt=QInputDialog::getText(this,QStringLiteral("Add Text"),QStringLiteral("Label:"),
                                            QLineEdit::Normal,QString(),&ok).trimmed();
    if(!ok||txt.isEmpty()) return;
    TextRecord t; t.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    t.floor=currentFloor(); t.position=pos; t.text=txt;
    document.addText(t);
    renderFloor(currentFloor());
    setTool(FloorPlanTool::Select);
}

void RoomLayoutPage::placeAxis(AxisDirection dir, const QPointF &start, const QPointF &end)
{
    const QString label = suggestAxisLabel(dir);
    if (label.isEmpty()) return;
    AxisRecord a;
    a.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    a.floor = currentFloor(); a.direction = dir;
    a.start = start; a.end = end; a.label = label;
    document.addAxis(a);
    renderFloor(currentFloor());
}

void RoomLayoutPage::deleteSelectedItems()
{
    bool changed = false;
    for(QGraphicsItem *item : scene->selectedItems()){
        const SceneItemInfo info = inspectSceneItem(item);
        if(info.kind == SceneItemKind::Wall)        { document.removeWall(info.id);   changed = true; }
        else if(info.kind == SceneItemKind::Symbol) { document.removeSymbol(info.id); changed = true; }
        else if(info.kind == SceneItemKind::Text)   { document.removeText(info.id);   changed = true; }
        else if(info.kind == SceneItemKind::Axis)   { document.removeAxis(info.id);   changed = true; }
    }
    if(changed) renderFloor(currentFloor());
}

void RoomLayoutPage::unassignSelectedRoomLabel()
{
    for(QGraphicsItem *item : scene->selectedItems()){
        const SceneItemInfo info = inspectSceneItem(item);
        if(info.kind != SceneItemKind::RoomLabel) continue;
        document.removeRoomLabel(info.id);
        renderFloor(currentFloor());
        return;
    }
}

void RoomLayoutPage::addFloor()
{
    const int sug=floors.isEmpty()?1:floors.constLast()+1; bool ok=false;
    const int nf=QInputDialog::getInt(this,QStringLiteral("Add Floor"),QStringLiteral("Floor:"),sug,1,99,1,&ok);
    if(!ok) return;
    if(floors.contains(nf)){ QMessageBox::information(this,QStringLiteral("Add Floor"),QStringLiteral("Floor %1 exists.").arg(nf)); return; }
    floors.append(nf); std::sort(floors.begin(),floors.end());
    refreshFloorSelector(nf); renderFloor(nf,true);
}

void RoomLayoutPage::saveLayout()
{
    QString err;
    if(!FloorPlanRepository(layoutDataFilePath()).save(document,&err)){
        QMessageBox::warning(this,QStringLiteral("Save"),err); return; }
}

QString RoomLayoutPage::layoutDataFilePath() const
{
    const QString dataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dataDirectory.isEmpty() && QDir().mkpath(dataDirectory)) {
        return QDir(dataDirectory).filePath(DemoDataFile);
    }
    return QDir::current().filePath(DemoDataFile);
}

void RoomLayoutPage::updateEditorStatus()
{
    const QString tool=
        currentTool==FloorPlanTool::DrawWall      ?QStringLiteral("LINE"):
        currentTool==FloorPlanTool::PlaceRoomLabel?QStringLiteral("ASSIGN ROOM"):
        currentTool==FloorPlanTool::PlaceDoor     ?QStringLiteral("DOOR"):
        currentTool==FloorPlanTool::PlaceStair    ?QStringLiteral("STAIR"):
        currentTool==FloorPlanTool::PlaceText     ?QStringLiteral("TEXT"):
        currentTool==FloorPlanTool::PlaceHAxis    ?QStringLiteral("H AXIS"):
        currentTool==FloorPlanTool::PlaceVAxis    ?QStringLiteral("V AXIS"):
                                                   QStringLiteral("SELECT");
    ui->editorStatusLabel->setText(QStringLiteral("%1 | Unsaved").arg(tool));
    ui->saveButton->setEnabled(editMode);
}

void RoomLayoutPage::updateSelectionActions()
{
    if (!editMode || currentTool != FloorPlanTool::Select) return;
    bool hasDeletable = false, hasRoomLabel = false;
    for(QGraphicsItem *item : scene->selectedItems()){
        const SceneItemInfo info = inspectSceneItem(item);
        hasDeletable |= (info.kind==SceneItemKind::Wall||info.kind==SceneItemKind::Symbol
                         ||info.kind==SceneItemKind::Text||info.kind==SceneItemKind::Axis);
        hasRoomLabel |= info.kind == SceneItemKind::RoomLabel;
    }
    ui->deleteButton->setEnabled(hasDeletable);
    ui->unassignRoomButton->setEnabled(hasRoomLabel);
}

void RoomLayoutPage::updateItemEditability()
{
    const bool ed=editMode&&currentTool==FloorPlanTool::Select;
    for(auto *item:scene->items()) setSceneItemEditable(item, ed);
    ui->deleteButton->setEnabled(false); ui->unassignRoomButton->setEnabled(false);
}

QString RoomLayoutPage::suggestAxisLabel(AxisDirection dir) const
{
    const int floor=currentFloor();
    QStringList exist;
    for(const AxisRecord &a:document.axes()) if(a.floor==floor&&a.direction==dir) exist<<a.label;
    if (dir == AxisDirection::Horizontal) {
        for (char c = 'A'; c <= 'Z'; ++c) {
            const QString s(1, QLatin1Char(c));
            if (!exist.contains(s)) return s;
        }
    } else {
        for (int n = 1; n <= 99; ++n) {
            const QString s = QString::number(n);
            if (!exist.contains(s)) return s;
        }
    }
    return QStringLiteral("X");
}

QWidget *RoomLayoutPage::createLegendItem(const QColor &color, const QString &text)
{
    auto *w=new QWidget(this); auto *l=new QHBoxLayout(w);
    l->setContentsMargins(0,0,0,0); l->setSpacing(7);
    auto *m=new QFrame(w); m->setFixedSize(11,11);
    m->setStyleSheet(QStringLiteral("background:%1;border-radius:5px;").arg(color.name()));
    auto *lbl=new QLabel(text,w); lbl->setStyleSheet(QStringLiteral("color:#d0d0d0;"));
    l->addWidget(m); l->addWidget(lbl); return w;
}

void RoomLayoutPage::setViewOnlyMode()
{
    ui->editModeButton->hide();
    ui->addFloorButton->hide();
}
