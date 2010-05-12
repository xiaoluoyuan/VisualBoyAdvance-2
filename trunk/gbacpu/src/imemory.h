/*
VisualBoyAdvance 2, a GBA system emulator
Copyright 2010 Andre-Michel Koehler

This file is part of VisualBoyAdvance 2.

VisualBoyAdvance 2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VisualBoyAdvance 2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VisualBoyAdvance 2. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef IMEMORY_H
#define IMEMORY_H

#include "types.h"

// memory interface
// connects CPU to memory subsystem
// all methods return false if the address was not available
class IMemory {
public:

    virtual bool store32( u32 data, u32 address ) = 0;
    virtual bool store16( u16 data, u32 address ) = 0;
    virtual bool store8( u8 data, u32 address ) = 0;

    virtual bool load32( u32 *data, u32 address ) = 0;
    virtual bool load16( u16 *data, u32 address ) = 0;
    virtual bool load8( u8 *data, u32 address ) = 0;
};

#endif // IMEMORY_H
