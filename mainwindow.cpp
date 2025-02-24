#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <winsock2.h>
#include <windows.h>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(new QTcpServer(this))
{
    ui->setupUi(this);

    // Kiểm tra xem QLabel có ảnh không
    if (!ui->label_7->pixmap().isNull()) {
        originalPixmap = ui->label_7->pixmap().toImage();
        //qDebug() << "Image loaded successfully!";
        //qDebug() << "Original image size:" << originalPixmap.width() << "x" << originalPixmap.height();
    } else {
        qDebug() << "No image found in label_7!";
    }

    connect(ui->width_1, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->width_2, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->height_1, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->height_2, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->expected, &QTextEdit::textChanged, this, &MainWindow::on_expected_textChanged);
    connect(ui->tolerance, &QTextEdit::textChanged, this, &MainWindow::on_tolerance_textChanged);

    // Khởi động server để nhận JSON từ client
    startServer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startServer() // khởi động server
{
    if (!server->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Server not start!";
    } else {
        qDebug() << "Server started!";
        connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);
    }
}

void MainWindow::onNewConnection()
{
    socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
}

/*
void MainWindow::onReadyRead() // ver 1
{
    QByteArray data = socket->readAll();

    QString jsonString(data);

    QImage receivedImage;
    receivedImage.loadFromData(data, "JPEG");
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());


    if (!jsonDoc.isNull()) {
        currentJsonObj = jsonDoc.object();  // Lưu trữ JSON vào biến currentJsonObj
        updateUIFromJson(currentJsonObj);  // Cập nhật lại UI ngay khi nhận được JSON mới
    }
    else{
        qDebug() << "NO JSONNnnnnnnn" << currentJsonObj;

    }
    if(!receivedImage.isNull())
    {
        ui->label_11->setPixmap(QPixmap::fromImage(receivedImage));
        qDebug()<<"Nhận được ảnh nè";
    }
    else{
        qDebug()<<"không load được ảnh";
    }
    qDebug() << "Size of received image:" << data.size()-551;
    socket->flush();
}*/

void MainWindow::onReadyRead()
{
    // Tăng kích thước buffer nhận
    socket->setReadBufferSize(65536);  // 64KB


    // Đọc toàn bộ dữ liệu từ socket
    QByteArray data = socket->readAll();

    if (data.size() < sizeof(size_t) * 2) {
        qDebug() << "Dữ liệu không đủ lớn để chứa kích thước JSON và ảnh!";
        return;
    }

    // Tách kích thước JSON và kích thước ảnh
    QDataStream stream(&data, QIODevice::ReadOnly);
    size_t jsonSize, imageSize;
    stream >> jsonSize >> imageSize;

    // Kiểm tra nếu kích thước dữ liệu đủ lớn để chứa cả JSON và ảnh
    if (data.size() < jsonSize + imageSize + sizeof(size_t) * 2) {
        qDebug() << "Dữ liệu chưa đủ. Chờ thêm dữ liệu từ socket.";
        return;
    }

    // Tách phần JSON và phần ảnh từ dữ liệu nhận được
    QByteArray jsonData = data.mid(sizeof(size_t) * 2, jsonSize);
    QByteArray imageData = data.mid(sizeof(size_t) * 2 + jsonSize, imageSize);

    // Xử lý JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isNull()) {
        currentJsonObj = jsonDoc.object();  // Lưu trữ JSON vào biến currentJsonObj
        updateUIFromJson(currentJsonObj);  // Cập nhật UI
        qDebug() << "Nhận được JSON thành công";
    } else {
        qDebug() << "Không thể phân tích JSON" << currentJsonObj;
    }

    // Xử lý ảnh
    QImage receivedImage;
    receivedImage.loadFromData(imageData, "JPEG");
    if (!receivedImage.isNull()) {
        ui->label_11->setPixmap(QPixmap::fromImage(receivedImage));  // Hiển thị ảnh
        qDebug() << "Nhận được ảnh thành công";
    } else {
        qDebug() << "Không thể load ảnh";
    }

    qDebug() << "Tổng kích thước dữ liệu nhận được:" << data.size();
    socket->flush();  // Làm sạch buffer của socket
}


