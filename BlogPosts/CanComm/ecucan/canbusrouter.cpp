// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include <QCanBus>
#include <QCanBusDevice>
#include <QMetaObject>
#include <QVariant>
#include <QtDebug>

#include "canbusrouter.h"

CanBusRouter::CanBusRouter(int canId, const QString &plugin, const QString &interface,
                           QObject *parent)
    : QObject(parent)
    , m_canId{canId}
{
    m_device = createDevice(plugin, interface);
    if (m_device == nullptr)
    {
        return;
    }
    connectToDevice();
    connect(m_device, &QCanBusDevice::errorOccurred,
            this, &CanBusRouter::onErrorOccurred);
    connect(m_device, &QCanBusDevice::framesReceived,
            this, &CanBusRouter::onFramesReceived,
            Qt::QueuedConnection);
    connect(m_device, &QCanBusDevice::framesWritten,
            this, &CanBusRouter::framesWritten);
}

CanBusRouter::~CanBusRouter()
{
    disconnectFromDevice();
    delete m_device;
}

QCanBusDevice::CanBusError CanBusRouter::error() const
{
    return m_error;
}

QString CanBusRouter::errorString() const
{
    return m_errorStr;
}

QCanBusDevice::CanBusDeviceState CanBusRouter::state() const
{
    return m_state;
}

QVector<QCanBusFrame> CanBusRouter::takeReceivedFrames(int ecuId)
{
    if (m_device == nullptr)
    {
        return {};
    }
    return m_receivedFrameCache.takeFrames(ecuId);
}

bool CanBusRouter::isReceiveOwnFrameEnabled() const
{
    if (m_device == nullptr)
    {
        return false;
    }
    return m_device->configurationParameter(QCanBusDevice::ReceiveOwnKey).toBool();
}

void CanBusRouter::setReceiveOwnFrameEnabled(bool enabled)
{
    if (m_device == nullptr)
    {
        return;
    }
    m_device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, enabled);
}

void CanBusRouter::writeFrame(const QCanBusFrame &frame)
{
    if (m_device == nullptr)
    {
        return;
    }
    // TODO: Find out whether own frame is always guaranteed to be received. If not, add some
    // error handling.
    if (isReceiveOwnFrameEnabled())
    {
        enqueueOutoingFrame(frame);
    }
    else
    {
        m_device->writeFrame(frame);
    }
}

void CanBusRouter::onErrorOccurred(QCanBusDevice::CanBusError error)
{
    m_error = error;
    m_errorStr = m_device->errorString();
    emit errorOccurred();
}

void CanBusRouter::onFramesReceived()
{
    if (m_device == nullptr)
    {
        return;
    }
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
    auto ecuIdColl = m_receivedFrameCache.updateFrames(readAllFrames());
#else
    auto ecuIdColl = m_receivedFrameCache.updateFrames(m_device->readAllFrames());
#endif
    processOwnFrames(ecuIdColl);
    emit framesReceived(ecuIdColl);
}

// QCanBus::createDevice() returns nullptr and an error message, if the plugin does not exist.
// It returns a non-null QCanBusDevice, if the plugin exists. Whether the CAN interface exists,
// does not matter.
QCanBusDevice *CanBusRouter::createDevice(const QString &plugin, const QString &interface)
{
    auto device = QCanBus::instance()->createDevice(plugin, interface, &m_errorStr);
    if (!m_errorStr.isEmpty())
    {
        m_error = QCanBusDevice::CanBusError::ConnectionError;
    }
    return device;
}

void CanBusRouter::connectToDevice()
{
    if (m_device == nullptr)
    {
        return;
    }
    if (!m_device->connectDevice())
    {
        m_error = m_device->error();
        m_errorStr = m_device->errorString();
    }
    m_state = m_device->state();
}

void CanBusRouter::disconnectFromDevice()
{
    if (m_device == nullptr)
    {
        return;
    }
    if (m_device->state() == QCanBusDevice::ConnectedState)
    {
        m_device->disconnectDevice();
    }
}

void CanBusRouter::enqueueOutoingFrame(const QCanBusFrame &frame)
{
    auto wasEmptyOnEntry = m_writtenFrameCache.isEmpty();
    // NOTE: If the frame was appended to the cache after writing it, the own frame could be
    // received before the frame is in the cache.
    m_writtenFrameCache.append(frame);
    if (wasEmptyOnEntry) {
        m_device->writeFrame(frame);
    }
}

void CanBusRouter::dequeueOutgoingFrame()
{
    if (!m_writtenFrameCache.isEmpty()) {
        m_writtenFrameCache.removeFirst();
    }
    if (!m_writtenFrameCache.isEmpty()) {
        m_device->writeFrame(m_writtenFrameCache.first());
    }
}

void CanBusRouter::processOwnFrames(QSet<int> &ecuIdColl)
{
    if (ecuIdColl.contains(m_canId))
    {
        ecuIdColl.remove(m_canId);
        const auto &ownFrameColl = m_receivedFrameCache.takeFrames(m_canId);
        for (int i = 0; i < ownFrameColl.count(); ++i)
        {
            dequeueOutgoingFrame();
        }
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
QVector<QCanBusFrame> CanBusRouter::readAllFrames()
{
    auto frameColl = QVector<QCanBusFrame>{};
    auto count = m_device->framesAvailable();
    for (int i = 0; i < count; ++i)
    {
        frameColl.append(m_device->readFrame());
    }
    return frameColl;
}
#endif
