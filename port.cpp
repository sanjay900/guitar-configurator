#include "port.h"
#include "QDebug"
#include "QThread"
#include <QProcess>
#include "input_types.h"
#include "submodules/Ardwiino/src/shared/config/config.h"
#include <iostream>
#include <QSettings>
Port::Port(const QSerialPortInfo &serialPortInfo, QObject *parent) : QObject(parent), m_board(ArdwiinoLookup::empty), m_waitingForNew(false)
{
    rescan(serialPortInfo);
}

Port::Port(QObject *parent) : QObject(parent), m_board(ArdwiinoLookup::empty), m_waitingForNew(false)
{
    m_description = "Searching for devices";
    m_port = "searching";
}

void Port::close() {
    if (m_serialPort != nullptr) {
        m_serialPort->close();
    }
}
void Port::rescan(const QSerialPortInfo &serialPortInfo) {
    m_isArdwiino = ArdwiinoLookup::getInstance()->isArdwiino(serialPortInfo);
    if (m_isArdwiino) {
        m_description = "Ardwiino - Reading Controller Information";
        m_port = serialPortInfo.systemLocation();
    } else {
        auto b = ArdwiinoLookup::detectBoard(serialPortInfo);
        if (b != nullptr) {
            m_board = *b;
            m_port = serialPortInfo.systemLocation();
            m_description = m_board.name + " - "+m_port;
            boardImageChanged(getBoardImage());
        }
    }
    emit descriptionChanged(m_description);
}
void Port::read(char id, QByteArray &readData, void* dest, unsigned long size) {
    readData.clear();
    m_serialPort->flush();
    m_serialPort->write(&id, 1);
    m_serialPort->waitForBytesWritten();
    m_serialPort->waitForReadyRead();
    readData.push_back(m_serialPort->readAll());
    if (dest != nullptr) {
        memcpy(dest, readData.data(), size);
    }
}
void Port::readData() {
    QByteArray readData;
    read(FW_CMD_R, readData, nullptr, 0);
    m_board = ArdwiinoLookup::findByBoard(readData);
    m_hasDFU = ArdwiinoLookup::getInstance()->hasDFUVariant(m_board);
    boardImageChanged(getBoardImage());

    read(MAIN_CMD_R, readData, &m_config.main, sizeof(main_config_t));
    read(PIN_CMD_R, readData, &m_config.pins, sizeof(pins_t));
    read(AXIS_CMD_R, readData, &m_config.axis, sizeof(axis_config_t));
    read(KEY_CMD_R, readData, &m_config.keys, sizeof(keys_t));

    memcpy(&m_config_device, &m_config, sizeof(config_t));
    readDescription();
}
void Port::write(char id, void* dest, unsigned long size) {
    m_serialPort->flush();
    m_serialPort->write(&id, 1);
    m_serialPort->write(static_cast<char*>(dest), static_cast<signed long>(size));
    m_serialPort->waitForBytesWritten();
}
void Port::writeConfig() {
    write(MAIN_CMD_W, &m_config.main, sizeof(main_config_t));
    write(PIN_CMD_W, &m_config.pins, sizeof(pins_t));
    write(AXIS_CMD_W, &m_config.axis, sizeof(axis_config_t));
    write(KEY_CMD_W, &m_config.keys, sizeof(keys_t));
    char data = REBOOT_CMD;
    m_serialPort->flush();
    m_serialPort->write(&data, 1);
    m_serialPort->waitForBytesWritten();
    m_serialPort->close();
    m_port_list = QSerialPortInfo::availablePorts();
    m_waitingForNew = true;
    waitingForNewChanged(m_waitingForNew);
}

void Port::readDescription() {
    controller_t controller;
    if (InputTypes::Value(m_config_device.main.input_type) == InputTypes::WII_TYPE) {
        QByteArray readData;
        read(CONTROLLER_CMD_R, readData, &controller, sizeof(controller_t));
    }
    m_description = "Ardwiino - "+ m_board.name+" - "+ArdwiinoLookup::getInstance()->lookupType(m_config_device.main.sub_type);
    m_description += " - " + ArdwiinoLookup::getInstance()->lookupExtension(m_config_device.main.input_type, controller.device_info);
    m_description += " - " + m_port;
    updateControllerName();
    emit descriptionChanged(m_description);
}

void Port::open(const QSerialPortInfo &serialPortInfo) {
    m_serialPort = new QSerialPort(serialPortInfo);
    if (m_isArdwiino) {
        m_serialPort->setBaudRate(57600);
        QObject::connect(m_serialPort, &QSerialPort::errorOccurred, this, &Port::handleError);
        if (m_serialPort->open(QIODevice::ReadWrite)) {
            readData();
        }
    }
}

