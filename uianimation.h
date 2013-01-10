#ifndef UIANIMATION_H
#define UIANIMATION_H

#include <QWidget>
#include <QLabel>

#include "uidial.h"
#include "minoanimation.h"

class UiAnimation : public QWidget
{
    Q_OBJECT
public:
    explicit UiAnimation(MinoAnimation *animation, QWidget *parent);
private:

signals:
    void customContextMenuRequested(const QPoint &pos);
public slots:
    //void customContextMenuRequested(const QPoint &pos);
};

#endif // UIANIMATION_H
