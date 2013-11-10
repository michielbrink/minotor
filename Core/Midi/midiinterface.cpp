/*
 * Copyright 2012, 2013 Gauthier Legrand
 * Copyright 2012, 2013 Romuald Conty
 * 
 * This file is part of Minotor.
 * 
 * Minotor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Minotor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Minotor.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "midiinterface.h"

#include <QStringList>
#include <QDebug>

#include "midi.h"
#include "midimapping.h"
#include "midimapper.h"

#include "minotor.h"

#include <QRegExp>

MidiInterface::MidiInterface(const QString& portName, Midi *parent) :
    QObject(parent),
    _midi(parent),
    _rtMidiIn(NULL),
    _rtMidiOut(NULL),
    _id(-1),
    _portIndex(0),
    _connected(false),
    _hasOutput(false),
    _acceptClock(false),
    _acceptProgramChange(false),
    _acceptControlChange(false),
    _acceptNoteChange(false)
{
    setObjectName(portName);
    try
    {
        //Midi management
        _rtMidiIn = new RtMidiIn();
        _rtMidiOut = new RtMidiOut();
    } catch ( RtError &error ) {
        error.printMessage();
    }
}

MidiInterface::~MidiInterface()
{
    if(_rtMidiIn)
        delete _rtMidiIn;
    if(_rtMidiOut)
        delete _rtMidiOut;
}

/*
 * MIDI messages specs available at:
 *   http://www.midi.org/techspecs/midimessages.php
 */

// Channel voice messages (CVM) [nnnn = 0-15 (MIDI Channel Number 1-16)]
// These messages are channel-dependent: we may implement channel filter if needed...
#define MIDI_CVM_NOTE_OFF           0b10000000      // 1000nnnn
#define MIDI_CVM_NOTE_ON            0b10010000      // 1001nnnn
#define MIDI_CVM_CONTROL_CHANGE     0b10110000      // 1011nnnn
#define MIDI_CVM_PROGRAM_CHANGE     0b11000000      // 1100nnnn

// System Real-Time Messages (SRTM)
#define MIDI_SRTM_CLOCK      248  // 11111000
#define MIDI_SRTM_STOP       252  // 11111100
#define MIDI_SRTM_START      250  // 11111010
#define MIDI_SRTM_CONTINUE   251  // 11111011

// System Common Messages (SCM)
#define MIDI_SCM_SYSEX   240  // 11110000

// Some SysEx
#define SYSEX_HEADER                0xf0 // SysEx header byte (1st byte)
#define SYSEX_TERMINATOR            0xf7 // SysEx terminator (last byte)
#define SYSEX_NON_REALTIME_MESSAGE  0x7e // Non-Realtime Message (2nd byte)
#define SYSEX_GENERAL_INFORMATION   0x06 // General information / Inquiry message (4th byte)
#define SYSEX_IDENTITY_REQUEST      0x01 // Identity request / Inquiry request (5th byte)
#define SYSEX_IDENTITY_REPLY        0x02 // Identity reply / Inquiry response (5th byte)

#define KORG_SYSEX_HEADER           0x42 // Specific Korg SysEx command (2nd byte)
#define KORG_SYSEX__SCENE_CHANGED   0x4f // Scene changed message (9th byte)

// Manufacturers IDs
// http://www.midi.org/techspecs/manid.php
#define MIDI_MANUFACTURER_ID__KORG          0x42
#define MIDI_KORG_FAMILY_ID__NANOKONTROL    0x0104
#define MIDI_KORG_FAMILY_ID__NANOKONTROL2   0x0113

