#pragma once
#include <math.h>

#include <QDebug>
#include <QJSValue>
#include <QObject>
#include <QQueue>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "ardwiino_defines.h"
#include "ardwiinolookup.h"
#include "submodules/Ardwiino/src/shared/output/serial_commands.h"
class Device : public QObject {
    Q_PROPERTY(QString description READ getDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_OBJECT
   public:
    explicit Device(QObject *parent = nullptr);
    virtual QString getDescription() = 0;
    virtual bool isReady() = 0;
    virtual void open() = 0;
    virtual void close() = 0;
    void setBoardType(QString board);
    inline bool operator==(const Device& other) {
        return typeid(*this) == typeid(other) && isEqual(other);
    }
   signals:
    void descriptionChanged();
    void readyChanged();
   protected:
    board_t m_board;
   private:
    inline virtual bool isEqual(const Device& other) const = 0;
};