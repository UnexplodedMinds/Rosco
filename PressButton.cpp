/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include "PressButton.h"


PressButton::PressButton( QWidget *pParent )
    : QLabel( pParent )
{
}


void PressButton::mousePressEvent( QMouseEvent *pEvent )
{
    emit clicked();
    QLabel::mousePressEvent( pEvent );
}
