#include "stdafx.h"
#include "AcceleratorManager.h"
#include "resource.h"
#include <afxres.h>

#include <afxtempl.h>  // MFC Templates extension
#ifndef CMapStringToWord
typedef CMap< CString, LPCSTR, WORD, WORD& > CMapStringToWord;
#endif

static CMapStringToWord winAccelStrings;
static bool initialized = false;

struct {
  const char *command;
  WORD id;
} winAccelCommands[] = {
  { "FileOpenGBA", ID_FILE_OPEN_GBA },
  { "FileLoad", ID_FILE_LOAD },
  { "FileSave", ID_FILE_SAVE },
  { "FileLoadGame01", ID_FILE_LOADGAME_SLOT1 },
  { "FileLoadGame02", ID_FILE_LOADGAME_SLOT2 },
  { "FileLoadGame03", ID_FILE_LOADGAME_SLOT3 },
  { "FileLoadGame04", ID_FILE_LOADGAME_SLOT4 },
  { "FileLoadGame05", ID_FILE_LOADGAME_SLOT5 },
  { "FileLoadGame06", ID_FILE_LOADGAME_SLOT6 },
  { "FileLoadGame07", ID_FILE_LOADGAME_SLOT7 },
  { "FileLoadGame08", ID_FILE_LOADGAME_SLOT8 },
  { "FileLoadGame09", ID_FILE_LOADGAME_SLOT9 },
  { "FileLoadGame10", ID_FILE_LOADGAME_SLOT10 },
  { "FileLoadGameAutoLoad", ID_FILE_LOADGAME_AUTOLOADMOSTRECENT },
  { "FileLoadGameRecent", ID_FILE_LOADGAME_MOSTRECENT },
  { "FileSaveGame01", ID_FILE_SAVEGAME_SLOT1 },
  { "FileSaveGame02", ID_FILE_SAVEGAME_SLOT2 },
  { "FileSaveGame03", ID_FILE_SAVEGAME_SLOT3 },
  { "FileSaveGame04", ID_FILE_SAVEGAME_SLOT4 },
  { "FileSaveGame05", ID_FILE_SAVEGAME_SLOT5 },
  { "FileSaveGame06", ID_FILE_SAVEGAME_SLOT6 },
  { "FileSaveGame07", ID_FILE_SAVEGAME_SLOT7 },
  { "FileSaveGame08", ID_FILE_SAVEGAME_SLOT8 },
  { "FileSaveGame09", ID_FILE_SAVEGAME_SLOT9 },
  { "FileSaveGame10", ID_FILE_SAVEGAME_SLOT10 },
  { "FileSaveGameOldest", ID_FILE_SAVEGAME_OLDESTSLOT },
  { "FileRecentReset", ID_FILE_RECENT_RESET },
  { "FileRecentFreeze", ID_FILE_RECENT_FREEZE },
  { "FileRecent01", ID_FILE_MRU_FILE1 },
  { "FileRecent02", ID_FILE_MRU_FILE2 },
  { "FileRecent03", ID_FILE_MRU_FILE3 },
  { "FileRecent04", ID_FILE_MRU_FILE4 },
  { "FileRecent05", ID_FILE_MRU_FILE5 },
  { "FileRecent06", ID_FILE_MRU_FILE6 },
  { "FileRecent07", ID_FILE_MRU_FILE7 },
  { "FileRecent08", ID_FILE_MRU_FILE8 },
  { "FileRecent09", ID_FILE_MRU_FILE9 },
  { "FileRecent10", ID_FILE_MRU_FILE10 },
  { "FilePause", ID_FILE_PAUSE },
  { "FileReset", ID_FILE_RESET },
  { "FileImportBatteryFile", ID_FILE_IMPORT_BATTERYFILE },
  { "FileExportBatteryFile", ID_FILE_EXPORT_BATTERYFILE },
  { "FileRomInformation", ID_FILE_ROMINFORMATION },
  { "FileToggleFullscreen", ID_FILE_TOGGLEMENU },
  { "FileClose", ID_FILE_CLOSE },
  { "FileExit", ID_FILE_EXIT },
  { "OptionsVideoX1", ID_OPTIONS_VIDEO_X1 },
  { "OptionsVideoX2", ID_OPTIONS_VIDEO_X2 },
  { "OptionsVideoX3", ID_OPTIONS_VIDEO_X3 },
  { "OptionsVideoX4", ID_OPTIONS_VIDEO_X4 },
  { "OptionsVideoFullscreenMaxScale", ID_OPTIONS_VIDEO_FULLSCREENMAXSCALE },
  { "OptionsVideoLayersBG0", ID_OPTIONS_VIDEO_LAYERS_BG0 },
  { "OptionsVideoLayersBG1", ID_OPTIONS_VIDEO_LAYERS_BG1 },
  { "OptionsVideoLayersBG2", ID_OPTIONS_VIDEO_LAYERS_BG2 },
  { "OptionsVideoLayersBG3", ID_OPTIONS_VIDEO_LAYERS_BG3 },
  { "OptionsVideoLayersOBJ", ID_OPTIONS_VIDEO_LAYERS_OBJ },
  { "OptionsVideoLayersWIN0", ID_OPTIONS_VIDEO_LAYERS_WIN0 },
  { "OptionsVideoLayersWIN1", ID_OPTIONS_VIDEO_LAYERS_WIN1 },
  { "OptionsVideoLayersOBJWIN", ID_OPTIONS_VIDEO_LAYERS_OBJWIN },
  { "OptionsVideoLayersReset", ID_OPTIONS_VIDEO_LAYERS_RESET },
  { "OptionsEmulatorDirectories", ID_OPTIONS_EMULATOR_DIRECTORIES },
  { "OptionsEmulatorShowSpeedNone", ID_OPTIONS_EMULATOR_SHOWSPEED_NONE },
  { "OptionsEmulatorShowSpeedPercentage", ID_OPTIONS_EMULATOR_SHOWSPEED_PERCENTAGE },
  { "OptionsEmulatorShowSpeedDetailed", ID_OPTIONS_EMULATOR_SHOWSPEED_DETAILED },
  { "OptionsEmulatorShowSpeedTransparent", ID_OPTIONS_EMULATOR_SHOWSPEED_TRANSPARENT },
  { "OptionsSoundChannel1", ID_OPTIONS_SOUND_CHANNEL1 },
  { "OptionsSoundChannel2", ID_OPTIONS_SOUND_CHANNEL2 },
  { "OptionsSoundChannel3", ID_OPTIONS_SOUND_CHANNEL3 },
  { "OptionsSoundChannel4", ID_OPTIONS_SOUND_CHANNEL4 },
  { "OptionsSoundDirectSoundA", ID_OPTIONS_SOUND_DIRECTSOUNDA },
  { "OptionsSoundDirectSoundB", ID_OPTIONS_SOUND_DIRECTSOUNDB },
  { "OptionsJoypadConfigure1", ID_OPTIONS_JOYPAD_CONFIGURE_1 },
  { "OptionsJoypadConfigure2", ID_OPTIONS_JOYPAD_CONFIGURE_2 },
  { "OptionsJoypadConfigure3", ID_OPTIONS_JOYPAD_CONFIGURE_3 },
  { "OptionsJoypadConfigure4", ID_OPTIONS_JOYPAD_CONFIGURE_4 },
  { "OptionsJoypadMotionConfigure", ID_OPTIONS_JOYPAD_MOTIONCONFIGURE },
  { "OptionsJoypadAutofireA", ID_OPTIONS_JOYPAD_AUTOFIRE_A },
  { "OptionsJoypadAutofireB", ID_OPTIONS_JOYPAD_AUTOFIRE_B },
  { "OptionsJoypadAutofireL", ID_OPTIONS_JOYPAD_AUTOFIRE_L },
  { "OptionsJoypadAutofireR", ID_OPTIONS_JOYPAD_AUTOFIRE_R },
  { "ToolsLogging", ID_TOOLS_LOGGING },
  { "ToolsCustomize", ID_TOOLS_CUSTOMIZE },
  { "HelpBugReport", ID_HELP_BUGREPORT },
  { "HelpFAQ", ID_HELP_FAQ },
  { "HelpAbout", ID_HELP_ABOUT },
  { "SystemMinimize", ID_SYSTEM_MINIMIZE }
};

bool winAccelGetID(const char *command, WORD& id)
{
  if(!initialized) {
    int count = sizeof(winAccelCommands)/sizeof(winAccelCommands[0]);

    for(int i = 0; i < count; i++) {
      winAccelStrings.SetAt(winAccelCommands[i].command, winAccelCommands[i].id);
    }
    initialized = true;
  }

  return winAccelStrings.Lookup(command, id) ? true : false;
}

void winAccelAddCommands(CAcceleratorManager& mgr)
{
  int count = sizeof(winAccelCommands)/sizeof(winAccelCommands[0]);

  for(int i = 0; i < count; i++) {
    if(!mgr.AddCommandAccel(winAccelCommands[i].id, winAccelCommands[i].command, false))
      mgr.CreateEntry(winAccelCommands[i].id, winAccelCommands[i].command);
  }

}
