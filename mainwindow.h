#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"lusb0_usb.h"
#include<QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    struct usb_device *findUSBDev(const unsigned short idVendor,
                                  const unsigned short idProduct);

    bool openUSB(struct usb_device *dev);

    void closeUSB();



    bool System_Register_Read(int Address, QString &Data);

    bool System_Register_Write(int Address, QString &Data);

    bool Device_Register_Read(int slavedId,int Address,QString &Data);

    bool Device_Register_Write(int slavedId,int Address,QString &Data);


    bool devOpenFlg;

    usb_dev_handle *devHandle;

    struct usb_device * dev;

    bool  runFlag;

    QTimer readTimer;

private slots:
    void on_pushButton_clicked();
    void read_usb();
private:
    Ui::MainWindow *ui;


};

#endif // MAINWINDOW_H
