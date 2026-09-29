QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += \
    $$PWD/src/core \
    $$PWD/src/domain \
    $$PWD/src/presentation/pages/admin \
    $$PWD/src/presentation/pages/admin/roomlayout \
    $$PWD/src/presentation/pages/user \
    $$PWD/src/presentation/dialogs \
    $$PWD/src/presentation/statistics \
    $$PWD/src/presentation/windows

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/app/main.cpp \
    src/core/Date.cpp \
    src/core/LinkedList.cpp \
    src/domain/Account.cpp \
    src/domain/Contract.cpp \
    src/domain/Payment.cpp \
    src/domain/Reservation.cpp \
    src/domain/Room.cpp \
    src/domain/RoomType.cpp \
    src/domain/Service.cpp \
    src/domain/ServiceUsage.cpp \
    src/domain/Tenant.cpp \
    src/presentation/pages/admin/AccountPage.cpp \
    src/presentation/pages/admin/BillingPage.cpp \
    src/presentation/pages/admin/ReservationPage.cpp \
    src/presentation/pages/admin/RoomPage.cpp \
    src/presentation/pages/admin/ServicePage.cpp \
    src/presentation/pages/admin/StatisticsPage.cpp \
    src/presentation/pages/admin/TenantPage.cpp \
    src/presentation/pages/admin/roomlayout/AxisGraphicsItem.cpp \
    src/presentation/pages/admin/roomlayout/CommandStack.cpp \
    src/presentation/pages/admin/roomlayout/FloorPlanCommands.cpp \
    src/presentation/pages/admin/roomlayout/FloorPlanDocument.cpp \
    src/presentation/pages/admin/roomlayout/FloorPlanRepository.cpp \
    src/presentation/pages/admin/roomlayout/GripItem.cpp \
    src/presentation/pages/admin/roomlayout/RoomLabelGraphicsItem.cpp \
    src/presentation/pages/admin/roomlayout/RoomLayoutPage.cpp \
    src/presentation/pages/admin/roomlayout/RoomLayoutView.cpp \
    src/presentation/pages/admin/roomlayout/SymbolGraphicsItem.cpp \
    src/presentation/pages/admin/roomlayout/TextAnnotationItem.cpp \
    src/presentation/pages/admin/roomlayout/WallGraphicsItem.cpp \
    src/presentation/pages/user/UserContractPage.cpp \
    src/presentation/pages/user/UserDashboardPage.cpp \
    src/presentation/pages/user/UserPaymentPage.cpp \
    src/presentation/pages/user/UserProfilePage.cpp \
    src/presentation/pages/user/UserRoomBookingPage.cpp \
    src/presentation/pages/user/UserServicePage.cpp \
    src/presentation/dialogs/AddService.cpp \
    src/presentation/dialogs/Addroom.cpp \
    src/presentation/dialogs/Addroomtype.cpp \
    src/presentation/dialogs/Adminaccount.cpp \
    src/presentation/dialogs/Booking.cpp \
    src/presentation/dialogs/Createpayment.cpp \
    src/presentation/dialogs/Editroom.cpp \
    src/presentation/dialogs/Editroomtype.cpp \
    src/presentation/dialogs/Editservice.cpp \
    src/presentation/dialogs/Edittenant.cpp \
    src/presentation/dialogs/Extend.cpp \
    src/presentation/dialogs/Paybill.cpp \
    src/presentation/statistics/PaymentStatistics.cpp \
    src/presentation/windows/Signin.cpp \
    src/presentation/windows/User.cpp \
    src/presentation/windows/admin.cpp

HEADERS += \
    src/core/Date.h \
    src/core/LinkedList.h \
    src/domain/Account.h \
    src/domain/Contract.h \
    src/domain/Payment.h \
    src/domain/Reservation.h \
    src/domain/Room.h \
    src/domain/RoomType.h \
    src/domain/Service.h \
    src/domain/ServiceUsage.h \
    src/domain/Tenant.h \
    src/presentation/pages/admin/roomlayout/AxisGraphicsItem.h \
    src/presentation/pages/admin/roomlayout/CommandStack.h \
    src/presentation/pages/admin/roomlayout/FloorPlanCommands.h \
    src/presentation/pages/admin/roomlayout/FloorPlanDocument.h \
    src/presentation/pages/admin/roomlayout/FloorPlanRepository.h \
    src/presentation/pages/admin/roomlayout/GripItem.h \
    src/presentation/pages/admin/roomlayout/RoomLabelGraphicsItem.h \
    src/presentation/pages/admin/roomlayout/RoomLayoutPage.h \
    src/presentation/pages/admin/roomlayout/RoomLayoutView.h \
    src/presentation/pages/admin/roomlayout/SymbolGraphicsItem.h \
    src/presentation/pages/admin/roomlayout/TextAnnotationItem.h \
    src/presentation/pages/admin/roomlayout/WallGraphicsItem.h \
    src/presentation/dialogs/AddService.h \
    src/presentation/dialogs/Addroom.h \
    src/presentation/dialogs/Addroomtype.h \
    src/presentation/dialogs/Adminaccount.h \
    src/presentation/dialogs/Booking.h \
    src/presentation/dialogs/Createpayment.h \
    src/presentation/dialogs/Editroom.h \
    src/presentation/dialogs/Editroomtype.h \
    src/presentation/dialogs/Editservice.h \
    src/presentation/dialogs/Edittenant.h \
    src/presentation/dialogs/Extend.h \
    src/presentation/dialogs/Paybill.h \
    src/presentation/statistics/PaymentStatistics.h \
    src/presentation/windows/Signin.h \
    src/presentation/windows/User.h \
    src/presentation/windows/admin.h

FORMS += \
    src/presentation/dialogs/AddService.ui \
    src/presentation/dialogs/Addroom.ui \
    src/presentation/dialogs/Addroomtype.ui \
    src/presentation/dialogs/Adminaccount.ui \
    src/presentation/dialogs/Booking.ui \
    src/presentation/dialogs/Createpayment.ui \
    src/presentation/dialogs/Editroom.ui \
    src/presentation/dialogs/Editroomtype.ui \
    src/presentation/dialogs/Editservice.ui \
    src/presentation/dialogs/Edittenant.ui \
    src/presentation/dialogs/Extend.ui \
    src/presentation/dialogs/Paybill.ui \
    src/presentation/pages/admin/roomlayout/RoomLayoutPage.ui \
    src/presentation/windows/Signin.ui \
    src/presentation/windows/User.ui \
    src/presentation/windows/admin.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc

# Keep demo data in source control and copy it beside the build working
# directory so the application can run immediately after a fresh build.
DEMO_DATA_FILES = \
    $$PWD/data/Account.txt \
    $$PWD/data/Contract.txt \
    $$PWD/data/FloorPlanGeometry.txt \
    $$PWD/data/Payment.txt \
    $$PWD/data/Reservation.txt \
    $$PWD/data/Room.txt \
    $$PWD/data/RoomType.txt \
    $$PWD/data/Service.txt \
    $$PWD/data/ServiceUsage.txt \
    $$PWD/data/Tenant.txt

demo_data.files = $$DEMO_DATA_FILES
win32 {
    CONFIG(debug, debug|release) {
        demo_data.path = $$OUT_PWD/debug
    } else {
        demo_data.path = $$OUT_PWD/release
    }
} else {
    demo_data.path = $$OUT_PWD
}
COPIES += demo_data

OTHER_FILES += $$DEMO_DATA_FILES
