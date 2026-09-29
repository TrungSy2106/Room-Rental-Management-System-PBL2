#ifndef ADMIN_H
#define ADMIN_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>

namespace Ui {
class Admin;
}

class Room;
class Tenant;
class Service;
class ServiceUsage;
class Reservation;
class Payment;
class Contract;
class Account;
class RoomType;
class RoomLayoutPage;

class Admin : public QMainWindow
{
    Q_OBJECT
public:
    explicit Admin(QWidget *parent = nullptr);
    ~Admin();

    void managerooms();
    void managetenants();
    void manageservices();
    void manageserviceusages();
    void managereservations();
    void managepayments();
    void managecontracts();
    void manageaccounts();
    void manageroomtypes();
    void managesta();
    void AccandNotipopup();
    void changeAdmincode();
    void changePassword();

    void displayRooms(const Room& room);
    void displayTenants(const Tenant& t);
    void displayServices(const Service& s);
    void displayServiceUsages(const ServiceUsage& su);
    void displayReservations(const Reservation& re);
    void displayPayments(const Payment& p);
    void displayContracts(const Contract& c);
    void displayAccounts(const Account& a);
    void displayRoomTypes(const RoomType& rt);
    void displayMonthlyComparison(int month, double tb, double tc, double t);
    void displayYearlyComparison(int year, double tb, double tc, double t);
    void drawchartMonths(int year);
    void drawchartYears(int start, int end);
    void updateAllFile();
private slots:
    void onExtendButtonClicked(int row);
    void on_pushButton_3_clicked();
    void on_roombtn_clicked();
    void on_addroom_clicked();
    void onEditButtonClicked(int row);
    void onDeleteButtonClicked(int row);
    void ondelete_Ser_btnClicked(int row);
    void ondelete_RT_btnClicked(int row);
    void onedit_tenant_btnClicked(int row);
    void onedit_Ser_btnClicked(int row);
    void onedit_RT_btnClicked(int row);
    void searchroom();
    void searchRe();
    void searchSer();
    void searchSerUsage();
    void searchtenant();
    void searchRoomType();
    void on_CBSR_currentIndexChanged(int index);
    void moverMenu();
    void on_pushButton_5_clicked();
    void on_Refbtn_clicked();
    void on_Tenantbtn_clicked();
    void onacceptbtnClicked(int row);
    void onrejectbtnClicked(int row);
    void on_RefTenantbtn_clicked();
    void on_pushButton_17_clicked();
    void on_Serbtn1_clicked();
    void on_Serbtn_clicked();
    void on_addSer_clicked();
    void on_RefbtnSer_clicked();
    void on_SerUsage1btn_clicked();
    void on_SerUsagebtn_clicked();
    void on_ReservationUsagebtn_clicked();
    void on_Reservation1btn_clicked();
    // void on_searchRer_clicked();
    void on_signoutbtn1_clicked();
    void on_signoutbtn_clicked();
    void onterminate_btnClicked(int row);


    void on_Paymentbtn_clicked();

    void on_Paymentbtn1_clicked();

    void on_Contractbtn_clicked();

    void on_Contractbtn1_clicked();

    void on_searchPayment_clicked();

    void on_RefPaymentbtn_clicked();

    void on_CBST_currentIndexChanged(int index);

    void on_CBSRe_currentIndexChanged(int index);

    void on_Accountbtn_clicked();

    void on_Accountbtn1_clicked();

    void on_RoomTypebtn1_clicked();

    void on_RoomTypebtn_clicked();

    void on_Statisticsbtn1_clicked();

    void on_Statisticsbtn_clicked();

    void on_TKbtn_clicked();

    void on_comboBoxTK_currentIndexChanged(int index);

    void on_CreatePaymentbtn_clicked();

    void on_RefbtnSerusage_clicked();

    void on_RefRebtn_clicked();

    void on_RefRTbtn_clicked();

    void on_addroomtype_clicked();

    void on_Accbtn_2_clicked();

private:
    Ui::Admin *ui;
    QPropertyAnimation *animation;
    QChartView *chartview = nullptr;
    RoomLayoutPage *roomLayoutPage = nullptr;
};

#endif // ADMIN_H
