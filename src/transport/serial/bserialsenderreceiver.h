#ifndef BSERIALSENDERRECEIVER_H
#define BSERIALSENDERRECEIVER_H

#include <QSerialPort>

#include "src/transport/network/bathreadnetworkreceiver.h"

class BSerialSenderReceiver : public BAThreadNetworkReceiver
{
    Q_OBJECT
public:
    BSerialSenderReceiver(QString pPortName, QSerialPort::BaudRate pBaudRate, QSerialPort::DataBits pDataBits,
                          QSerialPort::Parity pParity, QSerialPort::FlowControl pFlowControl,
                          QSerialPort::StopBits pStopBits);

signals:
    void errorOccurred(QSerialPort::SerialPortError error);

public slots:
    void send(QByteArray data);

    // BAThread interface
protected:
    void mainLoop() override;

private slots:
    void error(QSerialPort::SerialPortError error);

private:
    QString mPortName;
    QSerialPort::BaudRate mBaudRate;
    QSerialPort::DataBits mDataBits;
    QSerialPort::Parity mParity;
    QSerialPort::FlowControl mFlowControl;
    QSerialPort::StopBits mStopBits;
    QSerialPort *mSerialPort;

    const int TIME_OUT_MS = 20;
};

#endif // BSERIALSENDERRECEIVER_H
