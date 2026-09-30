#include "RoomLayoutPage.h"
#include "ui_RoomLayoutPage.h"

#include "AxisGraphicsItem.h"
#include "FloorPlanCommands.h"
#include "FloorPlanRepository.h"
#include "GripItem.h"
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

enum class SceneItemKind { None, Grip, Wall, RoomLabel, Symbol, Text, Axis };

struct SceneItemInfo {
    SceneItemKind kind = SceneItemKind::None;
    QString id;

    bool isDeletable() const
    {
        return kind == SceneItemKind::Wall || kind == SceneItemKind::Symbol
               || kind == SceneItemKind::Text || kind == SceneItemKind::Axis;
    }
};

SceneItemInfo inspectSceneItem(QGraphicsItem *item)
{
    if (dynamic_cast<GripItem *>(item)) return {SceneItemKind::Grip, {}};
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

QVector<SceneItemInfo> selectedSceneItemInfos(QGraphicsScene *scene)
{
    QVector<SceneItemInfo> selected;
    if (!scene) return selected;

    const auto items = scene->selectedItems();
    selected.reserve(items.size());
    for (QGraphicsItem *item : items)
        selected.append(inspectSceneItem(item));
    return selected;
}
}

template <typename Command, typename... Args>
void RoomLayoutPage::pushCommand(Args &&...args)
{
    undoStack.push(std::make_unique<Command>(
        &document,
        std::forward<Args>(args)...,
        [this]() { renderFloor(currentFloor()); }));
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
    connect(ui->orthoButton, &QPushButton::toggled, this, [this](bool on){
        ui->orthoButton->setText(on ? QStringLiteral("ORTHO: ON") : QStringLiteral("ORTHO: OFF"));
        ui->view->setOrthoEnabled(on); updateEditorStatus();});
    connect(ui->osnapButton, &QPushButton::toggled, this, [this](bool on){
        ui->osnapButton->setText(on ? QStringLiteral("OSNAP: ON") : QStringLiteral("OSNAP: OFF"));
        ui->view->setOsnapEnabled(on); updateEditorStatus();});
    connect(ui->deleteButton,        &QPushButton::clicked, this, &RoomLayoutPage::deleteSelectedItems);
    connect(ui->unassignRoomButton,  &QPushButton::clicked, this, &RoomLayoutPage::unassignSelectedRoomLabel);
    connect(ui->undoButton,  &QPushButton::clicked, this, [this]{ undoStack.undo(); });
    connect(ui->redoButton,  &QPushButton::clicked, this, [this]{ undoStack.redo(); });
    connect(ui->saveButton,  &QPushButton::clicked, this, &RoomLayoutPage::saveLayout);
    connect(ui->floorSelector, &QComboBox::currentIndexChanged, this, [this](int){
        renderFloor(currentFloor(), true);});
    connect(scene, &QGraphicsScene::selectionChanged,
            this, &RoomLayoutPage::updateSelectionActions);
    undoStack.setOnChange([this]() {
        updateEditorStatus();
        if (!documentLoaded || undoStack.isClean() || autosaveScheduled) return;

        autosaveScheduled = true;
        QTimer::singleShot(0, this, [this]() {
            autosaveScheduled = false;
            if (!undoStack.isClean()) saveLayout();
        });
    });

    ui->view->setWallCreatedHandler([this](QPointF s, QPointF e){ addWall(s, e); });
    ui->view->setRoomLabelPositionHandler([this](QPointF p){ placeRoomLabel(p); });
    ui->view->setSymbolPlacedHandler([this](SymbolType t, QPointF p){ placeSymbol(t, p); });
    ui->view->setTextPositionHandler([this](QPointF p){ placeTextAnnotation(p); });
    ui->view->setAxisCreatedHandler([this](AxisDirection d, QPointF s, QPointF e){ placeAxis(d, s, e); });
    ui->view->setGripReleasedHandler([this](QString id, GripOwnerType ot, bool isSt, QPointF pos){
        handleGripReleased(id, ot, isSt, pos);});
    ui->view->setItemsMovedHandler([this](QVector<QGraphicsItem*> items, QPointF delta){
        handleItemsMoved(items, delta);});
    ui->view->setRequestMoveHandler([this](){
        setTool(FloorPlanTool::Select);});
    const auto sc=[this](QKeySequence seq, std::function<void()> fn){
        auto *s=new QShortcut(seq,this); s->setContext(Qt::WidgetWithChildrenShortcut);
        connect(s, &QShortcut::activated, this, [this, fn=std::move(fn)](){ if(editMode) fn(); });};
    sc(QKeySequence(Qt::Key_F3), [this](){ ui->osnapButton->toggle(); });
    sc(QKeySequence(Qt::Key_F8), [this](){ ui->orthoButton->toggle(); });
    sc(QKeySequence::Undo, [this](){ undoStack.undo(); });
    sc(QKeySequence::Redo, [this](){ undoStack.redo(); });
    sc(QKeySequence::Copy,  [this](){ copySelectedItems(); });
    sc(QKeySequence::Paste, [this](){ pasteCopiedItems(); });
    sc(QKeySequence(Qt::Key_Delete), [this](){ deleteSelectedItems(); });
    sc(QKeySequence(Qt::CTRL | Qt::Key_A), [this](){
        for(auto *item : scene->items()){
            if(inspectSceneItem(item).isDeletable()) item->setSelected(true);
        }
    });
    sc(QKeySequence(Qt::Key_Escape), [this](){
        ui->view->cancelTransient(); scene->clearSelection();
        if(currentTool != FloorPlanTool::Select) setTool(FloorPlanTool::Select);});
    sc(QKeySequence(Qt::Key_R), [this](){
        QVector<QPair<QString, qreal>> changes;
        for(const SceneItemInfo &info : selectedSceneItemInfos(scene)){
            if(info.kind == SceneItemKind::Symbol){
                SymbolRecord r; if(document.findSymbol(info.id, &r))
                    changes.append({r.id, r.rotation + 90.0});
            }
        }
        if(!changes.isEmpty())
            pushCommand<RotateSymbolsCommand>(changes);
    });

    ui->deleteButton->setEnabled(false); ui->unassignRoomButton->setEnabled(false);
    ui->undoButton->setEnabled(false);   ui->redoButton->setEnabled(false);
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
    documentLoaded=true; undoStack.clear(); undoStack.setClean();
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
    activeGrips.clear();
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
    pushCommand<DrawWallCommand>(w);
}

