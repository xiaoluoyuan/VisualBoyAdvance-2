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


#ifndef CDRIVER_GRAPHICS_H
#define CDRIVER_GRAPHICS_H


#include "../graphics/cgbagraphics.h"


/// graphics driver interface
class CDriver_Graphics
{
public:
    virtual ~CDriver_Graphics() { };

    /// old method, draws only preprocessed data
    virtual bool displayFrame( const void *const data ) { return true; }

    /// new method, combines & renders prepared data.
    /// should also apply special effects (hw accelerated if possible)
    virtual bool renderFrame( CGBAGraphics::RESULT &data ) { return true; }
};


/// dummy graphics driver
class CDummyDriver_Graphics : public CDriver_Graphics
{
public:
    bool displayFrame( const void *const data ) { return true; };
};

#endif // CDRIVER_GRAPHICS_H
