#include "outdated_ardwiino.h"

#include <QThread>
#define OLD_ARDWIINO_API_SERIAL "1.2"
OutdatedArdwiino::OutdatedArdwiino(const QSerialPortInfo &serialPortInfo, QObject *parent) : SerialDevice(serialPortInfo, parent), m_isOldAPI(serialPortInfo.serialNumber() == OLD_ARDWIINO_API_SERIAL) {
}
bool OutdatedArdwiino::open() {
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        QThread *thread = QThread::create([this] {
            //We need a delay on linux for unos, as the 328p gets rebooted when we open.
            QThread::msleep(2000);
            QMetaObject::invokeMethod(this, [this] {
                QString readCmd;
                //At one point the API was changed
                if (m_isOldAPI) {
                    readCmd = "f";
                } else {
                    readCmd = "i\x06";
                }
                do {
                    m_board = ArdwiinoLookup::findByBoard(readWrite(readCmd.toUtf8()).trimmed().split('-')[0], false);
                } while (m_board.name.isEmpty() && m_serialPort->isOpen());
                if (!m_isOldAPI && m_serialPort->isOpen()) {
                    QString readFreq = "i\x04";
                    m_board.cpuFrequency = readWrite(readFreq.toUtf8()).trimmed().replace("UL", "").toInt();
                }
                emit descriptionChanged();
                emit boardImageChanged();
                emit boardNameChanged();
                emit readyChanged();
                close();
            });
        });
        thread->start();
        return true;
    }
    return false;
}
bool OutdatedArdwiino::isReady() {
    return !m_board.name.isEmpty() && m_serialPort->isOpen();
}
void OutdatedArdwiino::close() {
    SerialDevice::close();
}
QString OutdatedArdwiino::getDescription() {
    if (!isReady()) {
        return "Outdated Ardwiino - Loading Information";
    }
    return "Outdated Ardwiino - " + m_board.name;
}
void OutdatedArdwiino::bootloader() {
    m_serialPort->open(QIODevice::ReadWrite);
    QString cmd;
    if (m_board.protocol == "avr109") {
        cmd = "e";
    } else {
        cmd = "j";
    }
    m_serialPort->write(cmd.toUtf8());
    m_serialPort->waitForBytesWritten();
    close();
}