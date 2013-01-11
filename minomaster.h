#ifndef MINOMASTER_H
#define MINOMASTER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>

#include "minochannel.h"

class MinoMaster : public QObject
{
    Q_OBJECT
public:
    explicit MinoMaster(MinoChannel *channel1, MinoChannel *channel2, QObject *parent = 0);
    ~MinoMaster();
    QGraphicsScene *scene() { return &_scene; }
    QGraphicsView *view() { return &_view; }
    QGraphicsItemGroup *itemGroup() { return &_itemGroup; }
private:
    QGraphicsScene _scene;
    QGraphicsView _view;
    QGraphicsItemGroup _itemGroup;
    MinoChannel *_channel1;
    MinoChannel *_channel2;
signals:
    
public slots:
    // TODO this is dirty ;-)
    void valueToViewPort(int value);

    void setBrightness(qreal value);
};

#endif // MINOMASTER_H
