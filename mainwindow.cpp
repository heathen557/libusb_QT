#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>
#include<iostream>

#define MY_CONFIG 1
#define MY_INTF 0


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    runFlag = true;
    usb_init();                                  /* initialize the library */

    int numBus = usb_find_busses();              /* find all busses */
    qDebug()<<"NUMbUS = "<<numBus<<endl;

    int numDevs = usb_find_devices();           /* find all connected devices */
    qDebug()<<"numDevs = "<<numDevs<<endl;

    dev = findUSBDev(0,0);                      //查找usb设备
    openUSB(dev);                               //打开USB设备


    connect(&readTimer, SIGNAL(timeout()),this,SLOT(read_usb()));

}


//查找是不是存在想要链接的USB设备
struct usb_device *MainWindow::findUSBDev(const unsigned short idVendor,
                              const unsigned short idProduct)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */

    for(bus = usb_get_busses(); bus; bus = bus->next)
    {
        for(dev = bus->devices; dev; dev = dev->next)
        {
//            if((dev->descriptor.idVendor == idVendor)
//            && (dev->descriptor.idProduct == idProduct))
//            {
//                return dev;
//            }

            qDebug()<<"dev->descriptor.idVendor = "<<dev->descriptor.idVendor<<"  "
             <<"dev->descriptor.idProduct = "<<dev->descriptor.idProduct<<endl;
            return dev;
        }
    }
    return NULL;

}

//打开已经存在的USB设备
bool MainWindow::openUSB(struct usb_device *dev)
{
    devOpenFlg = false;

    devHandle = usb_open(dev);
    if(!devHandle)
    {
        qDebug() << "error opening device: ";
        qDebug() << usb_strerror();
        return false;
    }
    else
    {
        qDebug() << "success: device " << "MY_VID" << " : "<<  "MY_PID" << " opened";
        devOpenFlg = true;
    }

    if (usb_set_configuration(devHandle, MY_CONFIG) < 0)
    {
        qDebug() << "error setting config #" << MY_CONFIG << " : " << usb_strerror();
        usb_close(devHandle);
        return false;
    }
    else
    {
        qDebug() << "success: set configuration #" << MY_CONFIG;
    }

    if (usb_claim_interface(devHandle, MY_INTF) < 0)
    {
        qDebug() << "error claiming interface #" << MY_INTF;
        qDebug() << usb_strerror();
        usb_close(devHandle);
        return false;
    }
    else
    {
        qDebug() <<  "success: claim_interface #" << MY_INTF;
    }

//    read_usb();
    return true;

}


//关闭USB链接
void MainWindow::closeUSB()
{
    if(devHandle)
    {
        devOpenFlg = false;
//        UsbListener::quit();
        int ret = usb_close(devHandle); // Exit Thread
        qDebug() << "Close USB Device [" << ret << "]";
    }

}

//测试按钮
void MainWindow::on_pushButton_clicked()
{
    QString array;
    //系统注册 读取  0x13 = 19,array返回值
    bool res = System_Register_Read(19,array);
    qDebug()<<"[R]sys Read array="<<array<<"   res="<<res<<endl;
    //系统注册 写入测试

/**********************测试STR2******************************************/
    // 1、 0x11= 17  0x41=65
    QString str2 = "41 01 00";
    res = System_Register_Write(17,str2.mid(0,2));
    qDebug()<<"[w]sys write str2="<<str2.mid(0,2)<<"   res="<<res<<endl;

    // 0x12 = 18
    res = System_Register_Write(18,str2.mid(3,2));
    qDebug()<<"[w]sys write str2="<<str2.mid(3,2)<<"   res="<<res<<endl;

    //0xe2 = 226
    res = System_Register_Write(226,str2.mid(6,2));
    qDebug()<<"[w]sys write str2="<<str2.mid(6,2)<<"   res="<<res<<endl;

    //0x13 = 19
    res = System_Register_Read(19,array);
    qDebug()<<"[R]sys Read array="<<array<<"   res="<<res<<endl;


/************************测试STR3********************************************/
    QString str3 = "00 00 26 0A 00 64 00 14 00 01 00 00";   //len = 12

    //起始位置从32 开始
    for(int i=0 ; i<12; i++)
    {
        res = System_Register_Write(32+i, str3.mid(i*3,2));
        qDebug()<<"[w]sys write str3="<<str3.mid(i*3,2)<<"   res="<<res<<endl;
    }


/*************************测试STR1******************************************************/
    QString str1 = "00 44 1F 44 45 44 EE 02 64 11 22 44 88 88 44 22 11 03 40 00 1F E0 81 4A 84 08 00 00 CC 01 00 00 00 00 00 00 00 0A 06 06 06 06 06 34 FF FF FF FF 04 1E";
    //0xd8 = 216,
    for(int k=0; k<50; k++)
    {
        res = Device_Register_Write(216,k,str1.mid(3*k,2));
        qDebug()<<"[w]Device write str1="<<str1.mid(3*k,2)<<"   res="<<res<<endl;
    }

//    read_usb();

    readTimer.start(1);

}


