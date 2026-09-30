#include "User.h"
#include "ui_User.h"
#include "RoomLayoutPage.h"

void User::showFloorPlan()
{
    floorPlanPage = new RoomLayoutPage(this);
    floorPlanPage->setViewOnlyMode();
    const int idx = ui->stackedWidget_7->addWidget(floorPlanPage);

    auto switchToFloorPlan = [this, idx]() {
        ui->stackedWidget_7->setCurrentIndex(idx);
        floorPlanPage->reloadRooms();
    };

    connect(ui->floorPlanBtn,     &QPushButton::clicked, this, switchToFloorPlan);
    connect(ui->floorPlanBtnIcon, &QPushButton::clicked, this, switchToFloorPlan);

    connect(ui->floorPlanBtn,     &QPushButton::toggled,
            ui->floorPlanBtnIcon, &QPushButton::setChecked);
    connect(ui->floorPlanBtnIcon, &QPushButton::toggled,
            ui->floorPlanBtn,     &QPushButton::setChecked);
}