void MidiInterface::midiCallback(double deltatime, std::vector< unsigned char > *message)
{
    (void)deltatime;
    unsigned char command = message->at(0);
    quint8 channel = command & 0x0f;
    if ((command&0xf0) != 0xf0) // if it is NOT a System message
    {
        command &= 0xf0; // It removes channel from MIDI message
    }

    switch(command) {
    case MIDI_CVM_NOTE_OFF:
        if(_acceptNoteChange)
            emit noteChanged(_id, quint8 (channel), quint8 (message->at(1)), false, quint8(message->at(2)));
        break;
    case MIDI_CVM_NOTE_ON:
        if(_acceptNoteChange)
            emit noteChanged(_id, quint8 (channel), quint8 (message->at(1)), true, quint8(message->at(2)));
        break;
    case MIDI_CVM_CONTROL_CHANGE:
        if(_acceptControlChange)
            emit controlChanged(_id, quint8 (channel), quint8 (message->at(1)), quint8(message->at(2)));
        break;
    case MIDI_CVM_PROGRAM_CHANGE:
        if(_acceptProgramChange)
            emit programChanged(_id, quint8 (channel), quint8 (message->at(1)));
        break;
    case MIDI_SRTM_CLOCK: emit clockReceived(); break;
    case MIDI_SRTM_STOP: emit stopReceived(); break;
    case MIDI_SRTM_START: emit startReceived(); break;
    case MIDI_SRTM_CONTINUE: emit continueReceived(); break;
    case MIDI_SCM_SYSEX:
    {
        bool sysex_reconized = false;
        if (message->size() > 13)
        {
            if((message->at(1) == SYSEX_NON_REALTIME_MESSAGE)
                    && (message->at(3) == SYSEX_GENERAL_INFORMATION)
                    && (message->at(4) == SYSEX_IDENTITY_REPLY))
            {
                sysex_reconized = true;
                // Grab Manufacturer ID
                quint32 manufacturer_id;
                manufacturer_id = message->at(5);
                if(manufacturer_id == 0) // if use a long manufacturer ID
                {
                    manufacturer_id = message->at(6) << 16;
                    manufacturer_id = message->at(7) << 8;
                }

                QString description;
                switch (manufacturer_id)
                {
                case MIDI_MANUFACTURER_ID__KORG:
                {
                    description += "Manufacturer: Korg\n";
                    quint16 family_id = message->at(6) | (message->at(7) << 8);
                    switch(family_id)
                    {
                    case MIDI_KORG_FAMILY_ID__NANOKONTROL:
                        description += "Model: nanoKontrol\n";
                        break;
                    case MIDI_KORG_FAMILY_ID__NANOKONTROL2:
                        description += "Model: nanoKontrol2\n";
                        break;
                    default:
                        break;
                    }
                    quint16 minor_version = message->at(10) | (message->at(11) << 8);
                    quint16 major_version = message->at(12) | (message->at(13) << 8);
                    description += QString("Version: %1.%2\n").arg(major_version).arg(minor_version);
                    qDebug() << Q_FUNC_INFO << description.replace('\n', ' ');
                    _description = description;
                }
                    break;
                default:
                    qDebug() << Q_FUNC_INFO
                             << "Unknown MIDI Manufacturer ID:" << manufacturer_id;
                    break;
                }
            }
        }
        else if (message->size() == 11)
        {
            // HACK to use Korg nanoKontrol scene button in order to change master view
            if((message->at(1) == KORG_SYSEX_HEADER) && (message->at(8) == KORG_SYSEX__SCENE_CHANGED))
            {
                // message->at(2) == 0x40 // 4g where g is Global Channel
                // Software Project (nanoKontrol 1) (4 bytes)
                // message->at(3) == 0x00
                // message->at(4) == 0x01
                // message->at(5) == 0x04
                // message->at(6) == 0x00

                // message->at(7) == 0x5f // Data Dump Command  (Host<-Controller, 2Bytes Format)
                // message->at(8) == 0x4f // Not documented but seems to be "Screen changed"
                // message->at(9) stores scene ID (0x00 -> Scene 1, ... , 0x03 -> Scene 4

                // This SysEx is emitted by Korg nanoKontrol 1 when scene changed
                sysex_reconized = true;
                emit programChanged(_id, quint8 (channel), quint8 (message->at(9)));
            }
        }

        if(!sysex_reconized)
        {
            QString msg;
            for(uint i=0; i<message->size(); ++i)
            {
                msg += QString("%1 ").arg(message->at(i), 0, 16);
            }
            qDebug() << Q_FUNC_INFO
                     << "Unknown sysex received: size=" << message->size() << "content=[" << msg << "]";
            //Q_ASSERT(false);
        }
    }
        break;
    default:
        qDebug() << Q_FUNC_INFO
                 << QString("no handler for: %1 (%2)").arg(command).arg(command, 0, 16)
                 << "message size:" << message->size()
                 << "deltatime:" << deltatime;
    }
}