void RoomLayoutPage::placeRoomLabel(const QPointF &pos)
{
    const QString id=ui->roomSelector->currentData().toString();
    if(id.isEmpty()){ QMessageBox::information(this,QStringLiteral("Assign Room"),QStringLiteral("No unassigned room.")); return; }
    RoomLabelRecord l; l.floor=currentFloor(); l.roomId=id; l.position=pos;
    pushCommand<PlaceRoomLabelCommand>(l);
    setTool(FloorPlanTool::Select);
}

void RoomLayoutPage::placeSymbol(SymbolType type, const QPointF &pos)
{
    SymbolRecord s; s.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.floor=currentFloor(); s.type=type; s.position=pos;
    pushCommand<PlaceSymbolCommand>(s);
}

void RoomLayoutPage::placeTextAnnotation(const QPointF &pos)
{
    bool ok=false;
    const QString txt=QInputDialog::getText(this,QStringLiteral("Add Text"),QStringLiteral("Label:"),
                                            QLineEdit::Normal,QString(),&ok).trimmed();
    if(!ok||txt.isEmpty()) return;
    TextRecord t; t.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    t.floor=currentFloor(); t.position=pos; t.text=txt;
    pushCommand<PlaceTextAnnotationCommand>(t);
    setTool(FloorPlanTool::Select);
}

void RoomLayoutPage::placeAxis(AxisDirection dir, const QPointF &start, const QPointF &end)
{
    const QString label = suggestAxisLabel(dir);
    if (label.isEmpty()) return;
    AxisRecord a;
    a.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    a.floor = currentFloor();
    a.direction = dir;
    a.start = start;
    a.end = end;
    a.label = label;

    pushCommand<PlaceAxisCommand>(a);
}

void RoomLayoutPage::handleGripReleased(const QString &ownerId, GripOwnerType ownerType, bool isStart, const QPointF &newPos)
{
    if(ownerType==GripOwnerType::Wall){
        WallRecord rec; if(!document.findWall(ownerId,&rec)) return;
        const QPointF ns=isStart?newPos:rec.start, ne=isStart?rec.end:newPos;
        if(ns==rec.start&&ne==rec.end) return;
        pushCommand<EditWallCommand>(ownerId, rec.start, rec.end, ns, ne);
    } else {
        AxisRecord rec; if(!document.findAxis(ownerId,&rec)) return;
        QPointF ns=isStart?newPos:rec.start, ne=isStart?rec.end:newPos;
        if(ownerType==GripOwnerType::HAxis){ ns.setY(rec.end.y()); ne.setY(rec.end.y()); }
        else                               { ns.setX(rec.end.x()); ne.setX(rec.end.x()); }
        if(ns==rec.start&&ne==rec.end) return;
        pushCommand<EditAxisCommand>(ownerId, rec.start, rec.end, ns, ne);
    }
}

