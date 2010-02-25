#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>


inline void printMemory( u32 address, u8 busSize ) {
  iprintf(CON_POS(0,0) "Address: %.8X  Bus: %.2ubit\n\n     ", address, busSize);

  switch( busSize ) {
  case 8: {
      // print first row
      int x;
      for( x = 0; x < 8; x++ ) {
        iprintf("+%.2X",x);
      }

      // print data
      u8 *mem = (u8 *)address;
      for( u32 y = 0; y < 16; y++ ) {
        iprintf("\n%.4X:",(address+(y<<3))&0xFFFF);
        x = 8;
        while( x-- ) {
          iprintf(" %.2X", *(mem++));
        }
      }
    }
    break;
  case 16: {
      int x;
      for( x = 0; x < 8; x+=2 ) {
        iprintf("+%.4X",x);
      }

      u16 *mem = (u16 *)address;
      for( u32 y = 0; y < 16; y++ ) {
        iprintf("\n%.4X:",(address+(y<<3))&0xFFFF);
        x = 4;
        while( x-- ) {
          iprintf(" %.4X", *(mem++));
        }
      }
    }
    break;
  case 32: {
      int x;
      for( x = 0; x < 8; x+=4 ) {
        iprintf("+%.8X",x);
      }

      u32 *mem = (u32 *)address;
      for( u32 y = 0; y < 16; y++ ) {
        iprintf("\n%.4X:",(address+(y<<3))&0xFFFF);
        x = 2;
        while( x-- ) {
          iprintf(" %.8X", *(mem++));
        }
      }
    }
    break;
  }

}


int main(void) {
  u8 busSize = 8; // 8, 16, 32
  u32 address = 0;
  // the vblank interrupt must be enabled for VBlankIntrWait() to work
  // since the default dispatcher handles the bios flags no vblank handler
  // is required
  irqInit();
  irqEnable(IRQ_VBLANK);

  consoleDemoInit();

  bool update = true;
  while (1) {
    scanKeys();
    setRepeat( 20, 5 );
    const u16 keys = keysDownRepeat();
    if( keys & KEY_DOWN ) {
      address+=8;
      update = true;
    }
    if( keys & KEY_UP ) {
      address-=8;
      update = true;
    }
    if( keys & KEY_RIGHT ) {
      address+=0x100;
      update = true;
    }
    if( keys & KEY_LEFT ) {
      address-=0x100;
      update = true;
    }
    if( keys & KEY_R ) {
      address+=0x01000000;
      address&=0xFF000000;
      update = true;
    }
    if( keys & KEY_L ) {
      address-=0x01000000;
      address&=0xFF000000;
      update = true;
    }
    if( keys & KEY_A ) {
      address+=0x10000;
      update = true;
    }
    if( keys & KEY_B ) {
      address-=0x10000;
      update = true;
    }
    if( keys & KEY_SELECT ) {
      switch( busSize ) {
      case 8:
        busSize = 16;
        break;
      case 16:
        busSize = 32;
        break;
      case 32:
        busSize = 8;
        break;
      }
      iprintf(CON_CLS());
      update = true;
    }
    if( update ) {
      update = false;
      printMemory( address, busSize );
    }
    VBlankIntrWait();
  }
}