void Port::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError != QSerialPort::SerialPortError::NoError && serialPortError != QSerialPort::NotOpenError) {
        m_description = "Ardwiino - Error Communicating";
        m_serialPort->close();
    }
}
bool comp(const QSerialPortInfo a, const QSerialPortInfo b)
{
    return a.portName() < b.portName();
}
void Port::prepareUpload() {
    if (m_board.protocol == "avr109") {
        m_serialPort->close();
        m_serialPort->setBaudRate(QSerialPort::Baud1200);
        m_serialPort->setDataBits(QSerialPort::DataBits::Data8);
        m_serialPort->setStopBits(QSerialPort::StopBits::OneStop);
        m_serialPort->setParity(QSerialPort::Parity::NoParity);
        //We need this setting, but it has been deprecated. In the future when it is removed, it will be the default behaviour and will not be required anymore.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        m_serialPort->setSettingsRestoredOnClose(false);
#pragma GCC diagnostic pop
        if (m_serialPort->open(QIODevice::WriteOnly)) {
            m_serialPort->setDataTerminalReady(false);
        }
        m_serialPort->close();
        m_port_list = QSerialPortInfo::availablePorts();
    }
}

void Port::updateControllerName() {
    //On windows, update the controller name, so that users see the current device name in games
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_1209&PID_2882", QSettings::NativeFormat);
    settings.setValue("OEMName", m_description);
#endif
}
bool Port::findNew() {
    auto newSp = QSerialPortInfo::availablePorts();
    std::vector<QSerialPortInfo> diff;
    std::sort(newSp.begin(), newSp.end(), comp);
    std::set_difference(newSp.begin(), newSp.end(), m_port_list.begin(), m_port_list.end(), std::inserter(diff, diff.begin()), comp);
    m_port_list = newSp;
    if (diff.size() != 0) {
        auto info = diff.front();
        QThread::currentThread()->msleep(400);
        m_port = info.systemLocation();
        rescan(info);
        open(info);
        if (m_waitingForNew) {
            m_waitingForNew = false;
            waitingForNewChanged(m_waitingForNew);
        }
        return true;
    }
    return false;
}

void Port::loadKeys() {
    m_pins.clear();
    m_pins["up"] = m_config.keys.up;
    m_pins["down"] = m_config.keys.down;
    m_pins["left"] = m_config.keys.left;
    m_pins["right"] = m_config.keys.right;
    m_pins["start"] = m_config.keys.start;
    m_pins["back"] = m_config.keys.back;
    m_pins["left_stick"] = m_config.keys.left_stick;
    m_pins["right_stick"] = m_config.keys.right_stick;
    m_pins["LB"] = m_config.keys.LB;
    m_pins["RB"] = m_config.keys.RB;
    m_pins["home"] = m_config.keys.home;
    m_pins["capture"] = m_config.keys.capture;
    m_pins["a"] = m_config.keys.a;
    m_pins["b"] = m_config.keys.b;
    m_pins["x"] = m_config.keys.x;
    m_pins["y"] = m_config.keys.y;
    m_pins["lt"] = m_config.keys.lt;
    m_pins["rt"] = m_config.keys.rt;
    m_pins["l_x_lt"] = m_config.keys.l_x.neg;
    m_pins["l_x_gt"] = m_config.keys.l_x.pos;
    m_pins["l_y_lt"] = m_config.keys.l_y.neg;
    m_pins["l_y_gt"] = m_config.keys.l_y.pos;
    m_pins["r_x_lt"] = m_config.keys.r_x.neg;
    m_pins["r_x_gt"] = m_config.keys.r_x.pos;
    m_pins["r_y_lt"] = m_config.keys.r_y.neg;
    m_pins["r_y_gt"] = m_config.keys.r_y.pos;
}

void Port::saveKeys() {
    m_config.keys.up = uint8_t(m_pins["up"].toUInt());
    m_config.keys.down = uint8_t(m_pins["down"].toUInt());
    m_config.keys.left = uint8_t(m_pins["left"].toUInt());
    m_config.keys.right = uint8_t(m_pins["right"].toUInt());
    m_config.keys.start = uint8_t(m_pins["start"].toUInt());
    m_config.keys.back = uint8_t(m_pins["back"].toUInt());
    m_config.keys.left_stick = uint8_t(m_pins["left_stick"].toUInt());
    m_config.keys.right_stick = uint8_t(m_pins["right_stick"].toUInt());
    m_config.keys.LB = uint8_t(m_pins["LB"].toUInt());

    m_config.keys.RB = uint8_t(m_pins["RB"].toUInt());
    m_config.keys.home = uint8_t(m_pins["home"].toUInt());
    m_config.keys.capture = uint8_t(m_pins["capture"].toUInt());
    m_config.keys.a = uint8_t(m_pins["a"].toUInt());
    m_config.keys.b = uint8_t(m_pins["b"].toUInt());
    m_config.keys.x = uint8_t(m_pins["x"].toUInt());
    m_config.keys.y = uint8_t(m_pins["y"].toUInt());
    m_config.keys.lt = uint8_t(m_pins["lt"].toUInt());
    m_config.keys.rt = uint8_t(m_pins["rt"].toUInt());
    m_config.keys.l_x.neg = uint8_t(m_pins["l_x_lt"].toUInt());
    m_config.keys.l_x.pos = uint8_t(m_pins["l_x_gt"].toUInt());
    m_config.keys.l_y.neg = uint8_t(m_pins["l_y_lt"].toUInt());
    m_config.keys.l_y.pos = uint8_t(m_pins["l_y_gt"].toUInt());
    m_config.keys.r_x.neg = uint8_t(m_pins["r_x_lt"].toUInt());
    m_config.keys.r_x.pos = uint8_t(m_pins["r_x_gt"].toUInt());
    m_config.keys.r_y.neg = uint8_t(m_pins["r_y_lt"].toUInt());
    m_config.keys.r_y.pos = uint8_t(m_pins["r_y_gt"].toUInt());
}

