#include "midicontrollablereal.h"

MidiControllableReal::MidiControllableReal(QObject *parent) :
    MidiControllableParameter(parent),
    _value(0)
{
}

void MidiControllableReal::setValue(qreal value)
{
    if(_value != value)
    {
        _value = value;
        MidiControllableParameter::setValueFromMidi(value*127);
        emit valueChanged(_value);
    }
}

void MidiControllableReal::setValueFromMidi(quint8 value)
{
     setValue((qreal)value/127.0);
     MidiControllableParameter::setValueFromMidi(value);
}
