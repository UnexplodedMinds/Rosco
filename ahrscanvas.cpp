/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QPainter>
#include <QtDebug>
#include <QMouseEvent>
#include <QTimer>
#include <QFont>
#include <QLinearGradient>
#include <QLineF>

#include "AHRSCanvas.h"
#include "BugSelector.h"
#include "Keypad.h"
#include "PressButton.h"
#include "AHRSMainWin.h"


extern bool g_bEmulated;


AHRSCanvas::AHRSCanvas( QWidget *parent )
    : QWidget( parent ),
      m_pCanvas( 0 ),
      m_bInitialized( false ),
      m_iHeadBugAngle( -1 ),
      m_iWindBugAngle( -1 ),
      m_pRollIndicator( 0 ),
      m_pHeadIndicator( 0 ),
      m_pAltTape( 0 ),
      m_pSpeedTape( 0 ),
      m_pVertSpeedTape( 0 ),
      m_bTrafficOn( true ),
      m_bHideGPSLocation( false )
{
    m_planeIcon.load( ":/graphics/resources/Plane.png" );
    m_headIcon.load( ":/icons/resources/HeadingIcon.png" );
    m_windIcon.load( ":/icons/resources/WindIcon.png" );
    m_trafficAltKey.load( ":/graphics/resources/AltitudeKey.png" );
    m_headIcon = m_headIcon.scaled( 80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    m_windIcon = m_windIcon.scaled( 80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

    m_dDPIMult = static_cast<double>( physicalDpiX() ) / 144.0; // Android DPI vs Desktop DPI

    QTimer::singleShot( 500, this, SLOT( init() ) );
}


AHRSCanvas::~AHRSCanvas()
{
    if( m_pRollIndicator != 0 )
    {
        delete m_pRollIndicator;
        m_pRollIndicator = 0;
    }
    if( m_pHeadIndicator != 0 )
    {
        delete m_pHeadIndicator;
        m_pHeadIndicator = 0;
    }
    if( m_pAltTape != 0 )
    {
        delete m_pAltTape;
        m_pAltTape = 0;
    }
    if( m_pSpeedTape != 0 )
    {
        delete m_pSpeedTape;
        m_pSpeedTape = 0;
    }
    if( m_pVertSpeedTape != 0 )
    {
        delete m_pVertSpeedTape;
        m_pVertSpeedTape = 0;
    }
    if( m_pCanvas != 0 )
    {
        delete m_pCanvas;
        m_pCanvas = 0;
    }
}


void AHRSCanvas::init()
{
    m_pCanvas = new Canvas( width(), height() );

    CanvasConstants c = m_pCanvas->contants();

    m_iAltSpeedOffset = static_cast<int>( static_cast<double>( c.iTinyFontHeight ) * 0.37 );

    m_pRollIndicator = new QPixmap( static_cast<int>( c.dW2 ), static_cast<int>( c.dH2 / c.dAspectP ) );
    m_pHeadIndicator = new QPixmap( static_cast<int>( c.dW2 ), static_cast<int>( c.dH2 / c.dAspectP ) );
    m_pAltTape = new QPixmap( static_cast<int>( c.dW5 ) - 50, c.iTinyFontHeight * 400 );    // 20000 ft / 100 x double the font height
    m_pVertSpeedTape = new QPixmap( 50, c.dH2 );
    m_pSpeedTape = new QPixmap( static_cast<int>( c.dW5 ), c.iTinyFontHeight * 60 );        // 300 Knots x double the font height
    m_pRollIndicator->fill( Qt::transparent );
    m_pHeadIndicator->fill( Qt::transparent );
    m_pAltTape->fill( Qt::transparent );
    m_pSpeedTape->fill( Qt::transparent );
    m_pVertSpeedTape->fill( Qt::transparent );
    buildRollIndicator();
    buildHeadingIndicator();
    buildAltTape();
    buildSpeedTape();
    buildVertSpeedTape();
    m_bInitialized = true;

    connect( static_cast<AHRSMainWin *>( parentWidget()->parentWidget() ), SIGNAL( trafficToggled( bool ) ), this, SLOT( trafficToggled( bool ) ) );
}


void AHRSCanvas::resizeEvent( QResizeEvent *pEvent )
{
    if( pEvent == 0 )
        return;

    if( m_bInitialized )
    {
        m_bInitialized = false;
        delete m_pRollIndicator;
        delete m_pHeadIndicator;
        delete m_pAltTape;
        delete m_pSpeedTape;
        delete m_pVertSpeedTape;
        init();
    }
}


void AHRSCanvas::paintEvent( QPaintEvent *pEvent )
{
    if( (!m_bInitialized) || (pEvent == 0) )
        return;

    QPainter        ahrs( this );
    CanvasConstants c = m_pCanvas->contants();
    double          dPitchH = c.dH2 + (m_situation.dAHRSpitch / 22.5 * c.dH2);     // The visible portion is only 1/4 of the 90 deg range
    double          dArrowOffset = g_bEmulated ? 20 : 30;
    QString         qsHead( QString::number( static_cast<int>( m_situation.dAHRSGyroHeading ) ) );
    QPolygon        shape;
    QPen            linePen( Qt::black );
    double          dSlipSkid = c.dW2 - ((m_situation.dAHRSSlipSkid / 100.0) * c.dW2);
    QFont           tiny( "Roboto", 12, QFont::Normal );
    QFont           small( "Roboto", 16, QFont::Bold );
    QFont           med( "Roboto", 18, QFont::Bold );
    QFont           large( "Roboto", 24, QFont::Bold );

    if( dSlipSkid < (c.dW4 + 25.0) )
        dSlipSkid = c.dW4 + 25.0;
    else if( dSlipSkid > (c.dW2 + c.dW4 - 25.0) )
        dSlipSkid = c.dW2 + c.dW4 - 25.0;

    linePen.setWidth( 3 );

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );

    // Translate to dead center and rotate by stratux roll then translate back
    ahrs.translate( c.dW2, c.dH2 );
    ahrs.rotate( -m_situation.dAHRSroll );
    ahrs.translate( -c.dW2, -c.dH2 );

    // Top half sky blue gradient offset by stratux pitch
    QLinearGradient skyGradient( 0.0, -c.dH2, 0.0, dPitchH );
    skyGradient.setColorAt( 0, Qt::blue );
    skyGradient.setColorAt( 1, QColor( 85, 170, 255 ) );
    ahrs.fillRect( -400.0, -c.dH2, c.dW + 800.0, dPitchH + c.dH2, skyGradient );

    // Draw brown gradient horizon half offset by stratux pitch
    QLinearGradient groundGradient( 0.0, dPitchH, 0, c.dH + c.dH2 );
    groundGradient.setColorAt( 0, QColor( 170, 85, 0  ) );
    groundGradient.setColorAt( 1, Qt::black );
    ahrs.fillRect( -400.0, dPitchH, c.dW + 800.0, c.dH + c.dH2, groundGradient );
    ahrs.setPen( linePen );
    ahrs.drawLine( -400, dPitchH, c.dW + 400.0, dPitchH );

    ahrs.setClipRect( 0, (m_pRollIndicator->height() / 3) + c.iLargeFontHeight + 50.0, c.dW, c.dH );
    for( int i = 0; i < 50; i += 10 )
    {
        linePen.setColor( Qt::cyan );
        ahrs.setPen( linePen );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 2.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 2.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 5.0) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 5.0) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 7.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 7.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW5, dPitchH - ((i + 10.0) / 22.5 * c.dH2), c.dW2 + c.dW5, dPitchH - (( i + 10.0) / 22.5 * c.dH2) );
        linePen.setColor( QColor( 67, 33, 9 ) );
        ahrs.setPen( linePen );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 2.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 2.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 5.0) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 5.0) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 7.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 7.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW5, dPitchH + ((i + 10.0) / 22.5 * c.dH2), c.dW2 + c.dW5, dPitchH + (( i + 10.0) / 22.5 * c.dH2) );
    }
    ahrs.setClipping( false );

    // Reset rotation
    ahrs.resetTransform();

    // Slip/Skid indicator
    ahrs.setPen( QPen( Qt::white, 5 ) );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW2 - c.dW4, 1, c.dW2, c.iLargeFontHeight );
    ahrs.drawRect( c.dW2 - 30.0, 1.0, 60.0, c.iLargeFontHeight );
    ahrs.setPen( Qt::NoPen );
    ahrs.setBrush( Qt::white );
    ahrs.drawEllipse( dSlipSkid - 25.0,
                      1.0,
                      50.0,
                      c.iLargeFontHeight );

    // Draw the top roll indicator
    ahrs.translate( c.dW2, c.iLargeFontHeight + c.dH4 + 20.0 );
    ahrs.rotate( -m_situation.dAHRSroll );
    ahrs.translate( -c.dW2, -(c.iLargeFontHeight + c.dH4 + 20.0) );
    ahrs.drawPixmap( c.dW2 - c.dW4, (c.iLargeFontHeight * 2) + 20.0, *m_pRollIndicator );
    ahrs.resetTransform();

    QPolygonF arrow;

    arrow.append( QPointF( c.dW2, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) ) );
    arrow.append( QPointF( c.dW2 + dArrowOffset, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) + dArrowOffset ) );
    arrow.append( QPointF( c.dW2 - dArrowOffset, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) + dArrowOffset ) );
    ahrs.setBrush( Qt::white );
    ahrs.setPen( Qt::black );
    ahrs.drawPolygon( arrow );

    // Draw the yellow pitch indicators
    ahrs.setBrush( Qt::yellow );
    shape.append( QPoint( c.dW5 + c.dW20, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 - c.dW10 + (g_bEmulated ? 10 : 20) , c.dH2 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 + c.dH160 ) );
    shape.append( QPoint( c.dW5 + c.dW20, c.dH2 + c.dH160 ) );
    ahrs.drawPolygon( shape );
    shape.clear();
    shape.append( QPoint( c.dW - c.dW5 - c.dW20, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 + c.dW10 - (g_bEmulated ? 10 : 20), c.dH2 ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 + c.dH160 ) );
    shape.append( QPoint( c.dW - c.dW5 - c.dW20, c.dH2 + c.dH160 ) );
    ahrs.drawPolygon( shape );
    shape.clear();
    shape.append( QPoint( c.dW2, c.dH2 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 + (g_bEmulated ? 20 : 40) ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 + (g_bEmulated ? 20 : 40) ) );
    ahrs.drawPolygon( shape );

    // Draw the heading value over the indicator
    ahrs.setPen( QPen( Qt::white, 5 ) );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW2 - c.dW10, c.dH - m_pHeadIndicator->height() - 45.0 - c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( large );
    ahrs.drawText( c.dW2 - (m_pCanvas->largeWidth( qsHead ) / 2), c.dH - m_pHeadIndicator->height() - 45.0 - m_iAltSpeedOffset, qsHead );

    // Arrow for heading position above heading dial
    arrow.clear();
    arrow.append( QPointF( c.dW2, c.dH - m_pHeadIndicator->height() - 15.0 ) );
    arrow.append( QPointF( c.dW2 + dArrowOffset, c.dH - m_pHeadIndicator->height() - 35.0 ) );
    arrow.append( QPointF( c.dW2 - dArrowOffset, c.dH - m_pHeadIndicator->height() - 35.0 ) );
    ahrs.setBrush( Qt::white );
    ahrs.setPen( Qt::black );
    ahrs.drawPolygon( arrow );

    // Draw the heading pixmap and rotate it to the current heading
    ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
    ahrs.rotate( -m_situation.dAHRSGyroHeading );
    ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
    ahrs.drawPixmap( c.dW2 - (m_pHeadIndicator->width() / 2), c.dH - m_pHeadIndicator->height() - 10.0, *m_pHeadIndicator );
    ahrs.resetTransform();

    // Draw the central airplane
    ahrs.drawPixmap( QRect( c.dW2 - c.dW20, c.dH - 10 - (m_pHeadIndicator->height() / 2) - c.dH20, c.dW10, c.dH10 ), m_planeIcon );

    // Draw the heading bug
    if( m_iHeadBugAngle >= 0 )
    {
        ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
        ahrs.rotate( m_iHeadBugAngle + m_situation.dAHRSGyroHeading );
        ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
        ahrs.drawPixmap( c.dW2 - 50, c.dH - m_pHeadIndicator->height() - 50.0, m_headIcon );
        ahrs.resetTransform();
    }

    // Draw the wind bug
    if( m_iWindBugAngle >= 0 )
    {
        ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
        ahrs.rotate( m_iWindBugAngle + m_situation.dAHRSGyroHeading );
        ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
        ahrs.drawPixmap( c.dW2 - 50, c.dH - m_pHeadIndicator->height() - 50.0, m_windIcon );
        ahrs.resetTransform();
    }

    // Draw the Altitude tape
    linePen.setColor( Qt::white );
    linePen.setWidth( 5 );
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( c.dW - c.dW5, 1.0, c.dW5, c.dH2 - 1.0 );
    ahrs.setClipRect( c.dW - c.dW5 + 1.0, 2.0, c.dW5 - 4.0, c.dH2 - 4.0 );
    ahrs.drawPixmap( c.dW - c.dW5 + 5.0, c.dH4 - (c.iTinyFontHeight * 2) - (((20000.0 - m_situation.dBaroPressAlt) / 20000.0) * m_pAltTape->height()), *m_pAltTape );
    ahrs.setClipping( false );

    // Draw the dividing line and vertical speed static pixmap
    ahrs.drawLine( c.dW - 50.0, 1.0, c.dW - 50.0, c.dH2 - 1.0 );
    ahrs.drawPixmap( c.dW - 50.0, 0.0, *m_pVertSpeedTape );

    // Draw the vertical speed indicator
    ahrs.translate( 0.0, m_situation.dGPSVertSpeed / 1000.0 * c.dH4 );
    arrow.clear();
    arrow.append( QPoint( c.dW - dArrowOffset, c.dH4 ) );
    arrow.append( QPoint( c.dW, c.dH4 - dArrowOffset ) );
    arrow.append( QPoint( c.dW, c.dH4 + dArrowOffset ) );
    ahrs.setPen( Qt::black );
    ahrs.setBrush( Qt::white );
    ahrs.drawPolygon( arrow );
    ahrs.resetTransform();

    // Draw the current altitude
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW - c.dW5, c.dH4 - (c.iLargeFontHeight / 2), c.dW5 - 50.0, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( small );
    ahrs.drawText( c.dW - c.dW5 + 5, c.dH4 + (c.iLargeFontHeight / 2) - m_iAltSpeedOffset, QString::number( static_cast<int>( m_situation.dBaroPressAlt ) ) );

    // Draw the Speed tape
    linePen.setColor( Qt::white );
    linePen.setWidth( 5 );
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( 0, 1.0, c.dW5, c.dH2 - 1.0 );
    ahrs.setClipRect( 2.0, 2.0, c.dW5 - 4.0, c.dH2 - 4.0 );
    ahrs.drawPixmap( 3, c.dH4 - c.iSmallFontHeight - (((300.0 - m_situation.dGPSGroundSpeed) / 300.0) * m_pSpeedTape->height()), *m_pSpeedTape );
    ahrs.setClipping( false );

    // Draw the current speed
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( 0, c.dH4 - (c.iLargeFontHeight / 2), c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( large );
    ahrs.drawText( 5, c.dH4 + (c.iLargeFontHeight / 2) - m_iAltSpeedOffset, QString::number( static_cast<int>( m_situation.dGPSGroundSpeed ) ) );

    // Draw the G-Force indicator box and scale
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( 0, c.dH2, c.dW5, c.iLargeFontHeight );
    ahrs.setFont( tiny );
    ahrs.setPen( Qt::white );
    ahrs.drawText( 5, c.dH2 + c.iTinyFontHeight, "2" );
    ahrs.drawText( (c.dW5 / 2) - (c.iTinyFontWidth / 2), c.dH2 + c.iTinyFontHeight, "0" );
    ahrs.drawText( c.dW5 - c.iTinyFontWidth - 5, c.dH2 + c.iTinyFontHeight, "2" );

    // Arrow for G-Force indicator
    arrow.clear();
    arrow.append( QPoint( c.dW10, c.dH2 + c.iLargeFontHeight - dArrowOffset ) );
    arrow.append( QPoint( c.dW10 - dArrowOffset, c.dH2 + c.iLargeFontHeight ) );
    arrow.append( QPoint( c.dW10 + dArrowOffset, c.dH2 + c.iLargeFontHeight ) );
    ahrs.setPen( Qt::black );
    ahrs.setBrush( Qt::white );
    ahrs.translate( (m_situation.dAHRSGLoad - 1.0) * c.dW5, 0.0 );
    ahrs.drawPolygon( arrow );
    ahrs.resetTransform();

    // GPS Lat/Long
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW - c.dW5, c.dH2, c.dW5, c.iLargeFontHeight );
    ahrs.drawRect( c.dW - c.dW5, c.dH2 + c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::green );
    ahrs.setFont( med );
    QString qsLat = QString( "%1 %2" )
                        .arg( m_bHideGPSLocation ? 12.3456 : fabs( m_situation.dGPSlat ) )
                        .arg( (m_situation.dGPSlat < 0.0) ? "E" : "W" );
    QString qsLong = QString( "%1 %2" )
                        .arg( m_bHideGPSLocation ? 34.5678 : fabs( m_situation.dGPSlong ) )
                        .arg( (m_situation.dGPSlong < 0.0) ? "N" : "S" );
    ahrs.drawText( c.dW - c.dW5 + 8.0, c.dH2 + c.iLargeFontHeight - m_iAltSpeedOffset - 4, qsLat );
    ahrs.drawText( c.dW - c.dW5 + 8.0, c.dH2 + (c.iLargeFontHeight * 2) - m_iAltSpeedOffset - 4, qsLong );

    // Traffic altitude key
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( 0, c.dH2 + c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::NoPen );
    ahrs.drawPixmap( 3.0, c.dH2 + c.iLargeFontHeight + 2.0, c.dW5 - 5.0, c.iLargeFontHeight - 4.0, m_trafficAltKey );

    if( m_bTrafficOn )
        updateTraffic( &ahrs, c.dH2 + (c.iLargeFontHeight * 2.0) + 30.0 );
}