void Port::loadPins() {
    m_pins.clear();
    m_pins["up"] = m_config.pins.up;
    m_pins["down"] = m_config.pins.down;
    m_pins["left"] = m_config.pins.left;
    m_pins["right"] = m_config.pins.right;
    m_pins["start"] = m_config.pins.start;
    m_pins["back"] = m_config.pins.back;
    m_pins["left_stick"] = m_config.pins.left_stick;
    m_pins["right_stick"] = m_config.pins.right_stick;
    m_pins["LB"] = m_config.pins.LB;
    m_pins["RB"] = m_config.pins.RB;
    m_pins["home"] = m_config.pins.home;
    m_pins["unused"] = m_config.pins.unused;
    m_pins["a"] = m_config.pins.a;
    m_pins["b"] = m_config.pins.b;
    m_pins["x"] = m_config.pins.x;
    m_pins["y"] = m_config.pins.y;
    m_pins["lt"] = m_config.pins.lt;
    m_pins["rt"] = m_config.pins.rt;
    m_pins["l_x"] = m_config.pins.l_x;
    m_pins["l_y"] = m_config.pins.l_y;
    m_pins["r_x"] = m_config.pins.r_x;
    m_pins["r_y"] = m_config.pins.r_y;
    m_pin_inverts["lt"] = m_config.axis.inversions.lt;
    m_pin_inverts["rt"] = m_config.axis.inversions.rt;
    m_pin_inverts["l_x"] = m_config.axis.inversions.l_x;
    m_pin_inverts["l_y"] = m_config.axis.inversions.l_y;
    m_pin_inverts["r_x"] = m_config.axis.inversions.r_x;
    m_pin_inverts["r_y"] = m_config.axis.inversions.r_y;
}

void Port::savePins() {
    m_config.pins.up = uint8_t(m_pins["up"].toUInt());
    m_config.pins.down = uint8_t(m_pins["down"].toUInt());
    m_config.pins.left = uint8_t(m_pins["left"].toUInt());
    m_config.pins.right = uint8_t(m_pins["right"].toUInt());
    m_config.pins.start = uint8_t(m_pins["start"].toUInt());
    m_config.pins.back = uint8_t(m_pins["back"].toUInt());
    m_config.pins.left_stick = uint8_t(m_pins["left_stick"].toUInt());
    m_config.pins.right_stick = uint8_t(m_pins["right_stick"].toUInt());
    m_config.pins.LB = uint8_t(m_pins["LB"].toUInt());
    m_config.pins.RB = uint8_t(m_pins["RB"].toUInt());
    m_config.pins.home = uint8_t(m_pins["home"].toUInt());
    m_config.pins.unused = uint8_t(m_pins["unused"].toUInt());
    m_config.pins.a = uint8_t(m_pins["a"].toUInt());
    m_config.pins.b = uint8_t(m_pins["b"].toUInt());
    m_config.pins.x = uint8_t(m_pins["x"].toUInt());
    m_config.pins.y = uint8_t(m_pins["y"].toUInt());
    m_config.pins.lt = uint8_t(m_pins["lt"].toUInt());
    m_config.pins.rt = uint8_t(m_pins["rt"].toUInt());
    m_config.pins.l_x = uint8_t(m_pins["l_x"].toUInt());
    m_config.pins.l_y = uint8_t(m_pins["l_y"].toUInt());
    m_config.pins.r_x = uint8_t(m_pins["r_x"].toUInt());
    m_config.pins.r_y = uint8_t(m_pins["r_y"].toUInt());
    m_config.axis.inversions.lt = m_pin_inverts["lt"].toBool();
    m_config.axis.inversions.rt = m_pin_inverts["rt"].toBool();
    m_config.axis.inversions.l_x = m_pin_inverts["l_x"].toBool();
    m_config.axis.inversions.l_y = m_pin_inverts["l_y"].toBool();
    m_config.axis.inversions.r_x = m_pin_inverts["r_x"].toBool();
    m_config.axis.inversions.r_y = m_pin_inverts["r_y"].toBool();
}