void RoomLayoutPage::handleItemsMoved(QVector<QGraphicsItem *> items, QPointF delta)
{
    if(items.isEmpty()||delta.manhattanLength()<0.01) return;
    QVector<ItemMoveRecord> moves;
    const int floor=currentFloor();
    for(auto *item:items){
        const SceneItemInfo info = inspectSceneItem(item);
        if(info.kind == SceneItemKind::Wall){
            WallRecord r; if(!document.findWall(info.id,&r)) continue;
            ItemMoveRecord m; m.type=ItemMoveRecord::Type::Wall; m.id=r.id;
            m.oldS=r.start; m.oldE=r.end; m.newS=r.start+delta; m.newE=r.end+delta;
            moves<<m;
        } else if(info.kind == SceneItemKind::Axis){
            AxisRecord r; if(!document.findAxis(info.id,&r)) continue;
            ItemMoveRecord m; m.type=ItemMoveRecord::Type::Axis; m.id=r.id;
            m.oldS=r.start; m.oldE=r.end;
            if(r.direction==AxisDirection::Horizontal){ m.newS=r.start+QPointF(delta.x(),0); m.newE=r.end+QPointF(delta.x(),0); }
            else                                       { m.newS=r.start+QPointF(0,delta.y()); m.newE=r.end+QPointF(0,delta.y()); }
            moves<<m;
        } else if(info.kind == SceneItemKind::Symbol){
            SymbolRecord r; if(!document.findSymbol(info.id,&r)) continue;
            ItemMoveRecord m; m.type=ItemMoveRecord::Type::Symbol; m.id=r.id;
            m.oldPos=r.position; m.newPos=r.position+delta; moves<<m;
        } else if(info.kind == SceneItemKind::Text){
            TextRecord r; if(!document.findText(info.id,&r)) continue;
            ItemMoveRecord m; m.type=ItemMoveRecord::Type::Text; m.id=r.id;
            m.oldPos=r.position; m.newPos=r.position+delta; moves<<m;
        } else if(info.kind == SceneItemKind::RoomLabel){
            RoomLabelRecord r; if(!document.findRoomLabel(info.id,&r)||r.floor!=floor) continue;
            ItemMoveRecord m; m.type=ItemMoveRecord::Type::RoomLabel; m.id=r.roomId;
            m.oldPos=r.position; m.newPos=r.position+delta; moves<<m;
        }
    }
    if(moves.isEmpty()) return;
    pushCommand<MoveItemsCommand>(moves);
}

void RoomLayoutPage::deleteSelectedItems()
{
    QVector<WallRecord> w; QVector<SymbolRecord> s; QVector<TextRecord> t; QVector<AxisRecord> a;
    for(const SceneItemInfo &info : selectedSceneItemInfos(scene)){
        if(info.kind == SceneItemKind::Wall)       { WallRecord   r; if(document.findWall(info.id,&r))   w<<r; }
        else if(info.kind == SceneItemKind::Symbol){ SymbolRecord r; if(document.findSymbol(info.id,&r)) s<<r; }
        else if(info.kind == SceneItemKind::Text)  { TextRecord   r; if(document.findText(info.id,&r))   t<<r; }
        else if(info.kind == SceneItemKind::Axis)  { AxisRecord   r; if(document.findAxis(info.id,&r))   a<<r; }
    }
    if(w.isEmpty()&&s.isEmpty()&&t.isEmpty()&&a.isEmpty()) return;
    pushCommand<DeleteEditablesCommand>(w, s, t, a);
}

void RoomLayoutPage::unassignSelectedRoomLabel()
{
    for(const SceneItemInfo &info : selectedSceneItemInfos(scene)){
        if(info.kind != SceneItemKind::RoomLabel) continue;
        RoomLabelRecord r; if(!document.findRoomLabel(info.id,&r)) continue;
        pushCommand<UnassignRoomLabelCommand>(r);
        return;
    }
}