void AHRSCanvas::updateTraffic( QPainter *pAhrs, double dListPos )
{
    QList<StratuxTraffic> trafficList = m_trafficMap.values();
    StratuxTraffic        traffic;
    double                dDistInc = m_pHeadIndicator->height() / 80.0 * 1.75;   // The heading indicator outer diameter = 20NM
    QPen                  planePen( Qt::black, g_bEmulated ? 15 : 30, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin );
    CanvasConstants       c = m_pCanvas->contants();

    if( trafficList.count() > 0 )
    {
        pAhrs->setPen( Qt::black );
        pAhrs->setBrush( QColor( 255, 255, 255, 10 ) );
        pAhrs->drawRect( c.dW - c.dW5 + (g_bEmulated ? 10.0 : 40.0), dListPos - 10.0, c.dW5 - 20.0, c.iTinyFontHeight * (trafficList.count() + 1) );
    }

    // Draw a large dot for each aircraft; the outer edge of the heading indicator is calibrated to be 20 NM out from your position
    foreach( traffic, trafficList )
    {
        planePen.setColor( Qt::gray );
        if( traffic.dAlt >= 2000.0 )
            planePen.setColor( QColor( 0, 128, 128 ) );   // teal
        if( traffic.dAlt >= 5000.0 )
            planePen.setColor( QColor( 128, 0, 128 ) );   // purple
        if( traffic.dAlt >= 10000.0 )
            planePen.setColor( Qt::red );
        if( traffic.dAlt >= 12000.0 )
            planePen.setColor( Qt::magenta );
        if( traffic.dAlt >= 15000.0 )
            planePen.setColor( Qt::green );
        if( traffic.dAlt >= 18000.0 )
            planePen.setColor( Qt::yellow );
        if( traffic.dAlt >= 20000.0 )
            planePen.setColor( Qt::blue );
        if( traffic.dAlt >= 25000.0 )
            planePen.setColor( Qt::cyan );
        if( traffic.dAlt >= 30000.0 )
            planePen.setColor( QColor( 173, 255, 47 ) );  // snot green
        if( traffic.dAlt >= 40000.0 )
            planePen.setColor( QColor( 214, 153, 255 ) ); // lavender

        // If bearing and distance were able to be calculated then show relative position
        if( traffic.bHasADSB )
        {
            pAhrs->translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
            pAhrs->rotate( traffic.dBearing + m_situation.dAHRSMagHeading );
            pAhrs->translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
            planePen.setWidth( g_bEmulated ? 15 : 30 );
            pAhrs->setPen( planePen );
            pAhrs->drawPoint( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 - (traffic.dDist * dDistInc) );
            pAhrs->resetTransform();
        }

        // List the tail numbers along the right side
        planePen.setWidth( 1 );
        pAhrs->setPen( planePen );
        dListPos += c.iTinyFontHeight;
        pAhrs->drawText( c.dW - c.dW5 + (g_bEmulated ? 20.0 : 50.0), dListPos, traffic.qsReg );
    }
}