// https://stackoverflow.com/questions/45620230/data-loss-when-sending-image-over-socket-to-external-server


// void MainWindow::onReadyRead() // code gửi only image
// {
//     static QByteArray buffer;
//     static int expectedImageSize = -1;

//     buffer.append(socket->readAll()); // buffer  lưu giữ liệu đc tryueenf qua socket

//     while (!buffer.isEmpty()) {
//         if (expectedImageSize == -1) { // chưa biết kích thước của ảnh
//             if (buffer.size() >= sizeof(int)) {
//                 memcpy(&expectedImageSize, buffer.constData(), sizeof(int)); // coppy dữ liệu từ buffer vào image (data, size)
//                 buffer.remove(0, sizeof(int));  // xóa
//             } else {
//                 return;
//             }
//         }

//         if (buffer.size() >= expectedImageSize) { // đủ/dư dữ liệu để lưu img
//             QByteArray imageData = buffer.left(expectedImageSize); // .left là trích 1 phần dữ liệu  = img để lưu dữ liệu
//             buffer.remove(0, expectedImageSize);  // Remove the image data from the buffer

//             QImage receivedImage;
//             receivedImage.loadFromData(imageData, "JPEG");

//             if (!receivedImage.isNull()) {
//                 ui->label_11->setPixmap(QPixmap::fromImage(receivedImage));
//                 qDebug() << "Received image";
//             } else {
//                 qDebug() << "Failed to load image";
//             }

//             expectedImageSize = -1;
//         } else {
//             // không đủ dữ liệu
//             return;
//         }
//     }

//     socket->flush();
// }

// void MainWindow::onReadyRead()
// {
//     static QByteArray buffer;
//     static int expectedImageSize = -1;

//     buffer.append(socket->readAll()); // buffer chứa toàn bộ dữ liệu nhân từ socket

//     while (!buffer.isEmpty()) {
//         if (expectedImageSize == -1) {  // Nếu chưa biết kích thước của ảnh
//             if (buffer.size() >= sizeof(int)) { // đủ chỗ để ghi ảnh
//                 memcpy(&expectedImageSize, buffer.constData(), sizeof(int)); // sao chép dữ liệu
//                 buffer.remove(0, sizeof(int));  // Loại bỏ phần dữ liệu kích thước ảnh
//             } else {
//                 return;
//             }
//         }

//         //  biết kích thước ảnh
//         if (buffer.size() >= expectedImageSize) {  // đủ/dư dữ liệu cho ảnh
//             QByteArray imageData = buffer.left(expectedImageSize); // bỏ vào img phần dữ liệu img
//             buffer.remove(0, expectedImageSize);

//             QImage receivedImage;
//             receivedImage.loadFromData(imageData, "JPEG");

//             if (!receivedImage.isNull()) {
//                 ui->label_11->setPixmap(QPixmap::fromImage(receivedImage));
//                 qDebug() << "Received image";
//             } else {
//                 qDebug() << "Failed to load image";
//             }

//             expectedImageSize = -1;
//         } else {
//             return;  // Không đủ dữ liệu cho ảnh
//         }

//         // send img trước, json sau
//         if (!buffer.isEmpty()) {
//             // Giả định rằng sau khi ảnh được gửi, phần còn lại là chuỗi JSON
//             QString jsonString(buffer);  // Chuyển phần còn lại của buffer thành chuỗi JSON
//             QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

//             if (!jsonDoc.isNull()) {
//                 currentJsonObj = jsonDoc.object();  // Lưu trữ JSON vào biến currentJsonObj
//                 updateUIFromJson(currentJsonObj);   // Cập nhật UI dựa trên nội dung JSON
//                 qDebug() << "Received and processed JSON";
//             } else {
//                 qDebug() << "Failed to parse JSON";
//             }

//             buffer.clear();  // Xóa buffer sau khi đã xử lý xong dữ liệu JSON
//         }
//     }

//     socket->flush();  // Xóa dữ liệu tạm thời trong socket
// }

