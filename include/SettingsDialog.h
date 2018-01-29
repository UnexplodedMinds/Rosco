/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "ui_SettingsDialog.h"


class SettingsDialog : public QDialog, public Ui::SettingsDialogBase
{
    Q_OBJECT

public:
    explicit SettingsDialog( QWidget *pParent );
};

#endif // SETTINGSDIALOG_H
