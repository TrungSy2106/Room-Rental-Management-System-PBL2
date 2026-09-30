#ifndef USER_H
#define USER_H

#include <QMainWindow>

class Room;
class Service;
class Payment;
class Contract;
class RoomLayoutPage;

namespace Ui {
class User;
}

class User : public QMainWindow
{
    Q_OBJECT

public:
    void displayRooms(const Room& room);
    void displayServices(const Service& s);
    void displayPayments(const Payment& p);
    void displayContracts(const Contract& c);

    void managerooms();
    void manageservices();
    void managepayments();
    void managecontracts();
    void showlist();
    void AccandNotipopup();
    void showprofile();
    void showmyroom();
    void showFloorPlan();
    void searchroom();
    void searchSer();
    static void updateAllFile();
    explicit User(QWidget *parent = nullptr);
    ~User();

private slots:
    void onExtendButtonClicked(int row);
    void onpay_btnbtnClicked(int row);
    void onterminate_btnClicked(int row);
    void on_listWidget_clicked();
    void on_listWidget_2_clicked();
    void onEditButtonClicked(int i);
    void ondelete_Ser_btnClicked(int i);
    void on_TTCNbtn_clicked();
    void onedit_Ser_btnClicked(int row);
    void on_Refbtn_clicked();

    void on_editprofilebtn_clicked();

    void on_signoutbtn_clicked();

    void on_signoutbtn1_clicked();

    void on_Changepassbtn_clicked();

    void on_Changepass_clicked();

    void on_updateprofile_clicked();

    void on_pushButton_2_clicked();

    void on_roombtn_clicked();

    void on_pushButton_3_clicked();

    void on_Serbtn1_clicked();

    void on_Serbtn_clicked();

    void on_Paymentbtn_clicked();

    void on_Paymentbtn1_clicked();

    void on_Contractbtn_clicked();

    void on_Contractbtn1_clicked();

    void on_backbtn_clicked();

    void on_Accbtn_2_clicked();

    void on_searchPayment_clicked();
    void on_RefPaymentbtn_clicked();

private:
    Ui::User *ui;
    RoomLayoutPage *floorPlanPage = nullptr;
};

#endif // USER_H
