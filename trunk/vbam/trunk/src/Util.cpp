#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "System.h"
#include "NLS.h"
#include "Util.h"
#include "gba/Flash.h"
#include "gba/GBA.h"
#include "gba/Globals.h"
#include "gba/RTC.h"
#include "common/Port.h"

#include "common/fex.h"

#ifndef _MSC_VER
#define _stricmp strcasecmp
#endif // ! _MSC_VER

extern int systemColorDepth;
extern int systemRedShift;
extern int systemGreenShift;
extern int systemBlueShift;

extern u16 systemColorMap16[0x10000];
extern u32 systemColorMap32[0x10000];

void utilPutDword(u8 *p, u32 value)
{
  *p++ = value & 255;
  *p++ = (value >> 8) & 255;
  *p++ = (value >> 16) & 255;
  *p = (value >> 24) & 255;
}

void utilPutWord(u8 *p, u16 value)
{
  *p++ = value & 255;
  *p = (value >> 8) & 255;
}

bool utilWriteBMPFile(const char *fileName, int w, int h, u8 *pix)
{
  u8 writeBuffer[512 * 3];

  FILE *fp = fopen(fileName,"wb");

  if(!fp) {
    systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), fileName);
    return false;
  }

  struct {
    u8 ident[2];
    u8 filesize[4];
    u8 reserved[4];
    u8 dataoffset[4];
    u8 headersize[4];
    u8 width[4];
    u8 height[4];
    u8 planes[2];
    u8 bitsperpixel[2];
    u8 compression[4];
    u8 datasize[4];
    u8 hres[4];
    u8 vres[4];
    u8 colors[4];
    u8 importantcolors[4];
    //    u8 pad[2];
  } bmpheader;
  memset(&bmpheader, 0, sizeof(bmpheader));

  bmpheader.ident[0] = 'B';
  bmpheader.ident[1] = 'M';

  u32 fsz = sizeof(bmpheader) + w*h*3;
  utilPutDword(bmpheader.filesize, fsz);
  utilPutDword(bmpheader.dataoffset, 0x36);
  utilPutDword(bmpheader.headersize, 0x28);
  utilPutDword(bmpheader.width, w);
  utilPutDword(bmpheader.height, h);
  utilPutDword(bmpheader.planes, 1);
  utilPutDword(bmpheader.bitsperpixel, 24);
  utilPutDword(bmpheader.datasize, 3*w*h);

  fwrite(&bmpheader, 1, sizeof(bmpheader), fp);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;

  switch(systemColorDepth) {
  case 16:
    {
      u16 *p = (u16 *)(pix+(w+2)*(h)*2); // skip first black line
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u16 v = *p++;

          *b++ = ((v >> systemBlueShift) & 0x01f) << 3; // B
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        p++; // skip black pixel for filters
        p++; // skip black pixel for filters
        p -= 2*(w+2);
        fwrite(writeBuffer, 1, 3*w, fp);

        b = writeBuffer;
      }
    }
    break;
  case 24:
    {
      u8 *pixU8 = (u8 *)pix+3*w*(h-1);
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          if(systemRedShift > systemBlueShift) {
            *b++ = *pixU8++; // B
            *b++ = *pixU8++; // G
            *b++ = *pixU8++; // R
          } else {
            int red = *pixU8++;
            int green = *pixU8++;
            int blue = *pixU8++;

            *b++ = blue;
            *b++ = green;
            *b++ = red;
          }
        }
        pixU8 -= 2*3*w;
        fwrite(writeBuffer, 1, 3*w, fp);

        b = writeBuffer;
      }
    }
    break;
  case 32:
    {
      u32 *pixU32 = (u32 *)(pix+4*(w+1)*(h));
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u32 v = *pixU32++;

          *b++ = ((v >> systemBlueShift) & 0x001f) << 3; // B
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        pixU32++;
        pixU32 -= 2*(w+1);

        fwrite(writeBuffer, 1, 3*w, fp);

        b = writeBuffer;
      }
    }
    break;
  }

  fclose(fp);

  return true;
}

extern bool cpuIsMultiBoot;

bool utilIsGBAImage(const char * file)
{
  cpuIsMultiBoot = false;
  if(strlen(file) > 4) {
    const char * p = strrchr(file,'.');

    if(p != NULL) {
      if((_stricmp(p, ".agb") == 0) ||
         (_stricmp(p, ".gba") == 0) ||
         (_stricmp(p, ".bin") == 0) ||
         (_stricmp(p, ".elf") == 0))
        return true;
      if(_stricmp(p, ".mb") == 0) {
        cpuIsMultiBoot = true;
        return true;
      }
    }
  }

  return false;
}

