#include "minopropertycolor.h"

MinoPropertyColor::MinoPropertyColor(QObject *parent) :
    MinoProperty(parent)
{
    _mcrHue = new MidiControllableReal(this);
    connect(_mcrHue, SIGNAL(valueChanged(qreal)), this, SLOT(setHue(qreal)));
    _mcrHue->setObjectName("hue");
    _mcrHue->setLabel("Hue");

    _mcrLightness = new MidiControllableReal(this);
    connect(_mcrLightness, SIGNAL(valueChanged(qreal)), this, SLOT(setLightness(qreal)));
    _mcrLightness->setObjectName("light");
    _mcrLightness->setLabel("Light");

    // Set default objectName and label
    setObjectName("color");
    setLabel("Color");
}

void MinoPropertyColor::setColor(QColor color)
{
    _mcrHue->setValue(color.hueF());
    _mcrLightness->setValue(color.lightnessF());
    _color = color;
}

QColor MinoPropertyColor::color()
{
    return _color;
}

void MinoPropertyColor::setHue(qreal value)
{
    _color.setHslF(value,_color.saturationF(),_color.lightnessF());
}

void MinoPropertyColor::setLightness(qreal value)
{
    _color.setHslF(value,_color.saturationF(),_color.lightnessF());
}