void _midiCallback(double deltatime, std::vector< unsigned char > *message, void *userData )
{
    (static_cast<MidiInterface*>(userData))->midiCallback(deltatime, message);
}

bool MidiInterface::open()
{
    // object name is used as portname
    return open(portName());
}

bool MidiInterface::open(const QString& portName)
{
    if(_rtMidiIn)
    {
        QStringList ports;
        // Check available ports.
        unsigned int nPorts = _rtMidiIn->getPortCount();
        for (unsigned int i=0;i<nPorts;i++)
        {
            const QString inputName = QString(_rtMidiIn->getPortName(i).c_str());
            ports.append(inputName);
//            qDebug() << "In port: " << inputName;
        }
        int portIndex = ports.indexOf(portName);
        if(portIndex<0)
            return false;
        else
        {
            setPortName(portName);
            if (!openIn(portIndex)) { return false; }
        }
        if(_rtMidiOut)
        {
            QStringList ports;
            // Check available ports.
            unsigned int nPorts = _rtMidiOut->getPortCount();
            for (unsigned int i=0;i<nPorts;i++)
            {
                ports.append(QString(_rtMidiOut->getPortName(i).c_str()));
//                qDebug() << "Out port: " << QString(_rtMidiOut->getPortName(i).c_str());
            }
            // "nanoKONTROL2 28:0"  -> "nanoKONTROL2:0"
            QRegExp rx("(\\w+) (.+):(\\d+)");
            if(rx.indexIn(portName) == -1)
            {
                qDebug() << Q_FUNC_INFO
                         << "no match for:" << portName;
                Q_ASSERT(false);
            }
            else
            {
                QStringList sl = rx.capturedTexts();
//                    qDebug() << Q_FUNC_INFO
//                             <<   sl;
                Q_ASSERT(sl.count()==4);
                const QString outputName = sl.at(1) + QString(":") + sl.at(3);
                int portIndex = ports.indexOf(outputName);
                if(portIndex>=0)
                {
                    openOut(portIndex);
                }
            }
        }
        // Auto load mapping
        loadMapping();
        return true;
    }
    return false;
}

bool MidiInterface::openIn(const unsigned int portIndex)
{
    if(_rtMidiIn)
    {
        if(_connected)
        {
            qDebug() << Q_FUNC_INFO
                     << "Already opened port";
            return false;
        }
        else
        {
            _portIndex = portIndex;
            try {
                _rtMidiIn->openPort(portIndex);
                // Set our callback function.  This should be done immediately after
                // opening the port to avoid having incoming messages written to the
                // queue.
                _rtMidiIn->setCallback( &_midiCallback, this );

                // Don't ignore sysex, timing, or active sensing messages.
                _rtMidiIn->ignoreTypes( false, !_acceptClock, true );
                if(_id == -1)
                    setId(_midi->grabMidiInterfaceId());
                _connected = true;
                qDebug() << "MIDI In connected to: " << this->portName();
                emit(connected());
            } catch ( RtError &error ) {
                error.printMessage();
                _connected = false;
            }
        }
    }
    return _connected;
}

bool MidiInterface::openOut(const unsigned int portIndex)
{
    if(_rtMidiOut)
    {
        if(_hasOutput)
        {
            qDebug() << Q_FUNC_INFO
                     << "Already opened port";
            return false;
        }
        else
        {
            _portIndex = portIndex;
            try {
                _rtMidiOut->openPort(portIndex);
                _hasOutput = true;
                qDebug() << "MIDI Out connected to: " << this->portName();

                // Ask device to give its identity
                sendIdentityRequest();
            } catch ( RtError &error ) {
                error.printMessage();
                _hasOutput = false;
            }
        }
    }
    return _hasOutput;
}