//系统FPGA注册 读取
bool MainWindow::System_Register_Read(int Address, QString &Data)
{

    bool res = true;
    int transLen = 0;
    struct usb_ctrl_setup Cmd;
    Cmd.bRequestType = 0xC0;
    Cmd.bRequest = 0x01;
    Cmd.wValue = 0x0000;
    Cmd.wIndex = Address;  //need change
    Cmd.wLength = 0x0001;
    char data[1];
    QString arr;
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);
    Data = QString(data[0]);
    return res;
}


//系统FPGA注册 写入
bool MainWindow::System_Register_Write(int Address, QString &Data)
{
    bool res = true;
    int transLen = 0;
    char data[1];
    struct usb_ctrl_setup Cmd;
    Cmd.bRequestType = 0x40;
    Cmd.bRequest = 0x01;
    Cmd.wValue = 0x0000;
    Cmd.wIndex = Address;  // need change
    Cmd.wLength = 0x0001;
    data[0] = Data.toInt(NULL,16);    //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);
    return res;
}

//设备注册 读取
bool MainWindow::Device_Register_Read(int slavedId,int Address,QString &Data)
{
    bool res = true;
    int transLen = 0;
    char data[1];
    struct usb_ctrl_setup Cmd;
    Cmd.bRequestType = 0x40;
    Cmd.bRequest = 0x01;
    Cmd.wValue = 0x0000;
    Cmd.wIndex = 0x00f1;
    Cmd.wLength = 0x0001;
    data[0]= slavedId;   //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f5;
    data[0] = 0x33;
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f2;
    data[0] = Address;      //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f5;
    data[0] = 0xf9;
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.bRequestType = 0xC0;
    Cmd.wIndex = 0x00f4;
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Data = QString(data[0]);

    return res;
}


//设备注册 写入
bool MainWindow::Device_Register_Write(int slavedId,int Address,QString &Data)
{
    bool res = true;
    int transLen = 0;
    char data[1];
    struct usb_ctrl_setup Cmd;
    Cmd.bRequestType = 0x40;
    Cmd.bRequest = 0x01;
    Cmd.wValue = 0x0000;
    Cmd.wIndex = 0x00f1;
    Cmd.wLength = 0x0001;
    data[0] = slavedId;      //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f5;
    data[0] = 0x37;
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f2;
    data[0] = Address;      //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    Cmd.wIndex = 0x00f3;
    data[0] = Data.toInt(NULL,16);         //need modify
    res = res && usb_control_msg(devHandle,Cmd.bRequestType,Cmd.bRequest,Cmd.wValue,Cmd.wIndex,data,1,transLen);

    return res;
}




//读取USB设备数据
void MainWindow::read_usb()
{
    int ret;
    char readdata[4096];
//    uchar readdata[4096];
    QByteArray array;

//    while(runFlag)
    {
        //批量读(同步)
        ret = usb_bulk_read(devHandle, 129, readdata, sizeof(readdata), 2);

//        transfer_bulk_async(devHandle,129,readdata,sizeof(readdata),260);

        if (ret < 0) {
                qDebug("**************************************************error reading:%s", usb_strerror());
//                break;
                exit(1);
        }


        for(int i=0 ;i<ret; i++)
        {
            qDebug()<<"data["<<i<<"] = "<<quint8(readdata[i])<<"   ret ="<<ret<<endl;
        }

        readdata[ret] = '\0';

//        Sleep(100);
    }


}


MainWindow::~MainWindow()
{
    runFlag = false;
    closeUSB();
    delete ui;
}


