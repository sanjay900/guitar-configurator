#include "ardwiinolookup.h"
#include "QDebug"
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDir>
QVersionNumber ArdwiinoLookup::currentVersion = QVersionNumber(-1);
QVersionNumber ArdwiinoLookup::supports1Mhz = QVersionNumber(2,2,10);
const static auto versionRegex = QRegularExpression("^version-([\\d.]+)$");
ArdwiinoLookup::ArdwiinoLookup(QObject *parent):QObject(parent) {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd("firmware");
    QFile f(dir.filePath("version"));
    f.open(QFile::ReadOnly | QFile::Text);
    auto match2 = versionRegex.match(f.readAll());
    currentVersion = QVersionNumber::fromString(match2.captured(1));
}

auto ArdwiinoLookup::lookupType(uint8_t type) -> QString {
    return ArdwiinoDefines::getName(ArdwiinoDefines::subtype(type));
}

auto ArdwiinoLookup::isIncompatibleArdwiino(const QSerialPortInfo& serialPortInfo) -> bool {
    return isArdwiino(serialPortInfo) && (serialPortInfo.serialNumber() == "1.2" || versionRegex.match(serialPortInfo.serialNumber()).hasMatch());
}
auto ArdwiinoLookup::isOld(const QString& version) -> bool {
    auto match = versionRegex.match(version.trimmed());
    return QVersionNumber::fromString(match.captured(1)) < currentVersion;
}
auto ArdwiinoLookup::isOldAPIArdwiino(const QSerialPortInfo& serialPortInfo) -> bool {
    return isArdwiino(serialPortInfo) && serialPortInfo.serialNumber() == "1.2";
}

auto ArdwiinoLookup::is115200(const QSerialPortInfo& serialPortInfo) -> bool {
    auto match = versionRegex.match(serialPortInfo.serialNumber().toLower());
    return isArdwiino(serialPortInfo) && match.hasMatch() && QVersionNumber::fromString(match.captured(1)) < supports1Mhz;
}
//Ardwino PS3 Controllers use sony vids. No other sony controller should expose a serial port however, so we should be fine doing this.
auto ArdwiinoLookup::isArdwiino(const QSerialPortInfo& serialPortInfo) -> bool {
    return serialPortInfo.vendorIdentifier() == HARMONIX_VID || serialPortInfo.vendorIdentifier() == SONY_VID || serialPortInfo.vendorIdentifier() == SWITCH_VID || (serialPortInfo.vendorIdentifier() == ARDWIINO_VID && serialPortInfo.productIdentifier() == ARDWIINO_PID);
}
auto ArdwiinoLookup::isAreadyDFU(const QSerialPortInfo& serialPortInfo) -> bool {
    return serialPortInfo.productIdentifier() == 0x0036 || serialPortInfo.productIdentifier() == 0x9207;
}
ArdwiinoLookup* ArdwiinoLookup::instance = nullptr;
const board_t ArdwiinoLookup::empty =  {"","","",0,{},"","",0,"","", false};
const board_t ArdwiinoLookup::boards[5] = {
    {"uno-atmega16u2", "uno-usb","Arduino Uno",57600,{},"dfu","atmega16u2",16000000,"Arduino-COMBINED-dfu-usbserial-atmega16u2-Uno-Rev3.hex","images/ArduinoUno.svg",true},
    {"uno-at90usb82", "uno-usb","Arduino Uno",57600,{},"dfu","at90usb82",16000000,"UNO-dfu_and_usbserial_combined.hex","images/ArduinoUno.svg",true},
    {"uno", "uno-main","Arduino Uno",115200,{0x0043, 0x7523, 0x0001, 0xea60, 0x0243},"arduino","atmega328p",16000000,"","images/ArduinoUno.svg",true},
    {"micro", "micro","Arduino Pro Micro",57600,{0x9203, 0x9204,0x9205, 0x9206, 0x9207, 0x9208},"avr109","atmega32u4",8000000,"","images/ArduinoProMicro.svg", false},
    {"leonardo", "leonardo","Arduino Leonardo",57600,{0x0036, 0x8036, 0x800c},"avr109","atmega32u4",16000000,"","images/ArduinoLeonardo.svg", false},
};

auto ArdwiinoLookup::findByBoard(const QString& board_name) -> const board_t {
    for (const auto& board: boards) {
        if (board.shortName == board_name) {
            return board;
        }
    }
    return empty;
}

auto ArdwiinoLookup::detectBoard(const QSerialPortInfo& serialPortInfo) -> const board_t {
    for (const auto& board: boards) {
        for (auto& pid : board.productIDs) {
            if (pid && pid == serialPortInfo.productIdentifier()) {
                return board;
            }
        }
    }
    return empty;
}
auto ArdwiinoLookup::getInstance() -> ArdwiinoLookup* {
    if (!ArdwiinoLookup::instance) {
        ArdwiinoLookup::instance = new ArdwiinoLookup();
    }
    return ArdwiinoLookup::instance;
}