void RoomLayoutPage::copySelectedItems()
{
    clipboardWalls.clear();
    clipboardSymbols.clear();
    clipboardTexts.clear();
    clipboardAxes.clear();
    pasteCount = 0;

    for (const SceneItemInfo &info : selectedSceneItemInfos(scene)) {
        if (info.kind == SceneItemKind::Wall) {
            WallRecord record;
            if (document.findWall(info.id, &record)) clipboardWalls.append(record);
        } else if (info.kind == SceneItemKind::Symbol) {
            SymbolRecord record;
            if (document.findSymbol(info.id, &record)) clipboardSymbols.append(record);
        } else if (info.kind == SceneItemKind::Text) {
            TextRecord record;
            if (document.findText(info.id, &record)) clipboardTexts.append(record);
        } else if (info.kind == SceneItemKind::Axis) {
            AxisRecord record;
            if (document.findAxis(info.id, &record)) clipboardAxes.append(record);
        }
    }
}

void RoomLayoutPage::pasteCopiedItems()
{
    if (clipboardWalls.isEmpty() && clipboardSymbols.isEmpty()
        && clipboardTexts.isEmpty() && clipboardAxes.isEmpty()) {
        return;
    }

    ++pasteCount;
    const int floor = currentFloor();
    const QPointF offset(20.0 * pasteCount, 20.0 * pasteCount);
    QVector<WallRecord> walls = clipboardWalls;
    QVector<SymbolRecord> symbols = clipboardSymbols;
    QVector<TextRecord> texts = clipboardTexts;
    QVector<AxisRecord> axes = clipboardAxes;

    for (WallRecord &wall : walls) {
        wall.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        wall.floor = floor;
        wall.start += offset;
        wall.end += offset;
    }
    for (SymbolRecord &symbol : symbols) {
        symbol.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        symbol.floor = floor;
        symbol.position += offset;
    }
    for (TextRecord &text : texts) {
        text.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        text.floor = floor;
        text.position += offset;
    }
    for (AxisRecord &axis : axes) {
        axis.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        axis.floor = floor;
        axis.start += offset;
        axis.end += offset;
    }

    pushCommand<PasteEditablesCommand>(walls, symbols, texts, axes);
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
    undoStack.setClean();
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
    const QString st=undoStack.isClean()?QStringLiteral("Saved"):QStringLiteral("Unsaved");
    ui->editorStatusLabel->setText(QStringLiteral("%1 | %2").arg(tool,st));
    ui->undoButton->setEnabled(undoStack.canUndo()); ui->redoButton->setEnabled(undoStack.canRedo());
    ui->saveButton->setEnabled(!undoStack.isClean());
}

void RoomLayoutPage::updateSelectionActions()
{
    if (!editMode || currentTool != FloorPlanTool::Select) return;

    bool hasDeletable = false;
    bool hasRoomLabel = false;
    for (const SceneItemInfo &info : selectedSceneItemInfos(scene)) {
        hasDeletable |= info.isDeletable();
        hasRoomLabel |= info.kind == SceneItemKind::RoomLabel;
    }

    ui->deleteButton->setEnabled(hasDeletable);
    ui->unassignRoomButton->setEnabled(hasRoomLabel);
    refreshGrips();
}

void RoomLayoutPage::updateItemEditability()
{
    const bool ed=editMode&&currentTool==FloorPlanTool::Select;
    for(auto *item:scene->items()) setSceneItemEditable(item, ed);
    ui->deleteButton->setEnabled(false); ui->unassignRoomButton->setEnabled(false);
    if(!ed){ activeGrips.clear(); }
}

void RoomLayoutPage::refreshGrips()
{
    for(auto *g:activeGrips){ if(g->scene()) scene->removeItem(g); delete g; }
    activeGrips.clear();
    if(!editMode||currentTool!=FloorPlanTool::Select) return;

    const auto addGrip=[&](const QString &ownerId, GripOwnerType ot, GripRole role, QPointF ownPos, QPointF otherPos){
        auto *g=new GripItem(ownerId,ot,role,otherPos);
        g->setPos(ownPos); scene->addItem(g); activeGrips<<g; };

    for(const SceneItemInfo &info : selectedSceneItemInfos(scene)){
        if(info.kind == SceneItemKind::Wall){
            WallRecord r; if(!document.findWall(info.id,&r)) continue;
            addGrip(r.id,GripOwnerType::Wall,GripRole::StartPoint,r.start,r.end);
            addGrip(r.id,GripOwnerType::Wall,GripRole::EndPoint,  r.end,  r.start);
        } else if(info.kind == SceneItemKind::Axis){
            AxisRecord r; if(!document.findAxis(info.id,&r)) continue;
            const GripOwnerType ot=r.direction==AxisDirection::Horizontal?GripOwnerType::HAxis:GripOwnerType::VAxis;
            addGrip(r.id,ot,GripRole::StartPoint,r.start,r.end);
            addGrip(r.id,ot,GripRole::EndPoint,  r.end,  r.start);
        }
    }
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
