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


#include "cemugba.h"
#include "backupmedia.h"

#include "gba/GBA.h"
#include "gba/Sound.h"
#include <assert.h>


CEmuGBA::CEmuGBA()
{
  m_snd = NULL;
  m_soundDriverLoaded = false;

  m_gfx = NULL;
  m_graphicsDriverLoaded = false;

  m_gfxdbg = NULL;
  m_graphicsDebugDriverLoaded = false;

  m_inp = NULL;
  m_inputDriverLoaded = false;

  m_romLoaded = false;
  m_soundInitialized = false;

  m_backupMedia = NULL;
  saveDataLocked = false;
}


CEmuGBA::~CEmuGBA()
{
  if( m_backupMedia != NULL ) {
    delete m_backupMedia;
  }
}


bool CEmuGBA::loadROM( const u8 *const romData, const u32 romSize )
{
  const bool allDriversLoaded =
             m_soundDriverLoaded &&
             m_graphicsDriverLoaded &&
             m_inputDriverLoaded;
  if( !allDriversLoaded ) return false;

  // initialize backup media
  if( m_backupMedia != NULL ) {
    delete m_backupMedia;
    m_backupMedia = NULL;
  }
  m_backupMedia = new BackupMedia( (u32*)romData, romSize );
  backupMedia = m_backupMedia; // set global variable in core

  CPULoadRom( romData, romSize );

  if( m_soundInitialized ) {
    soundReset();
  } else {
    soundDriver = m_snd; // set global variable in core
    soundInit();
    m_soundInitialized = true;
  }
  CPUInit();
  CPUReset();
  m_romLoaded = true;
  return true;
}


bool CEmuGBA::closeROM()
{
  if( m_romLoaded ) {
    CPUCleanUp();
    m_romLoaded = false;
    return true;
  }

  return false;
}


bool CEmuGBA::emulate()
{
  if( !m_romLoaded ) return false;

  // don't emulate while save data is locked
  if( saveDataLocked ) return false;

  CPULoop( cyclesPerFrame );
  return true;
}


bool CEmuGBA::setDriverSound( CDriver_Sound *drv )
{
  if( drv == NULL ) return false;
  m_snd = drv;
  m_soundDriverLoaded = true;
  if( m_soundInitialized ) {
    soundDriver = m_snd;
    soundInit();
  }
  return true;
}


bool CEmuGBA::setDriverGraphics( CDriver_Graphics *drv )
{
  if( drv == NULL ) return false;
  m_gfx = drv;
  graphicsDriver = m_gfx; // set global variable in core
  m_graphicsDriverLoaded = true;
  return true;
}


bool CEmuGBA::setDebugDriverGraphics( CDriver_Graphics *drv )
{
  m_gfxdbg = drv;
  graphicsDebugDriver = m_gfxdbg; // set global variable in core
  m_graphicsDebugDriverLoaded = true;
  return true;
}


bool CEmuGBA::setDriverInput( CDriver_Input *drv )
{
  if( drv == NULL ) return false;
  m_inp = drv;
  inputDriver = m_inp; // set global variable in core
  m_inputDriverLoaded = true;
  return true;
}


void CEmuGBA::toggleFastForward( const bool enable ) {
  fastforward = enable;
}


u32 CEmuGBA::getSaveDataSize() {
  if( m_backupMedia == NULL ) return 0;
  u32 result = 0;
  m_backupMedia->getData( &result );
  return result;
}

u8 *CEmuGBA::lockSaveData() {
  if( m_backupMedia == NULL ) return NULL;
  saveDataLocked = true;
  return m_backupMedia->getData( 0 );
}


void CEmuGBA::unlockSaveData() {
  saveDataLocked = false;
}
