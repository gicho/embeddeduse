// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include <tuple>
#include <QRandomGenerator>
#include <QString>

#include "canbusrouter.h"
#include "ecu.h"
#include "j1939_broadcast_frames.h"
#include "j1939_frame.h"

Ecu::Ecu(int ecuId, CanBusRouter *router, QObject *parent)
    : QObject(parent)
    , m_router{router}
    , m_ecuId{ecuId}
{
}

Ecu::~Ecu()
{
}

int Ecu::ecuId() const
{
    return m_ecuId;
}

bool Ecu::isLogging() const
{
    return m_logging;
}

void Ecu::setLogging(bool enabled)
{
    m_logging = enabled;
}

void Ecu::onFramesReceived(const QSet<int> &ecuIdColl)
{
    if (!ecuIdColl.contains(1))
    {
        return;
    }
    for (const auto &frame : m_router->takeReceivedFrames(1))
    {
        if (isReadParameter(frame)) {
            receiveReadParameter(frame);
        }
    }
}

void Ecu::onErrorOccurred()
{
    emit logMessage(QString("ERROR: %1.").arg(m_router->errorString()));
}

bool Ecu::isReadParameter(const J1939Frame &frame) const
{
    return frame.frameId() == 0x18ef0201U && frame.payload()[0] == char(1);
}

void Ecu::sendReadParameter(quint16 pid, quint32 value)
{
    emitLogMessage(QString("Ecu/Send: Read(0x%2, 0x%3)")
                   .arg(pid, 4, 16, QLatin1Char('0'))
                   .arg(value, 8, 16, QLatin1Char('0')));
    m_router->writeFrame(ReadParameterResponse(0x01U, static_cast<quint8>(ecuId()), pid, value));
}

void Ecu::receiveReadParameter(const J1939Frame &frame)
{
    auto payload{frame.decode<ReadParameterRequest::Payload>()};
    emitLogMessage(QString("Ecu/Recv: Read(0x%2, 0x%3)")
                   .arg(quint16(payload.parameterId), 4, 16, QLatin1Char('0'))
                   .arg(quint32(payload.parameterValue), 8, 16, QLatin1Char('0')));
    sendReadParameter(quint16(payload.parameterId), QRandomGenerator::global()->generate());
}

void Ecu::sendUnsolicitedFrames()
{
    if (ecuId() == 3) {
        emitLogMessage(QString{"Sent from ECU 3: A03VehicleSpeed(750, 344)"});
        m_router->writeFrame(A03VehicleSpeed{750, 344, 1340, 3499});

        emitLogMessage(QString{"Sent from ECU 3: A03VehicleSpeed(750, 407)"});
        m_router->writeFrame(A03VehicleSpeed{750, 407, 1340, 3499});

        emitLogMessage(QString{"Sent from ECU 3: A03VehicleSpeed(750, 489)"});
        m_router->writeFrame(A03VehicleSpeed{750, 489, 1340, 3499});
    }
    else if (ecuId() == 2) {
        emitLogMessage(QString{"Sent from ECU 2: A02AxleTilt(389, -813, 1034)"});
        m_router->writeFrame(A02AxleTilt{389, -813, 1034});

        emitLogMessage(QString{"Sent from ECU 2: A02AxleTilt(443, -978, 981)"});
        m_router->writeFrame(A02AxleTilt{443, -978, 981});

        emitLogMessage(QString{"Sent from ECU 2: A02AxleTilt(508, -1088, 855)"});
        m_router->writeFrame(A02AxleTilt{508, -1088, 855});
    }
}

void Ecu::emitLogMessage(const QString &info)
{
    if (isLogging())
    {
        emit logMessage(info);
    }
}