void MidiInterface::sendIdentityRequest()
{
    // Send a Inquiry Request message
    QByteArray sysex;
    sysex.resize(6);
    sysex[0] = SYSEX_HEADER; // MIDI System exclusive message start
    sysex[1] = SYSEX_NON_REALTIME_MESSAGE; // Non-Realtime Message
    sysex[2] = 0x00; // Channel to inquire
    sysex[3] = SYSEX_GENERAL_INFORMATION; // Inquiry Message
    sysex[4] = SYSEX_IDENTITY_REQUEST; // Inquiry Request
    sysex[5] = SYSEX_TERMINATOR; // MIDI System exclusive message terminator
    sendMessage(sysex);
}

bool MidiInterface::sendMessage(const int channel, const int control, const int value)
{
    if (_hasOutput)
    {
        std::vector< unsigned char > message;
        message.push_back(MIDI_CVM_CONTROL_CHANGE | (channel&0x0f));
        message.push_back(control);
        message.push_back(value);
        _rtMidiOut->sendMessage(&message);
    }
    return true;
}

bool MidiInterface::sendMessage(const QByteArray &bytes)
{
    if (_hasOutput)
    {
        std::vector< unsigned char > message;
        for(int i=0; i<bytes.count(); ++i)
        {
            message.push_back(bytes[i]);
        }
        _rtMidiOut->sendMessage(&message);
        qDebug() << Q_FUNC_INFO << "data:" << bytes.toHex();
    }
    return true;
}

bool MidiInterface::close()
{
    if(_rtMidiIn && _connected)
    {
        _rtMidiIn->closePort();
        _rtMidiIn->cancelCallback();
        _connected = false;
        if(_rtMidiOut && _hasOutput)
        {
            _rtMidiOut->closePort();
            _hasOutput = false;
        }
        qDebug() << "MIDI disconnected.";
        emit(connected(false));
        flushMapping();
    }
    return !_connected;
}

bool MidiInterface::isConnected()
{
    return _connected;
}

void MidiInterface::setPortName(QString portName)
{
    setObjectName(portName);
}

QString MidiInterface::portName() const
{
    return objectName();
}

void MidiInterface::autoconnect()
{
    if(isUsed())
    {
        if(!_connected)
            open();
    }
    else
    {
        if(_connected)
            close();
    }
}

void MidiInterface::setAcceptClock(bool on)
{
    _acceptClock = on;
    if(_rtMidiIn)
    {
        // Don't ignore sysex, timing, or active sensing messages.
        _rtMidiIn->ignoreTypes( false, !_acceptClock, true );
    }
    autoconnect();
}

void MidiInterface::setAcceptProgramChange(bool on)
{
    _acceptProgramChange = on;
    autoconnect();
}

void MidiInterface::setAcceptControlChange(bool on)
{
    _acceptControlChange = on;
    autoconnect();
}

void MidiInterface::setAcceptNoteChange(bool on)
{
    _acceptNoteChange = on;
    autoconnect();
}

void MidiInterface::setMapping(const QString& mapping)
{
    if(!_mapping.isEmpty())
    {
        if(_connected) flushMapping();
    }
    _mapping = mapping;

    if(_mapping.isEmpty())
    {
        setAcceptClock(false);
        setAcceptProgramChange(false);
        setAcceptControlChange(false);
        setAcceptNoteChange(false);
        close();
    }
    else
    {
        if(_connected) loadMapping();
    }
}

void MidiInterface::loadMapping()
{
    if(!_mapping.isEmpty())
    {
        MidiMapping * mm = MidiMapping::loadFromFile(_mapping);
        if(mm)
        {
            Minotor::minotor()->midiMapper()->loadMidiMapping(this, mm);
            delete mm;
        } else {
            qDebug() << Q_FUNC_INFO
                     << "Invalid file" << _mapping;
            // Do keep invalid mapping filename
            setMapping("");
        }
    }
}

void MidiInterface::flushMapping()
{
    Minotor::minotor()->midiMapper()->flushMidiMapping(this);
}
