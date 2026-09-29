#include "admin.h"
#include "ui_admin.h"
#include "Signin.h"
#include "Room.h"
#include "RoomType.h"
#include "Tenant.h"
#include "Service.h"
#include "ServiceUsage.h"
#include "Reservation.h"
#include "Contract.h"
#include "Payment.h"
#include "Account.h"
#include "RoomLayoutPage.h"

using namespace std;

Admin::Admin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Admin)
{
    ui->setupUi(this);

    roomLayoutPage = new RoomLayoutPage(this);
    ui->stackedWidget->addWidget(roomLayoutPage);

    QIcon roomLayoutIcon;
    roomLayoutIcon.addFile(QStringLiteral(":/new/prefix1/Resources/Room.png"),
                           QSize(),
                           QIcon::Normal,
                           QIcon::Off);
    roomLayoutIcon.addFile(QStringLiteral(":/new/prefix1/Resources/Room1.png"),
                           QSize(),
                           QIcon::Normal,
                           QIcon::On);

    auto *compactLayoutButton = new QPushButton(ui->MI_AD);
    compactLayoutButton->setObjectName(QStringLiteral("RoomLayoutButtonCompact"));
    compactLayoutButton->setToolTip(QStringLiteral("Room Floor Plan"));
    compactLayoutButton->setIcon(roomLayoutIcon);
    compactLayoutButton->setIconSize(QSize(30, 18));
    compactLayoutButton->setMinimumHeight(30);
    compactLayoutButton->setCheckable(true);
    compactLayoutButton->setAutoExclusive(true);
    compactLayoutButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; }"
        "QPushButton:checked { background: white; border-radius: 3px; }"));
    ui->verticalLayout_2->addWidget(compactLayoutButton);

    auto *expandedLayoutButton = new QPushButton(QStringLiteral("Room Floor Plan"), ui->MIT_AD);
    expandedLayoutButton->setObjectName(QStringLiteral("RoomLayoutButton"));
    expandedLayoutButton->setToolTip(QStringLiteral("Room Floor Plan"));
    expandedLayoutButton->setIcon(roomLayoutIcon);
    expandedLayoutButton->setIconSize(QSize(24, 18));
    expandedLayoutButton->setMinimumSize(186, 30);
    expandedLayoutButton->setCheckable(true);
    expandedLayoutButton->setAutoExclusive(true);
    expandedLayoutButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: white; border: none; text-align: left; padding-left: 20px; }"
        "QPushButton:checked { color: #3fd391; background: white; border-radius: 3px; }"));
    ui->verticalLayout_4->addWidget(expandedLayoutButton);

    const auto showRoomLayout = [this, compactLayoutButton, expandedLayoutButton]() {
        roomLayoutPage->reloadRooms();
        ui->stackedWidget->setCurrentWidget(roomLayoutPage);
        compactLayoutButton->setChecked(true);
        expandedLayoutButton->setChecked(true);
    };
    connect(compactLayoutButton, &QPushButton::clicked, this, showRoomLayout);
    connect(expandedLayoutButton, &QPushButton::clicked, this, showRoomLayout);

    ui->Accbtn->setText(QString::fromStdString("    " + Account::currentTenantID));
    ui->MI_AD->setHidden(true);
    managerooms();
    managetenants();
    manageservices();
    manageserviceusages();
    managereservations();
    managepayments();
    managecontracts();
    manageaccounts();
    manageroomtypes();
    managesta();
    AccandNotipopup();
    ui->roombtn->click();
    ui->stackedWidget_2->setCurrentIndex(1);

    // connect(ui->LineEditSearchTenant, &QLineEdit::returnPressed, this, [this]() {
    //     ui->searchtenant->click();
    // });
    // connect(ui->LineEditSearchRoom, &QLineEdit::returnPressed, this, [this]() {
    //     ui->searchroom->click();
    // });
    // Nofitication
    connect(ui->LineEditSearchPayment, &QLineEdit::returnPressed, this, [this]() {ui->searchPayment->click();});
    connect(ui->LineEditSearchRoom, &QLineEdit::textChanged, this, &Admin::searchroom);
    connect(ui->LineEditSearchTenant, &QLineEdit::textChanged, this, &Admin::searchtenant);
    connect(ui->LineEditSearchSer, &QLineEdit::textChanged, this, &Admin::searchSer);
    connect(ui->LineEditSearchSerUsage, &QLineEdit::textChanged, this, &Admin::searchSerUsage);
    connect(ui->LineEditSearchRe, &QLineEdit::textChanged, this, &Admin::searchRe);
    connect(ui->LineEditSearchRoomType, &QLineEdit::textChanged, this, &Admin::searchRoomType);
    connect(ui->TK3year, &QLineEdit::returnPressed, this, [this]() {ui->TKbtn->click();});
    connect(ui->TK2year1, &QLineEdit::returnPressed, this, [this]() {ui->TKbtn->click();});
    connect(ui->TK2year2, &QLineEdit::returnPressed, this, [this]() {ui->TKbtn->click();});
}

