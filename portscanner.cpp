#include "portscanner.h"
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QIcon>
#include <QDirIterator>
#include <QBitmap>
PortScanner::PortScanner(Programmer *programmer, QObject *parent) : QObject(parent), m_selected(nullptr), programmer(programmer)
{
    m_model.push_back(new Port());
    if (settings.contains("configMode")) {
        m_graphical = settings.value("configMode").toBool();
    } else {
        m_graphical = true;
    }
}
void PortScanner::addPort(const QSerialPortInfo& serialPortInfo) {
    if (m_selected != nullptr) {
        m_selected->handleConnection(serialPortInfo);
        programmer->program(m_selected);
        if (!programmer->getRestore()) {
            m_selected->handleConnection(serialPortInfo);
            return;
        }
    }
    auto port = new Port(serialPortInfo);
    if (port->getPort() == nullptr) return;
    m_model.erase(std::remove_if(m_model.begin(), m_model.end(), [](QObject* object){return (dynamic_cast<Port*>(object))->getPort() == "searching";}),m_model.end());
    m_model.push_back(port);
    port->open(serialPortInfo);
    connect(port, &Port::descriptionChanged,this,&PortScanner::update);
    connect(port, &Port::typeChanged, this, &PortScanner::clearImages);
    emit modelChanged();
}
void PortScanner::update() {
    //For whatever reason, just updating the model is not enough to make the QML combobox update correctly. However, removing all items then adding them again does work.
    auto m = m_model;
    m_model.clear();
    m_model.push_back(new Port());
    emit modelChanged();
    m_model.clear();
    m_model << m;
    emit modelChanged();
}
void PortScanner::removePort(const QSerialPortInfo& serialPortInfo) {
    if (m_selected != nullptr) {
        //Pass through to selected, replace its scanning implementation
    }
    m_model.erase(std::remove_if(m_model.begin(), m_model.end(), [serialPortInfo](QObject* object){return (dynamic_cast<Port*>(object))->getPort() == serialPortInfo.systemLocation();}), m_model.end());
    if (m_model.length() == 0) {
        m_model.push_back(new Port());
    }
    emit modelChanged();
}
void PortScanner::fixLinux() {
    QFile f("/sys/bus/usb/drivers/xpad/new_id");
    f.open(QFile::ReadOnly);
    QString s = QString(f.readAll());
    f.close();
    if(!s.contains("1209 2882")) {
        QProcess p;
        p.start("pkexec", {"tee", "-a", "/sys/bus/usb/drivers/xpad/new_id"});
        p.waitForStarted();
        p.write("1209 2882");
        p.closeWriteChannel();
        p.waitForFinished();
    }
}
void PortScanner::clearImages() {
    images.clear();
}

void PortScanner::toggleGraphics() {
    m_graphical = !m_graphical;
    graphicalChanged();
    settings.setValue("configMode", m_graphical);
}
QVector<uchar> data;
int width;
int height;
QString PortScanner::findElement(QString base, int w, int h, int mouseX, int mouseY) {
    if (images.isEmpty()) {
        width = w;
        height = h;
        data.clear();
        data.resize(width*height);
        auto imageList = QDir(":/"+base+"/components").entryList();
        imageList.sort();
        QVector<QRgb> colorMap;
        colorMap.push_back(qRgba(0,0,0,0));
        colorMap.push_back(qRgba(255,255,255,255));
        for (auto image: imageList) {
            auto i = QIcon(":/"+base+"/components/"+image).pixmap(QSize(width,height)).toImage();
            i = i.convertToFormat(QImage::Format_Indexed8, colorMap, Qt::AutoColor);
            auto s2 = i.height() * i.width();
            auto s3 = data.data();
            auto s = i.bits();
            while (s2--) {
                if (*s != 0) {
                    *s3 = *s;
                }
                s3++;
                s++;
            }
            images.push_back(image);
            colorMap.push_front(qRgba(0,0,0,0));
        }
    }
    int x = (double)mouseX/w*width;
    int y = (double)mouseY/h*height;
    auto c = data[y*width+x];
    if (c != 0) {
        return "/"+base+"/components/"+images[c-1];
    }
    return "";
}