void AHRSCanvas::situation( StratuxSituation s )
{
    m_situation = s;
    update();
}


void AHRSCanvas::traffic( int iICAO, StratuxTraffic t )
{
    QMapIterator<int, StratuxTraffic> it( m_trafficMap );
    bool                              bTrafficRemoved = true;

    // Each time this is updated, remove an old entry
    while( bTrafficRemoved )
    {
        bTrafficRemoved = false;
        while( it.hasNext() )
        {
            it.next();
            // Anything older than 60 seconds discard
            if( it.value().dAge > 60.0 )
            {
                m_trafficMap.remove( it.key() );
                bTrafficRemoved = true;
                break;
            }
        }
    }

    m_trafficMap.insert( iICAO, t );
    update();
}


void AHRSCanvas::mousePressEvent( QMouseEvent *pEvent )
{
    if( pEvent == 0 )
        return;

    CanvasConstants c = m_pCanvas->contants();
    QPoint          pressPt( pEvent->pos() );
    QRect           headRect( c.dW2 - (m_pHeadIndicator->width() / 2), c.dH - m_pHeadIndicator->height() - 10.0, m_pHeadIndicator->width(), m_pHeadIndicator->height() );
    QRect           gpsRect( c.dW - c.dW5, c.dH2, c.dW5, c.iLargeFontHeight * 2.0 );

    if( headRect.contains( pressPt ) )
    {
        int         iButton = -1;
        BugSelector bugSel( this );

        bugSel.setGeometry( c.dW2 - 250.0 + (g_bEmulated ? 2000 : 0), c.dH - (m_pHeadIndicator->height() / 2) - 150.0, 500.0, 300.0 );

        iButton = bugSel.exec();

        Keypad keypad( this );

        if( keypad.exec() == QDialog::Accepted )
        {
            int iAngle = keypad.value();

            if( (iAngle >= 360) || (iAngle < 0) )
                iAngle = 0;
            if( iButton == QDialog::Accepted )
                m_iHeadBugAngle = iAngle;
            else if( iButton == QDialog::Rejected )
                m_iWindBugAngle = iAngle;
        }
        else
        {
            if( iButton == QDialog::Accepted )
                m_iHeadBugAngle = -1;
            else if( iButton == QDialog::Rejected )
                m_iWindBugAngle = -1;
        }
    }
    else if( gpsRect.contains( pressPt ) )
        m_bHideGPSLocation = (!m_bHideGPSLocation);
}


