#include "portscanner.h"
#include <QDebug>

PortScanner::PortScanner(QObject *parent) : QObject(parent), m_selected(nullptr)
{
    m_model.push_back(new Port(nullptr));
}
void PortScanner::checkPorts() {
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (QObject *obj : m_model) {
        auto port = static_cast<Port*>(obj);
        if (port->getPort() == "searching") continue;
        if (std::find_if(serialPortInfos.begin(), serialPortInfos.end(), [port](const QSerialPortInfo &serialPortInfo){return port->getPort() == serialPortInfo.systemLocation();}) == serialPortInfos.end()) {
            m_model.removeOne(port);
        }
    }
    if (serialPortInfos.length() > 0) {
        m_model.erase(std::remove_if(m_model.begin(), m_model.end(), [](QObject* object){return (static_cast<Port*>(object))->getPort() == "searching";}),m_model.end());
    } else if (m_model.length() == 0) {
        m_model.push_back(new Port());
    }
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        auto port = new Port(serialPortInfo);
        if (port->getPort() == nullptr) continue;
        auto find = std::find_if(m_model.begin(), m_model.end(), [serialPortInfo](QObject* object){return (static_cast<Port*>(object))->getPort() == serialPortInfo.systemLocation();});
        if (find == m_model.end()) {
            m_model.push_back(port);
            port->open(serialPortInfo);
        }
    }
    emit modelChanged(m_model);
}