bool utilIsGBImage(const char * file)
{
  if(strlen(file) > 4) {
    const char * p = strrchr(file,'.');

    if(p != NULL) {
      if((_stricmp(p, ".dmg") == 0) ||
         (_stricmp(p, ".gb") == 0) ||
         (_stricmp(p, ".gbc") == 0) ||
         (_stricmp(p, ".cgb") == 0) ||
         (_stricmp(p, ".sgb") == 0))
        return true;
    }
  }

  return false;
}

bool utilIsGzipFile(const char *file)
{
  if(strlen(file) > 3) {
    const char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gz") == 0)
        return true;
      if(_stricmp(p, ".z") == 0)
        return true;
    }
  }

  return false;
}

// strip .gz or .z off end
void utilStripDoubleExtension(const char *file, char *buffer)
{
  if(buffer != file) // allows conversion in place
    strcpy(buffer, file);

  if(utilIsGzipFile(file)) {
    char *p = strrchr(buffer, '.');

    if(p)
      *p = 0;
  }
}

// Opens and scans archive using accept(). Returns File_Extractor if found.
// If error or not found, displays message and returns NULL.
static File_Extractor* scan_arc(const char *file, bool (*accept)(const char *),
		char (&buffer) [2048] )
{
	fex_err_t err;
	File_Extractor* fe = fex_open( file, &err );
	if(!fe)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s: %s"), file, err);
		return NULL;
	}

	// Scan filenames
	bool found=false;
	while(!fex_done(fe)) {
		strncpy(buffer,fex_name(fe),sizeof buffer);
		buffer [sizeof buffer-1] = '\0';

		utilStripDoubleExtension(buffer, buffer);

		if(accept(buffer)) {
			found = true;
			break;
		}

		fex_err_t err = fex_next(fe);
		if(err) {
			systemMessage(MSG_BAD_ZIP_FILE, N_("Cannot read archive %s: %s"), file, err);
			fex_close(fe);
			return NULL;
		}
	}

	if(!found) {
		systemMessage(MSG_NO_IMAGE_ON_ZIP,
									N_("No image found in file %s"), file);
		fex_close(fe);
		return NULL;
	}
	return fe;
}

static bool utilIsImage(const char *file)
{
	return utilIsGBAImage(file) || utilIsGBImage(file);
}

IMAGE_TYPE utilFindType(const char *file)
{
	char buffer [2048];
	if ( !utilIsImage( file ) ) // TODO: utilIsArchive() instead?
	{
		File_Extractor* fe = scan_arc(file,utilIsImage,buffer);
		if(!fe)
			return IMAGE_UNKNOWN;
		fex_close(fe);
		file = buffer;
	}

	return utilIsGBAImage(file) ? IMAGE_GBA : IMAGE_UNKNOWN;
}

static int utilGetSize(int size)
{
  int res = 1;
  while(res < size)
    res <<= 1;
  return res;
}

u8 *utilLoad(const char *file,
             bool (*accept)(const char *),
             u8 *data,
             int &size)
{
	// find image file
	char buffer [2048];
	File_Extractor *fe = scan_arc(file,accept,buffer);
	if(!fe)
		return NULL;

	// Allocate space for image
	int fileSize = fex_size(fe);
	if(size == 0)
		size = fileSize;

	u8 *image = data;

	if(image == NULL) {
		// allocate buffer memory if none was passed to the function
		image = (u8 *)malloc(utilGetSize(size));
		if(image == NULL) {
			fex_close(fe);
			systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
										"data");
			return NULL;
		}
		size = fileSize;
	}

	// Read image
	int read = fileSize <= size ? fileSize : size; // do not read beyond file
	fex_err_t err = fex_read_once(fe, image, read);
	fex_close(fe);
	if(err) {
		systemMessage(MSG_ERROR_READING_IMAGE,
									N_("Error reading image from %s: %s"), buffer, err);
		if(data == NULL)
			free(image);
		return NULL;
	}

	size = fileSize;

	return image;
}

void utilUpdateSystemColorMaps()
{
  switch(systemColorDepth) {
  case 16:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap16[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }
    }
    break;
  case 24:
  case 32:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }
    }
    break;
  }
}

// Check for existence of file.
bool utilFileExists( const char *filename )
{
	FILE *f = fopen( filename, "r" );
	if( f == NULL ) {
		return false;
	} else {
		fclose( f );
		return true;
	}
}
