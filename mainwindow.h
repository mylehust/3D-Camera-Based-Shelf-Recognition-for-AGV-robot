#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QByteArray>
#include <QTcpServer>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSliderValueChanged();

    void on_expected_textChanged();

    void on_tolerance_textChanged();

private:
    Ui::MainWindow *ui;
    QImage originalPixmap;
    QTcpSocket *socket;
    QTcpServer *server;
    QJsonObject currentJsonObj;
    void cropImage(int width1, int width2, int height1, int height2);
    void startServer();
    void updateUIFromJson(const QJsonObject &jsonObj);
    void onNewConnection();
    void onReadyRead();
    void requestData();
    // Các biến cần thiết để lưu kích thước và dữ liệu
    uint32_t jsonSize = 0;
    uint32_t imageSize = 0;

    QByteArray jsonData;  // Dữ liệu JSON được nhận
    QByteArray imageData;  // Dữ liệu ảnh được nhận
};
#endif // MAINWINDOW_H