/*
void MainWindow::onReadyRead() // thêm header để phân biệt
{
    static QByteArray buffer;
    static int expectedSize = -1;         // Kích thước của phần dữ liệu tiếp theo
    static QString currentDataType;        // Loại dữ liệu hiện tại (JSON hoặc IMG)
    static bool isReadingHeader = true;    // Cờ để theo dõi xem đang đọc header hay dữ liệu

    buffer.append(socket->readAll());  // Nhận dữ liệu mới từ socket và thêm vào buffer

    while (!buffer.isEmpty()) {
        if (isReadingHeader) {
            // Đọc header để xác định loại dữ liệu (4 bytes)
            if (buffer.size() >= 4) {
                currentDataType = QString(buffer.left(4));  // Lấy 4 byte đầu tiên (header)
                buffer.remove(0, 4);  // Loại bỏ header khỏi buffer

                // Kiểm tra nếu header không phải là "IMG " hoặc "JSON"
                if (currentDataType != "IMG " && currentDataType != "JSON") {
                    qDebug() << "Error: Unknown header received: " << currentDataType;
                    return;  // Nếu không nhận dạng được header, thoát và đợi thêm dữ liệu
                }

                // Sau khi đọc header, bắt đầu đọc kích thước dữ liệu
                isReadingHeader = false;
                expectedSize = -1;  // Reset expectedSize để chuẩn bị cho dữ liệu tiếp theo
            } else {
                // Nếu không đủ 4 bytes để đọc header, thoát và đợi thêm dữ liệu
                return;
            }
        }

        // Đọc kích thước dữ liệu
        if (expectedSize == -1) {
            if (buffer.size() >= sizeof(int)) {
                memcpy(&expectedSize, buffer.constData(), sizeof(int));  // Lấy kích thước dữ liệu
                buffer.remove(0, sizeof(int));  // Loại bỏ phần kích thước đã đọc
            } else {
                // Nếu không đủ dữ liệu để đọc kích thước, thoát và đợi thêm dữ liệu
                return;
            }
        }

        // Đọc dữ liệu dựa trên kích thước đã xác định
        if (buffer.size() >= expectedSize) {
            QByteArray data = buffer.left(expectedSize);  // Trích xuất dữ liệu
            buffer.remove(0, expectedSize);  // Loại bỏ phần dữ liệu vừa xử lý

            if (currentDataType == "JSON") {
                // Xử lý dữ liệu JSON
                QString jsonString(data);
                QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

                if (!jsonDoc.isNull()) {
                    currentJsonObj = jsonDoc.object();  // Lưu trữ JSON vào biến currentJsonObj
                    updateUIFromJson(currentJsonObj);   // Cập nhật UI dựa trên nội dung JSON
                    qDebug() << "Received and processed JSON";
                } else {
                    qDebug() << "Failed to parse JSON";
                }
            } else if (currentDataType == "IMG ") {
                // Xử lý dữ liệu ảnh
                QImage receivedImage;
                receivedImage.loadFromData(data, "JPEG");

                if (!receivedImage.isNull()) {
                    ui->label_11->setPixmap(QPixmap::fromImage(receivedImage));
                    qDebug() << "Received image";
                } else {
                    qDebug() << "Failed to load image";
                }
            }

            // Đặt lại để chuẩn bị đọc header cho phần dữ liệu tiếp theo
            expectedSize = -1;
            currentDataType.clear();
            isReadingHeader = true;  // Chuẩn bị đọc header tiếp theo
        } else {
            // Nếu không đủ dữ liệu cho phần hiện tại, thoát và đợi thêm
            return;
        }
    }

    socket->flush();  // Xóa dữ liệu tạm thời trong socket
}
*/

