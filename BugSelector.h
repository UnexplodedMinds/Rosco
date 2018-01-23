/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __BUGSELECTOR_H__
#define __BUGSELECTOR_H__

#include <QDialog>

#include "ui_BugSelector.h"


class BugSelector : public QDialog, public Ui::BugSelectorBase
{
    Q_OBJECT

public:
    explicit BugSelector( QWidget *pParent );
};

#endif // __BUGSELECTOR_H__
