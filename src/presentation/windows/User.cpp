#include "User.h"
#include "ui_User.h"
#include "Signin.h"
#include "Account.h"
#include "Contract.h"
#include "Payment.h"
#include "Reservation.h"
#include "Room.h"
#include "RoomType.h"
#include "Service.h"
#include "ServiceUsage.h"
#include "Tenant.h"

User::User(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::User)
{
    ui->setupUi(this);
    ui->roombtn->click();
    Tenant *tenant = Tenant::tenantList.searchID(Account::currentTenantID);
    // ui->Accbtn->setText(QString::fromStdString("  " + Account::currentTenantID));
    ui->Accbtn->setText(QString::fromStdString("  " + tenant->getFirstName()));

    ui->MI_AD->setHidden(true);
    managerooms();
    manageservices();
    managecontracts();
    managepayments();
    AccandNotipopup();
    showprofile();
    showlist();
    showmyroom();
    showFloorPlan();
    ui->ID->setText("ID: " + QString::fromStdString(Account::currentTenantID));
    // connect(ui->listWidget, &QListWidget::clicked, this, &User::on_listWidget_clicked);
    connect(ui->LineEditSearchRoom, &QLineEdit::textChanged, this, &User::searchroom);
    connect(ui->LineEditSearchSer, &QLineEdit::textChanged, this, &User::searchSer);
}

User::~User()
{
    delete ui;
}

void User::AccandNotipopup() {
    //Account
    QFrame *AccPopup = new QFrame(this);
    AccPopup->setFrameShape(QFrame::StyledPanel);
    AccPopup->setFixedSize(150, 150);
    AccPopup->setStyleSheet("background-color: white; border: 1px solid gray; border-radius: 5px;");
    AccPopup->hide();

    QVBoxLayout *popupLayout = new QVBoxLayout(AccPopup);

    QVBoxLayout *buttonsLayout = new QVBoxLayout();

    QPushButton *changeAdminCodeBtn = new QPushButton("Profile", AccPopup);
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

    int check = Payment::checkUnpaidPayments(Account::currentTenantID);
    if ( check!=0 ){
        ui->notifyButton->setIcon(QIcon(":/new/prefix1/Resources/nofitication2.png"));
        QHBoxLayout *notificationLayout = new QHBoxLayout();

        QLabel *messageLabel1 = new QLabel("Payment");
        messageLabel1->setStyleSheet("color: #7A4D9C; margin-left: 10px; border: none;");
        popupLayout1->addWidget(messageLabel1);

        listLayout->addLayout(notificationLayout);
        QLabel *label = new QLabel("Bạn còn " + QString::number(check) + " VNĐ cần thanh toán.");
        label->setStyleSheet("margin-left: 10px; font-size: 14px; border: none;");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        popupLayout1->addWidget(label);
    }
    changeAdminCodeBtn->setCursor(Qt::PointingHandCursor);
    changePasswordBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setCursor(Qt::PointingHandCursor);
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
            QPoint popupParentPos = this->mapFromGlobal(buttonGlobalPos);\
            AccPopup->move(popupParentPos);

            // AccPopup->move(popupParentPos - QPoint(AccPopup->width() - ui->Accbtn->width(), 0));
            AccPopup->show();
        }
    });
    connect(changeAdminCodeBtn, &QPushButton::clicked, this, [this]() {
        ui->Accbtn->click();
        ui->Accountbtn->click();
        ui->stackedWidget_7->setCurrentIndex(5);
        ui->TTCNbtn->click();
    });
    connect(changePasswordBtn, &QPushButton::clicked, this, [this]() {
        ui->Accbtn->click();
        ui->Accountbtn->click();
        ui->stackedWidget_7->setCurrentIndex(5);
        ui->Changepassbtn->click();
    });
    connect(logoutBtn, &QPushButton::clicked, this, [this]() {
        ui->signoutbtn->click();
    });
}

void User::on_signoutbtn_clicked()
{
    Signin *login = new Signin();
    login->show();
    this->close();
    updateAllFile();
}

void User::on_signoutbtn1_clicked()
{
    Signin *login = new Signin();
    login->show();
    this->close();
    updateAllFile();
}

void User::updateAllFile() {
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

void User::on_Accbtn_2_clicked()
{
    ui->Accbtn->click();
}
