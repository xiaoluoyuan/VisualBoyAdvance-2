/*  VisualBoyAdvance 2
    Copyright (C) 2009  VBA development team

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


#ifndef CGBAGRAPHICS_H
#define CGBAGRAPHICS_H


#include "../common/Types.h"


class CGBAGraphics
{
public:
  CGBAGraphics();
  ~CGBAGraphics();

  void setVRAM( const u16 *vram_src );
  void setIO( const u16 *io );
  void render();

private:
  u16 *vram; // video memory

  // video registers
  struct {
    u8 bgMode; // 0-5
    bool frameNumber; // frame buffer 0 or 1
    bool oamAccessDuringHBlank;
    bool objCharMapping;
    bool forcedBlank;
    bool displayBG0;
    bool displayBG1;
    bool displayBG2;
    bool displayBG3;
    bool displayOBJ;
    bool displayWIN0;
    bool displayWIN1;
    bool displayOBJWIN;
  } DISPCNT;
};


#endif // CGBAGRAPHICS_H
