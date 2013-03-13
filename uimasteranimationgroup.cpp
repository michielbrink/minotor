#include "uimasteranimationgroup.h"

#include "uianimation.h"
#include "minoprogram.h"

#include <QStyle>
#include <QDebug>
#include <QPushButton>

UiMasterAnimationGroup::UiMasterAnimationGroup(MinoAnimationGroup *group, QWidget *parent) :
    QGroupBox(parent),
    _group(group)
{
    QVBoxLayout *lGroupBox = new QVBoxLayout(this);
    lGroupBox->setSpacing(0);
    lGroupBox->setMargin(0);
    lGroupBox->setContentsMargins(1,1,1,1);

    QWidget *wBorder = new QWidget(this);
    wBorder->setObjectName("group");
    lGroupBox->addWidget(wBorder);
    QVBoxLayout *lBorder = new QVBoxLayout(wBorder);
    lBorder->setSpacing(0);
    lBorder->setMargin(0);
    lBorder->setContentsMargins(1,1,1,1);

    _wContent = new QWidget(wBorder);
    lBorder->addWidget(_wContent);
    QVBoxLayout *lContent = new QVBoxLayout(_wContent);
    lContent->setSpacing(2);
    lContent->setMargin(0);
    lContent->setContentsMargins(1,1,1,1);

    QWidget *wTools = new QWidget(_wContent);
    lContent->addWidget(wTools);
    QHBoxLayout *lTools = new QHBoxLayout(wTools);
    lTools->setSpacing(0);
    lTools->setMargin(0);
    lTools->setContentsMargins(6,0,6,0);

    //Delayed button
    QPushButton *pbDelayedEnable = new QPushButton(wTools);
    pbDelayedEnable->setFocusPolicy(Qt::NoFocus);
    pbDelayedEnable->setIcon(QIcon(":/pictos/delayed.png"));
    pbDelayedEnable->setIconSize(QSize(8,8));
    pbDelayedEnable->setMinimumSize(12,12);
    pbDelayedEnable->setMaximumSize(12,12);
    pbDelayedEnable->setCheckable(true);
    connect(pbDelayedEnable, SIGNAL(toggled(bool)), _group, SLOT(setDelayedEnabled(bool)));
    lTools->addWidget(pbDelayedEnable);
    lTools->addStretch();

    //Enable button
    _pbEnable = new QPushButton(wTools);
    _pbEnable->setFocusPolicy(Qt::NoFocus);
    _pbEnable->setIcon(QIcon(":/pictos/power.png"));
    _pbEnable->setIconSize(QSize(20,20));
    _pbEnable->setMinimumSize(20,20);
    _pbEnable->setMaximumSize(20,20);
    _pbEnable->setCheckable(true);
    connect(_pbEnable, SIGNAL(toggled(bool)), _group, SLOT(setEnabled(bool)));
    lTools->addWidget(_pbEnable);
    connect(_pbEnable, SIGNAL(toggled(bool)), pbDelayedEnable, SLOT(setChecked(bool)));

    lTools->addStretch();

    // XXX Spacers

    QFrame *fSeparator = new QFrame(_wContent);
    fSeparator->setObjectName("groupLine");
    fSeparator->setFrameShape(QFrame::HLine);
    fSeparator->setFrameShadow(QFrame::Sunken);
    fSeparator->setLineWidth(1);
    lContent->addWidget(fSeparator);

     _wAnimations = new QWidget(_wContent);
    lContent->addWidget(_wAnimations);
    _lAnimations = new QHBoxLayout(_wAnimations);
    _lAnimations->setSpacing(5);
    _lAnimations->setMargin(0);
    _lAnimations->setContentsMargins(5,0,5,5);

    foreach (MinoAnimation *animation, _group->animations())
    {
        addAnimation(animation);
    }
    lContent->addStretch();
    this->enable(_group->enabled());
    connect(_group, SIGNAL(enabledChanged(bool)), this, SLOT(enable(bool)));
    connect(_group, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

UiMasterAnimationGroup::~UiMasterAnimationGroup()
{
}

void UiMasterAnimationGroup::addAnimation(MinoAnimation *animation, int index)
{
    if (index == -1)
        index = _lAnimations->count();
    UiAnimation *uiAnimation = new UiAnimation(animation, _wAnimations);
    connect(uiAnimation, SIGNAL(animationMoved(int,int)), this, SLOT(_moveAnimation(int,int)));
    uiAnimation->setExpanded(true);
    _lAnimations->insertWidget(index, uiAnimation);
}

void UiMasterAnimationGroup::insertAnimation(UiAnimation *animation, int destId)
{
    _lAnimations->insertWidget(destId, animation);
    animation->setParent(_wAnimations);
}

void UiMasterAnimationGroup::enable(const bool on)
{
    _pbEnable->setChecked(on);
    this->setProperty("active", on);
    this->style()->unpolish(this);
    this->style()->polish(this);
}

UiAnimation* UiMasterAnimationGroup::takeAt(int index)
{
    QLayoutItem *li = _lAnimations->takeAt(index);
    if(li->widget())
    {
        return dynamic_cast<UiAnimation*>(li->widget());
    }
    else
    {
        return NULL;
    }
}