/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/


#include "SettingsDialog.h"


// The Qt WebEngineView literally does everything we need to do
// (it just displays a web page) so there is nothing to do here but setup the UI.
SettingsDialog::SettingsDialog( QWidget *pParent )
    : QDialog( pParent )
{
    setupUi( this );

    showMaximized();
}
