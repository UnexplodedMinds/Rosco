/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __CANVAS_H__
#define __CANVAS_H__


struct CanvasConstants
{
    double dW;
    double dW2;
    double dW4;
    double dW5;
    double dW10;
    double dW20;

    double dH;
    double dH2;
    double dH4;
    double dH5;
    double dH10;
    double dH20;
    double dH160;

    double dAspectP;
    double dAspectL;

    int iTinyFontHeight;
    int iSmallFontHeight;
    int iMedFontHeight;
    int iLargeFontHeight;

    int iTinyFontWidth;
};


class Canvas
{
public:
    Canvas( double dWidth, double dHeight );

    CanvasConstants contants();
    int             largeWidth( const QString &qsText );

private:
    CanvasConstants m_preCalc;
};


#endif // __CANVAS_H__