void AHRSCanvas::buildAltTape()
{
    QPainter        ahrs( m_pAltTape );
    QFont           altFont( "Roboto", 14 );
    int             iAlt, iV = 1, iY;
    CanvasConstants c = m_pCanvas->contants();

    ahrs.setFont( altFont );
    for( iAlt = 20000; iAlt >= 0; iAlt -= 100 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.drawText( 0, iY, QString::number( iAlt ) );
        iY = iY - (c.iTinyFontHeight / 2) + m_iAltSpeedOffset - 2;
        ahrs.drawLine( m_pAltTape->width() - 24, iY, m_pAltTape->width() - 4, iY );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        for( int i = 0; i < 3; i++ )
        {
            iY += (c.iTinyFontHeight / 2 );
            ahrs.drawLine( m_pAltTape->width() - 19, iY, m_pAltTape->width() - 4, iY );
        }
        iV++;
    }
}


void AHRSCanvas::buildVertSpeedTape()
{
    QPainter ahrs( m_pVertSpeedTape );
    QFont    vertFont( "Roboto", 10 );
    int      iVert, iV = 1, iY;
    int      iLineHeight = height() / 80;

    ahrs.setFont( vertFont );
    // For some reason this is slightly off on Android
    if( !g_bEmulated )
        ahrs.scale( 1.0, 0.9 );
    for( iVert = 10; iVert >= -10; iVert-- )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * iLineHeight * 2;
        if( (iVert % 2) == 0 )
            ahrs.drawText( 5, iY, QString::number( abs( iVert ) ) );
        iY -= 3;
        ahrs.drawLine( m_pVertSpeedTape->width() - 14, iY - (g_bEmulated ? 2 : 7), m_pVertSpeedTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        if( iVert != -10 )
        {
            iY += iLineHeight;
            ahrs.drawLine( m_pVertSpeedTape->width() - 9, iY - (g_bEmulated ? 2 : 7), m_pVertSpeedTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        }
        iV++;
    }
}


void AHRSCanvas::buildSpeedTape()
{
    QPainter        ahrs( m_pSpeedTape );
    QFont           speedFont( "Roboto", 24, QFont::Bold );
    int             iSpeed, iV = 1, iY;
    CanvasConstants c = m_pCanvas->contants();

    ahrs.setFont( speedFont );
    for( iSpeed = 300; iSpeed >= 0; iSpeed -= 10 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.drawText( 30, iY + (c.iLargeFontHeight / 6), QString::number( iSpeed ) );
        iY = iY - (c.iTinyFontHeight / 2) + m_iAltSpeedOffset - 2;
        ahrs.drawLine( 0, iY, 25, iY );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        for( int i = 0; i < 3; i++ )
        {
            iY += (c.iTinyFontHeight / 2 );
            ahrs.drawLine( 0, iY, 15, iY );
        }
        iV++;
    }
}


void AHRSCanvas::buildRollIndicator()
{
    QPainter     ahrs( m_pRollIndicator );
    double       dW = m_pRollIndicator->width();
    double       dH = m_pRollIndicator->height();
    double       dW2 = dW / 2.0;
    double       dH2 = dH / 2.0;
    QPen         linePen( Qt::white, 3 );
    QFont        rollFont( "Roboto Thin", 12 );
    QFontMetrics rollMetrics( rollFont );
    QString      qsRoll;
    QRect        rollRect;

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );

    // -30 to 30 with black lines on -30, 0 and 30
    for( int i = 0; i < 7; i++ )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( -30 + (i * 10) );
        ahrs.translate( -dW2, -dH2 );
        if( (i == 0) || (i == 3) || (i == 6) )
            linePen.setColor( QColor( 0xFF, 0xA5, 0x00, 0xFF ) );   // Orange
        else
            linePen.setColor( Qt::white );
        ahrs.setPen( linePen );
        ahrs.drawLine( dW2, 0, dW2, 20 );
        qsRoll = QString::number( abs( -30 + (i * 10) ) );
        qsRoll.chop( 1 );
        rollRect = rollMetrics.boundingRect( qsRoll );
        ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
        ahrs.resetTransform();
    }
    // Red lines on -60, -45, 45 and 60
    linePen.setColor( Qt::red );
    ahrs.setPen( linePen );
    // -45
    qsRoll = "45";
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( -45 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    rollRect = rollMetrics.boundingRect( qsRoll );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // 45
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( 45 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // -60
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( -60 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    qsRoll = "6";
    rollRect = rollMetrics.boundingRect( qsRoll );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // 60
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( 60 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();

    linePen.setColor( Qt::white );
    ahrs.setPen( linePen );
    for( double d = 0; d <= 120.0; d += 0.1 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( -60.0 + d );
        ahrs.translate( -dW2, -dH2 );
        ahrs.drawLine( dW2, 20.0, dW2, 21.0 );
        ahrs.resetTransform();
    }
}


void AHRSCanvas::buildHeadingIndicator()
{
    QPainter ahrs( m_pHeadIndicator );
    double   dW = m_pHeadIndicator->width();
    double   dH = m_pHeadIndicator->height();
    double   dW2 = dW / 2.0;
    double   dH2 = dH / 2.0;
    QPen     linePen( Qt::white, 3 );
    bool     bThis = true;
    QColor   orange( 255, 165, 0 );

    QFont        headFont( "Roboto", 28 );
    QFontMetrics headMetrics( headFont );
    QString      qsHead;
    QRect        headRect;

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );
    // Heading background
    ahrs.setPen( Qt::NoPen );
    ahrs.setBrush( Qt::black );
    ahrs.drawEllipse( 0.0, 0.0, dW, dH );
    ahrs.setPen( linePen );
    // Tens of degrees
    for( int i = 0; i < 360; i += 10 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( i );
        ahrs.translate( -dW2, -dH2 );
        ahrs.drawLine( dW2, 0, dW2, 20 );
        ahrs.resetTransform();
    }
    // Every five degrees
    linePen.setWidth( 2 );
    linePen.setColor( Qt::gray );
    ahrs.setPen( linePen );
    for( int i = 5; i < 360; i += 5 )
    {
        if( bThis )
        {
            ahrs.translate( dW2, dH2 );
            ahrs.rotate( i );
            ahrs.translate( -dW2, -dH2 );
            ahrs.drawLine( dW2, 0, dW2, 10 );
            ahrs.resetTransform();
            bThis = false;
        }
        else
            bThis = true;
    }
    ahrs.resetTransform();
    // Number labels
    ahrs.setFont( headFont );
    for( int i = 0; i < 36; i += 3 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( i * 10 );
        ahrs.translate( -dW2, -dH2 );
        if( i == 0 )
        {
            ahrs.setPen( orange );
            qsHead = "N";
        }
        else if( i == 9 )
        {
            ahrs.setPen( orange );
            qsHead = "E";
        }
        else if( i == 18 )
        {
            ahrs.setPen( orange );
            qsHead = "S";
        }
        else if( i == 27 )
        {
            ahrs.setPen( orange );
            qsHead = "W";
        }
        else
        {
            ahrs.setPen( Qt::white );
            qsHead = QString::number( abs( i ) );
        }
        headRect = headMetrics.boundingRect( qsHead );
        ahrs.drawText( dW2 - (headRect.width() / 2.0), 22.0 + headRect.height(), qsHead );
        ahrs.resetTransform();
    }
}


void AHRSCanvas::trafficToggled( bool bOn )
{
    m_bTrafficOn = bOn;
}