Admin::~Admin()
{
    delete ui;
    delete animation;
}

void Admin::on_pushButton_5_clicked()
{
    // Admin::moverMenu();
}

void Admin::on_signoutbtn1_clicked()
{
    Signin *login = new Signin();
    login->show();
    this->close();
    updateAllFile();
}

void Admin::on_signoutbtn_clicked()
{
    Signin *login = new Signin();
    login->show();
    this->close();
    updateAllFile();
}

void Admin::AccandNotipopup() {
    //Account
    QFrame *AccPopup = new QFrame(this);
    AccPopup->setFrameShape(QFrame::StyledPanel);
    AccPopup->setFixedSize(150, 150);
    AccPopup->setStyleSheet("background-color: white; border: 1px solid gray; border-radius: 5px;");
    AccPopup->hide();

    QVBoxLayout *popupLayout = new QVBoxLayout(AccPopup);

    QVBoxLayout *buttonsLayout = new QVBoxLayout();

    QPushButton *changeAdminCodeBtn = new QPushButton("Change AdminCode", AccPopup);
    changeAdminCodeBtn->setStyleSheet("border: none; padding: 10px;text-align: left;");
    buttonsLayout->addWidget(changeAdminCodeBtn);

    QPushButton *changePasswordBtn = new QPushButton("Change Password", AccPopup);
    changePasswordBtn->setStyleSheet("border: none; padding: 10px;text-align: left;");
    buttonsLayout->addWidget(changePasswordBtn);

    QFrame *line = new QFrame(AccPopup);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    buttonsLayout->addWidget(line);

    QPushButton *logoutBtn = new QPushButton("Sign Out", AccPopup);
    logoutBtn->setStyleSheet("border: none; padding: 10px;text-align: left;");
    buttonsLayout->addWidget(logoutBtn);

    popupLayout->addLayout(buttonsLayout);

    popupLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    AccPopup->move(this->mapToGlobal(QPoint(50, 50)));
    // Notification
    QFrame *notificationPopup = new QFrame(this);
    notificationPopup->setFrameShape(QFrame::StyledPanel);
    notificationPopup->setFixedSize(250, 300);
    notificationPopup->setStyleSheet("background-color: white; border: 1px solid gray; border-radius: 5px;");
    notificationPopup->hide();

    QVBoxLayout *popupLayout1 = new QVBoxLayout(notificationPopup);
    QLabel *titleLabel = new QLabel("Notifications");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(40);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: purple; background-color: white; border: none;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    popupLayout1->addWidget(titleLabel);

    QWidget *notificationList = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(notificationList);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(10);

    int check = Reservation::checkNewReservations();
    if ( check!=0 ){
        ui->notifyButton->setIcon(QIcon(":/new/prefix1/Resources/nofitication2.png"));
        QHBoxLayout *notificationLayout = new QHBoxLayout();

        QLabel *messageLabel1 = new QLabel("New Reservation");
        messageLabel1->setStyleSheet("color: #7A4D9C; margin-left: 10px; border: none;");
        popupLayout1->addWidget(messageLabel1);

        listLayout->addLayout(notificationLayout);
        QLabel *label = new QLabel("There are " + QString::number(check) + " new reservation requests. Please review and decide.");
        label->setStyleSheet("margin-left: 10px; font-size: 14px; border: none;");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        popupLayout1->addWidget(label);
    }
    popupLayout1->addWidget(notificationList);
    popupLayout1->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    connect(ui->notifyButton, &QPushButton::clicked, this, [=]() {
        ui->notifyButton->setIcon(QIcon(":/new/prefix1/Resources/nofitication.png"));
        if (AccPopup->isVisible()) {
            AccPopup->hide();
        }
        if (notificationPopup->isVisible()) {
            notificationPopup->hide();
        } else {
            QPoint buttonGlobalPos = ui->notifyButton->mapToGlobal(QPoint(0, ui->notifyButton->height()));
            QPoint popupParentPos = this->mapFromGlobal(buttonGlobalPos);

            notificationPopup->move(popupParentPos - QPoint(notificationPopup->width() - ui->notifyButton->width(), 0));
            notificationPopup->show();
        }
    });
    connect(ui->Accbtn, &QPushButton::clicked, this, [=]() {
        if (notificationPopup->isVisible()) {
            notificationPopup->hide();
        }
        if (AccPopup->isVisible()) {
            AccPopup->hide();
        } else {
            QPoint buttonGlobalPos = ui->Accbtn->mapToGlobal(QPoint(0, ui->Accbtn->height()));
            QPoint popupParentPos = this->mapFromGlobal(buttonGlobalPos);
            AccPopup->move(popupParentPos);

            // AccPopup->move(popupParentPos - QPoint(AccPopup->width() - ui->Accbtn->width(), 0));
            AccPopup->show();
        }
        changeAdminCodeBtn->setCursor(Qt::PointingHandCursor);
        changePasswordBtn->setCursor(Qt::PointingHandCursor);
        logoutBtn->setCursor(Qt::PointingHandCursor);
    });
    connect(changeAdminCodeBtn, &QPushButton::clicked, this, [this]() {
        changeAdmincode();
    });

    connect(changePasswordBtn, &QPushButton::clicked, this, [this]() {
        changePassword();
    });

    connect(logoutBtn, &QPushButton::clicked, this, [this]() {
        ui->signoutbtn->click();
    });
}

void Admin::moverMenu()
{
    int width = ui->MIT_AD->width();
    int normal = 71;
    int extender;

    if (width == 71) {
        extender = 245;
        QPropertyAnimation *animation = new QPropertyAnimation(ui->MIT_AD, "minimumWidth");
        ui->MI_AD->setVisible(false);
        ui->MIT_AD->setHidden(false);
        animation->setDuration(300);
        animation->setStartValue(width);
        animation->setEndValue(extender);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        extender = normal;
        QPropertyAnimation *animation = new QPropertyAnimation(ui->MIT_AD, "minimumWidth");
        animation->setDuration(300);
        animation->setStartValue(width);
        animation->setEndValue(extender);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            ui->MI_AD->setVisible(true);
            ui->MIT_AD->setHidden(true);
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);

    }
}

void Admin::updateAllFile() {
    RoomType::updateFile("RoomType.txt");
    Room::updateFile("Room.txt");
    Tenant::updateFile("Tenant.txt");
    Service::updateFile("Service.txt");
    ServiceUsage::updateFile("ServiceUsage.txt");
    Reservation::updateFile("Reservation.txt");
    Contract::updateFile("Contract.txt");
    Payment::updateFile("Payment.txt");
    Account::updateFile("Account.txt");
}
