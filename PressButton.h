/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __PRESSBUTTON_H__
#define __PRESSBUTTON_H__

#include <QObject>
#include <QLabel>


class QWidget;


class PressButton : public QLabel
{
    Q_OBJECT

public:
    explicit PressButton( QWidget *pParent );

protected:
    void mousePressEvent( QMouseEvent *pEvent );

signals:
    void clicked();
};

#endif // __PRESSBUTTON_H__
