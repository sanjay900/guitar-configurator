#ifndef PORT_H
#define PORT_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "ardwiinolookup.h"
#include "submodules/Ardwiino/src/shared/config/config.h"
#include "submodules/Ardwiino/src/shared/controller/controller.h"

class Port : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(bool isArdwiino READ isArdwiino)
    Q_PROPERTY(QVariantMap pin_inverts MEMBER m_pin_inverts NOTIFY pinInvertsChanged)
    Q_PROPERTY(QVariantMap pins MEMBER m_pins NOTIFY pinsChanged)
    Q_PROPERTY(QString image READ getImage NOTIFY imageChanged)
    Q_PROPERTY(QString boardImage READ getBoardImage NOTIFY boardImageChanged)
    Q_PROPERTY(bool hasDFU MEMBER m_hasDFU NOTIFY dfuFound)
    Q_PROPERTY(InputTypes::Value inputType READ getInputType WRITE setInputType NOTIFY inputTypeChanged)
    Q_PROPERTY(Controllers::Value type READ getType WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(MPU6050Orientations::Value orientation READ getOrientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(TiltTypes::Value tiltType READ getTiltType WRITE setTiltType NOTIFY tiltTypeChanged)
public:
    explicit Port(const QSerialPortInfo &serialPortInfo, QObject *parent = nullptr);
    explicit Port(QObject *parent = nullptr);
    board_t getBoard() const {
        return m_board;
    }
    void prepareUpload();
    void findNew();
    void open(const QSerialPortInfo &serialPortInfo);
    void close();
signals:
    void descriptionChanged(QString newValue);
    void imageChanged(QString newValue);
    void pinsChanged(QVariantMap newValue);
    void pinInvertsChanged(QVariantMap newValue);
    void boardImageChanged(QString newValue);
    void inputTypeChanged(InputTypes::Value newValue);
    void typeChanged(Controllers::Value newValue);
    void orientationChanged(MPU6050Orientations::Value newValue);
    void tiltTypeChanged(TiltTypes::Value newValue);
    void dfuFound(bool found);

public slots:
    void writeConfig();
    QString description() const {
        return m_description;
    }
    board_t board() const {
        return m_board;
    }
    QString boardName() const {
        return m_board.name;
    }
    void setBoardFreq(uint freq) {
        m_board.cpuFrequency = freq;
    }
    bool isArdwiino() const {
        return m_isArdwiino;
    }
    bool isGuitar() const {
        return ArdwiinoLookup::getInstance()->getControllerTypeName(getType()).toLower().contains("guitar");
    }
    QString getPort() const {
        return m_port;
    }
    QString getImage() const {
        return Controllers::getImage(Controllers::Value(m_config.main.sub_type));
    }
    QString getBoardImage() const {
        return m_board.image;
    }
    Controllers::Value getType() const {
        return Controllers::Value(m_config.main.sub_type);
    }
    MPU6050Orientations::Value getOrientation() const {
        return MPU6050Orientations::Value(m_config.axis.mpu_6050_orientation);
    }
    InputTypes::Value getInputType() const {
        return InputTypes::Value(m_config.main.input_type);
    }
    TiltTypes::Value getTiltType() const {
        return TiltTypes::Value(m_config.main.tilt_type);
    }
    void setType(Controllers::Value value) {
        m_config.main.sub_type = value;
        imageChanged(getImage());
        typeChanged(value);
    }
    void setInputType(InputTypes::Value value) {
        m_config.main.input_type = value;
        inputTypeChanged(value);

    }
    void setTiltType(TiltTypes::Value value) {
        m_config.main.tilt_type = value;
        tiltTypeChanged(value);
    }
    void setOrientation(MPU6050Orientations::Value value) {
        m_config.axis.mpu_6050_orientation = value;
        orientationChanged(value);
    }
    void handleError(QSerialPort::SerialPortError serialPortError);
    bool findNewAsync();
    void readDescription();
    void loadPins();
    void savePins();
    void loadKeys();
    void saveKeys();
private:
    void readData();
    void read(char id, QByteArray& location, void* dest, unsigned long size);
    void write(char id, void* dest, unsigned long size);
    void rescan(const QSerialPortInfo &serialPortInfo);
    QString m_description;
    QString m_port;
    QSerialPort* m_serialPort;
    board_t m_board;
    QList<QSerialPortInfo> m_port_list;
    bool m_isArdwiino;
    bool m_hasDFU;
    config_t m_config_device;
    config_t m_config;
    QVariantMap m_pins;
    QVariantMap m_pin_inverts;
};

#endif // PORT_H
