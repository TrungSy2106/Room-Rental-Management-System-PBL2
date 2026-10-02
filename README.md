# Room Management System

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-Widgets%20%7C%20Charts-41CD52.svg)
![Build](https://img.shields.io/badge/build-qmake-orange.svg)

Room Management System là ứng dụng desktop quản lý phòng được xây dựng bằng **C++17** và **Qt**. Hệ thống cung cấp giao diện riêng cho quản trị viên và người thuê, hỗ trợ quản lý nghiệp vụ thuê phòng, thanh toán, dịch vụ, thống kê và thiết kế sơ đồ mặt bằng trực quan.

## Tính năng chính

### Quản trị hệ thống

- Quản lý phòng, loại phòng và người thuê.
- Quản lý đặt phòng, hợp đồng, dịch vụ và thanh toán.
- Theo dõi việc sử dụng dịch vụ và thống kê doanh thu.
- Quản lý tài khoản và thông tin quản trị viên.

### Người thuê

- Xem tổng quan và cập nhật hồ sơ cá nhân.
- Tìm kiếm, xem thông tin và đặt phòng.
- Theo dõi hợp đồng, dịch vụ và lịch sử thanh toán.

### Thiết kế sơ đồ mặt bằng cho dãy trọ

- Tạo và quản lý sơ đồ theo nhiều tầng.
- Vẽ tường; bố trí phòng, cửa, cầu thang, văn bản và hệ trục.
- Chọn, di chuyển, xoay, sao chép, dán và xóa đối tượng.
- **Command Pattern** hỗ trợ Undo/Redo cho các thao tác chỉnh sửa.
- Chế độ **ORTHO** và **OSNAP** với Endpoint, Midpoint, Intersection và Perpendicular.
- Lưu tự động dữ liệu sơ đồ vào thư mục dữ liệu của ứng dụng.

## Công nghệ sử dụng

- C++17
- Qt Core, GUI, Widgets và Charts
- Qt Designer

## Cấu trúc dự án

```text
PBL2/
├── data/                         # Dữ liệu mẫu của ứng dụng
├── Resources/                    # Hình ảnh và biểu tượng giao diện
├── src/
│   ├── app/                      # Điểm khởi chạy chương trình
│   ├── core/                     # Date, LinkedList và tiện ích chung
│   ├── domain/                   # Các lớp nghiệp vụ
│   └── presentation/
│       ├── dialogs/              # Các hộp thoại nhập liệu
│       ├── pages/admin/          # Chức năng quản trị
│       ├── pages/user/           # Chức năng người thuê
│       ├── statistics/           # Thống kê thanh toán
│       └── windows/              # Sign in, Admin và User windows
├── Pbl2.pro                      # Cấu hình qmake
└── Resources.qrc                 # Qt Resource Collection
```

## Cài đặt và chạy

### Yêu cầu

- Qt 5 hoặc Qt 6 có module **Charts**.
- Trình biên dịch hỗ trợ C++17, chẳng hạn MinGW hoặc MSVC.
- Qt Creator hoặc qmake trên `PATH`.

### Sử dụng Qt Creator

1. Mở file `Pbl2.pro` trong Qt Creator.
2. Chọn Qt Kit phù hợp.
3. Chạy **Build** rồi **Run**.

### Sử dụng dòng lệnh

```bash
qmake Pbl2.pro
mingw32-make
```

qmake sẽ sao chép dữ liệu mẫu từ thư mục `data/` sang thư mục chạy của chương trình.