void MainWindow::updateUIFromJson(const QJsonObject &currentJsonObj)
{
    qDebug() << "currentJsonObj: " << currentJsonObj;

    if (currentJsonObj.isEmpty()) {
        qDebug() << "currentJsonObj is empty";
        return;
    }

    // Kiểm tra sự tồn tại của "data" và "detectionResult"
    if (!currentJsonObj.contains("data") || !currentJsonObj["data"].isObject()) {
        qDebug() << "currentJsonObj does not contain 'data' or 'data' is not an object.";
        return;
    }

    QJsonObject dataObject = currentJsonObj["data"].toObject();

    if (!dataObject.contains("detectionResult") || !dataObject["detectionResult"].isObject()) {
        qDebug() << "'data' does not contain 'detectionResult' or 'detectionResult' is not an object.";
        return;
    }

    QJsonObject detectionResult = dataObject["detectionResult"].toObject();

    QJsonObject leftcorner = detectionResult["leftCorner"].toObject();
    QJsonObject rightcorner = detectionResult["rightCorner"].toObject();

    // Kiểm tra và lấy giá trị từ leftCorner và rightCorner
    double leftX = leftcorner.contains("X") ? leftcorner["X"].toDouble() : 0.0;
    double leftY = leftcorner.contains("Y") ? leftcorner["Y"].toDouble() : 0.0;
    double leftZ = leftcorner.contains("Z") ? leftcorner["Z"].toDouble() : 0.0;
    double rightX = rightcorner.contains("X") ? rightcorner["X"].toDouble() : 0.0;
    double rightY = rightcorner.contains("Y") ? rightcorner["Y"].toDouble() : 0.0;
    double rightZ = rightcorner.contains("Z") ? rightcorner["Z"].toDouble() : 0.0;

    double cornerDis = detectionResult.contains("cornersDistance") ? detectionResult["cornersDistance"].toDouble() : 0.0;
    bool DollyFound = detectionResult.contains("dollyFound") ? detectionResult["dollyFound"].toBool() : false;

    double expected = ui->expected->toPlainText().toDouble();
    double tolerance = ui->tolerance->toPlainText().toDouble();
    double minexp = expected - tolerance;
    double maxexp = expected + tolerance;

    qDebug() << "maxDis = " << maxexp << "minDis= " << minexp;

    if (cornerDis >= minexp && cornerDis <= maxexp) {
        ui->Left_cordinate->setText(QString("X= %1; Y= %2; Z= %3").arg(leftX).arg(leftY).arg(leftZ));
        ui->Right_cordinate->setText(QString("X= %1; Y= %2; Z= %3").arg(rightX).arg(rightY).arg(rightZ));
        ui->distance->setText(QString::number(cornerDis));
        ui->Dolly->setText(DollyFound ? "Found" : "Not Found");
    } else {
        ui->Left_cordinate->setText("none");
        ui->Right_cordinate->setText("none");
        ui->distance->setText(QString::number(cornerDis));
        ui->Dolly->setText("Not Found");
    }
}


void MainWindow::onSliderValueChanged()
{
    int width1_percent = ui->width_1->value();
    int width2_percent = ui->width_2->value();
    int height1_percent = ui->height_1->value();
    int height2_percent = ui->height_2->value();

    int width1 = (originalPixmap.width() * width1_percent) / 100;
    int width2 = (originalPixmap.width() * width2_percent) / 100;
    int height1 = (originalPixmap.height() * height1_percent) / 100;
    int height2 = (originalPixmap.height() * height2_percent) / 100;

    ui->text_wid1->setText(QString::number(width1));
    ui->text_wid2->setText(QString::number(width2));
    ui->text_hei1->setText(QString::number(height1));
    ui->text_hei2->setText(QString::number(height2));

    cropImage(width1, width2, height1, height2);
}

void MainWindow::cropImage(int width1, int width2, int height1, int height2)
{
    if (width2 < width1) {
        std::swap(width1, width2);
    }
    if (height2 < height1) {
        std::swap(height1, height2);
    }

    if (width2 > originalPixmap.width()) {
        width2 = originalPixmap.width();
    }
    if (height2 > originalPixmap.height()) {
        height2 = originalPixmap.height();
    }

    QRect cropRect(width1, height1, width2 - width1, height2 - height1);
    QImage croppedImage = originalPixmap.copy(cropRect);

    ui->label_7->setPixmap(QPixmap::fromImage(croppedImage));
}

void MainWindow::on_expected_textChanged()
{
    //qDebug() << "Expected text changed, updating UI with current JSON.";
    updateUIFromJson(currentJsonObj);
}

void MainWindow::on_tolerance_textChanged()
{
    //qDebug() << "Tolerance text changed, updating UI with current JSON.";
    updateUIFromJson(currentJsonObj);
}


