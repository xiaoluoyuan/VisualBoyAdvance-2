/*  VisualBoyAdvance 2
    Copyright (C) 2009-2010  VBA development team

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "cdebugwindow_graphics.h"

#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QFileDialog>


CDebugWindow_Graphics::CDebugWindow_Graphics( QWidget *parent )
  : QDialog( parent )
{
  setFixedSize( 256, 256 );
  move( parent->frameGeometry().topRight() );

  for( quint8 s = 0; s < nSurfaces; s++ ) {
    surface[s] = NULL;
    surface[s] = new QImage();
  }

  showBG0();

  menu = new QMenu( this );
  menu->addAction( tr("export to file"), this, SLOT( exportToFile() ) );
  menu->addAction( tr("BG0"), this, SLOT( showBG0() ) );
  menu->addAction( tr("BG1"), this, SLOT( showBG1() ) );
  menu->addAction( tr("BG2"), this, SLOT( showBG2() ) );
  menu->addAction( tr("BG3"), this, SLOT( showBG3() ) );
}


CDebugWindow_Graphics::~CDebugWindow_Graphics()
{
  for( quint8 s = 0; s < nSurfaces; s++ ) {
    delete surface[s];
    surface[s] = NULL;
  }
}


bool CDebugWindow_Graphics::renderFrame( CGBAGraphics::RESULT &data ) {
  for( quint8 s = 0; s < nSurfaces; s++ ) {
    delete surface[s];
    surface[s] = NULL;
  }

  if( data.DISPCNT.displayBG[currentSurface] ) {
      setFixedSize( data.BGIMAGE[currentSurface].width,
                    data.BGIMAGE[currentSurface].height );
      surface[currentSurface] =
          new QImage( (uchar*)data.BGIMAGE[currentSurface].picture,
                      data.BGIMAGE[currentSurface].width,
                      data.BGIMAGE[currentSurface].height,
                      QImage::Format_ARGB32 );
  }
  repaint();
  return true;
}


void CDebugWindow_Graphics::paintEvent( QPaintEvent *event ) {
  if( surface[currentSurface] != NULL ) {
    QPainter p( this );
    p.drawImage( QPoint(0,0), *surface[currentSurface] );
  }
}


void CDebugWindow_Graphics::mousePressEvent ( QMouseEvent *event ) {
  if( event->button() == Qt::RightButton ) {
    menu->popup( event->globalPos() );
  }
}


void CDebugWindow_Graphics::exportToFile() {
  if( surface[currentSurface] == NULL ) return;
  if( surface[currentSurface]->isNull() ) return;
  // create a backup of the current surface because emulation continues while
  // user selects a file name.
  QImage backup( *surface[currentSurface] );
  const QString filter = tr("Portable Network Graphics (*.png)");
  const QString filename = QFileDialog::getSaveFileName( this,
    tr("export surface to file..."), QString(), filter );
  backup.save( filename, "PNG", 100 );
}


void CDebugWindow_Graphics::showBG0() {
  currentSurface = 0;
  setWindowTitle( tr("BG0") );
}


void CDebugWindow_Graphics::showBG1() {
  currentSurface = 1;
  setWindowTitle( tr("BG1") );
}


void CDebugWindow_Graphics::showBG2() {
  currentSurface = 2;
  setWindowTitle( tr("BG2") );
}


void CDebugWindow_Graphics::showBG3() {
  currentSurface = 3;
  setWindowTitle( tr("BG3") );
}
