#include "stdafx.h"
#include "MainWnd.h"

#include <shlwapi.h>

#include "FileDlg.h"
#include "RomInfo.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "Logging.h"

#include "../gba/GBA.h"
#include "../gba/Sound.h"

#include "../version.h"


void MainWnd::OnFileOpenGBA()
{
	if( fileOpenSelect() ) {
		FileRun();
	}
}

void MainWnd::OnFilePause()
{
  theApp.paused = !theApp.paused;
  if(emulating) {
    if(theApp.paused) {
      theApp.wasPaused = true;
      soundPause();
    } else {
      soundResume();
    }
  }
}

void MainWnd::OnUpdateFilePause(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(theApp.paused);
}

void MainWnd::OnFileReset()
{
  if(emulating) {
    theApp.emulator.emuReset();
    systemScreenMessage(winResLoadString(IDS_RESET));
  }
}

void MainWnd::OnUpdateFileReset(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}

void MainWnd::OnUpdateFileRecentFreeze(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(theApp.recentFreeze);

  if(pCmdUI->m_pMenu == NULL)
    return;

  CMenu *pMenu = pCmdUI->m_pMenu;

  int i;
  for(i = 0; i < 10; i++) {
    if(!pMenu->RemoveMenu(ID_FILE_MRU_FILE1+i, MF_BYCOMMAND))
      break;
  }

  for(i = 0; i < 10; i++) {
    CString p = theApp.recentFiles[i];
    if(p.GetLength() == 0)
      break;
    int index = p.ReverseFind('\\');

    if(index != -1)
      p = p.Right(p.GetLength()-index-1);

    pMenu->AppendMenu(MF_STRING, ID_FILE_MRU_FILE1+i, p);
  }
  theApp.winAccelMgr.UpdateMenu((HMENU)*pMenu);
}

BOOL MainWnd::OnFileRecentFile(UINT nID)
{
  if(theApp.recentFiles[(nID&0xFFFF)-ID_FILE_MRU_FILE1].GetLength()) {
    theApp.filename = theApp.recentFiles[(nID&0xFFFF)-ID_FILE_MRU_FILE1];
    if(FileRun())
      emulating = true;
    else {
      emulating = false;
      soundPause();
    }
  }
  return TRUE;
}

void MainWnd::OnFileRecentReset()
{
  int i = 0;
  for(i = 0; i < 10; i++)
    theApp.recentFiles[i] = "";
}

void MainWnd::OnFileRecentFreeze()
{
  theApp.recentFreeze = !theApp.recentFreeze;
}

void MainWnd::OnFileExit()
{
  SendMessage(WM_CLOSE);
}

void MainWnd::OnFileClose()
{
  // save battery file before we change the filename...
  if(rom != NULL) {
    writeBatteryFile();
    soundPause();
    theApp.emulator.emuCleanUp();
  }
  emulating = 0;
  RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
  SetWindowText( VBA_NAME_AND_SUBVERSION );
}

void MainWnd::OnUpdateFileClose(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}

void MainWnd::OnFileLoad()
{
  CString buffer;
  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    buffer = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    buffer = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(isDriveRoot(saveDir))
    filename.Format("%s%s.sgm", saveDir, buffer);
  else
    filename.Format("%s\\%s.sgm", saveDir, buffer);

  LPCTSTR exts[] = { ".sgm" };
  CString filter = winLoadFilter(IDS_FILTER_SGM);
  CString title = winResLoadString(IDS_SELECT_SAVE_GAME_NAME);

  FileDlg dlg(this, filename, filter, 0, "", exts, saveDir, title, false);

  if(dlg.DoModal() == IDOK) {
    bool res = loadSaveGame(dlg.GetPathName());

    if(res)
      systemScreenMessage(winResLoadString(IDS_LOADED_STATE));
  }
}

void MainWnd::OnUpdateFileLoad(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}

BOOL MainWnd::OnFileLoadSlot(UINT nID)
{
  nID = nID + 1 - ID_FILE_LOADGAME_SLOT1;

  CString buffer;
  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    buffer = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    buffer = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(isDriveRoot(saveDir))
    filename.Format("%s%s%d.sgm", saveDir, buffer, nID);
  else
    filename.Format("%s\\%s%d.sgm", saveDir, buffer, nID);

  CString format = winResLoadString(IDS_LOADED_STATE_N);
  buffer.Format(format, nID);

  bool res = loadSaveGame(filename);

  systemScreenMessage(buffer);

  systemDrawScreen();

  return res;
}

void MainWnd::OnFileSave()
{
  CString buffer;
  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    buffer = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    buffer = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(isDriveRoot(saveDir))
    filename.Format("%s%s.sgm", saveDir, buffer);
  else
    filename.Format("%s\\%s.sgm", saveDir, buffer);

  LPCTSTR exts[] = { ".sgm" };
  CString filter = winLoadFilter(IDS_FILTER_SGM);
  CString title = winResLoadString(IDS_SELECT_SAVE_GAME_NAME);

  FileDlg dlg(this, filename, filter, 0, "", exts, saveDir, title, true);

  if(dlg.DoModal() == IDOK) {
    bool res = writeSaveGame(dlg.GetPathName());
    if(res)
      systemScreenMessage(winResLoadString(IDS_WROTE_STATE));
  }
}

void MainWnd::OnUpdateFileSave(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}

BOOL MainWnd::OnFileSaveSlot(UINT nID)
{
  nID = nID + 1 - ID_FILE_SAVEGAME_SLOT1;

  CString buffer;
  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    buffer = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    buffer = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(isDriveRoot(saveDir))
    filename.Format("%s%s%d.sgm", saveDir, buffer, nID);
  else
    filename.Format("%s\\%s%d.sgm", saveDir, buffer, nID);

  bool res = writeSaveGame(filename);

  CString format = winResLoadString(IDS_WROTE_STATE_N);
  buffer.Format(format, nID);

  systemScreenMessage(buffer);

  systemDrawScreen();

  return res;
}

void MainWnd::OnFileImportBatteryfile()
{
  LPCTSTR exts[] = { ".sav", ".dat" };
  CString filter = winLoadFilter(IDS_FILTER_SAV);
  CString title = winResLoadString(IDS_SELECT_BATTERY_FILE);

  CString saveDir = regQueryStringValue("batteryDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  FileDlg dlg(this, "", filter, 0, "", exts, saveDir, title, false);

  if(dlg.DoModal() == IDCANCEL)
    return;

  CString str1 = winResLoadString(IDS_SAVE_WILL_BE_LOST);

  if(MessageBox(str1,
                NULL,
                MB_OKCANCEL) == IDCANCEL)
    return;

  bool res = false;

  res = theApp.emulator.emuReadBattery(dlg.GetPathName());

  //if(!res)
    //printErrorMessage(ERR_CANNOT_OPEN_FILE, "Cannot open file %s", dlg.GetPathName());
  //else {
    //Removed the reset to allow loading a battery file 'within' a save state.
    //theApp.emulator.emuReset();
  //}
}

void MainWnd::OnUpdateFileImportBatteryfile(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}


void MainWnd::OnFileExportBatteryfile()
{
  CString name;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    name = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    name = theApp.filename;

  LPCTSTR exts[] = {".sav", ".dat" };

  CString filter = winLoadFilter(IDS_FILTER_SAV);
  CString title = winResLoadString(IDS_SELECT_BATTERY_FILE);

  CString saveDir = regQueryStringValue("batteryDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  FileDlg dlg(this,
              name,
              filter,
              1,
              "SAV",
              exts,
              saveDir,
              title,
              true);

  if(dlg.DoModal() == IDCANCEL) {
    return;
  }

  bool result = false;

  if(theApp.cartridgeType == IMAGE_GBA)
    result = theApp.emulator.emuWriteBattery(dlg.GetPathName());

  //if(!result)
    //printErrorMessage(ERR_ERROR_CREATING_FILE, "Error creating file %s",
                  //dlg.GetPathName());
}

void MainWnd::OnUpdateFileExportBatteryfile(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}


void MainWnd::OnFileRominformation()
{
  if(theApp.cartridgeType == IMAGE_GBA) {
    RomInfoGBA dlg(rom);
    dlg.DoModal();
  }
}

void MainWnd::OnUpdateFileRominformation(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
}

//OnFileToggleFullscreen
void MainWnd::OnFileTogglemenu()
{
	if( theApp.videoOption <= VIDEO_4X ) {
		// switch to full screen
		toolsLoggingClose(); // close log dialog
		theApp.updateWindowSize( theApp.lastFullscreen );
	} else {
		// switch to windowed mode
		theApp.updateWindowSize( theApp.lastWindowed );
	}
}

void MainWnd::OnUpdateFileTogglemenu(CCmdUI* pCmdUI)
{
	// HACK: when uncommented, Esc key will not be send to MainWnd
	//pCmdUI->Enable(theApp.videoOption > VIDEO_4X);
}


void MainWnd::OnFileSavegameOldestslot()
{
  if(!emulating)
    return;

  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    filename = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    filename = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(!isDriveRoot(saveDir))
    saveDir += "\\";

  CString name;
  CFileStatus status;
  CString str;
  time_t time = 0;
  int found = 0;

  for(int i = 0; i < 10; i++) {
    name.Format("%s%s%d.sgm", saveDir, filename, i+1);

    if(emulating && CFile::GetStatus(name, status)) {
      if( (status.m_mtime.GetTime() < time) || !time ) {
        time = status.m_mtime.GetTime();
        found = i;
      }
    } else {
      found = i;
      break;
    }
  }
  OnFileSaveSlot(ID_FILE_SAVEGAME_SLOT1+found);
}

void MainWnd::OnUpdateFileSavegameOldestslot(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);
  if(pCmdUI->m_pSubMenu != NULL) {
    CMenu *pMenu = pCmdUI->m_pSubMenu;
    CString filename;

    int index = theApp.filename.ReverseFind('\\');

    if(index != -1)
      filename = theApp.filename.Right(theApp.filename.GetLength()-index-1);
    else
      filename = theApp.filename;

    CString saveDir = regQueryStringValue("saveDir", NULL);
	if( saveDir[0] == '.' ) {
		// handle as relative path
		char baseDir[MAX_PATH+1];
		GetModuleFileName( NULL, baseDir, MAX_PATH );
		baseDir[MAX_PATH] = '\0'; // for security reasons
		PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
		strcat( baseDir, "\\" );
		strcat( baseDir, saveDir );
		saveDir = baseDir;
	}

    if(saveDir.IsEmpty())
      saveDir = getDirFromFile(theApp.filename);

    if(!isDriveRoot(saveDir))
      saveDir += "\\";

    CString name;
    CFileStatus status;
    CString str;

    for(int i = 0; i < 10; i++) {
      name.Format("%s%s%d.sgm", saveDir, filename, i+1);

      if(emulating && CFile::GetStatus(name, status)) {
        CString timestamp = status.m_mtime.Format("%Y/%m/%d %H:%M:%S");
        str.Format("%d %s", i+1, timestamp);
      } else {
        str.Format("%d ----/--/-- --:--:--", i+1);
      }
      pMenu->ModifyMenu(ID_FILE_SAVEGAME_SLOT1+i, MF_STRING|MF_BYCOMMAND, ID_FILE_SAVEGAME_SLOT1+i, str);
    }

    theApp.winAccelMgr.UpdateMenu(pMenu->GetSafeHmenu());
  }
}

void MainWnd::OnFileLoadgameMostrecent()
{
  if(!emulating)
    return;

  CString filename;

  int index = theApp.filename.ReverseFind('\\');

  if(index != -1)
    filename = theApp.filename.Right(theApp.filename.GetLength()-index-1);
  else
    filename = theApp.filename;

  CString saveDir = regQueryStringValue("saveDir", NULL);
  if( saveDir[0] == '.' ) {
	  // handle as relative path
	  char baseDir[MAX_PATH+1];
	  GetModuleFileName( NULL, baseDir, MAX_PATH );
	  baseDir[MAX_PATH] = '\0'; // for security reasons
	  PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
	  strcat( baseDir, "\\" );
	  strcat( baseDir, saveDir );
	  saveDir = baseDir;
	}

  if(saveDir.IsEmpty())
    saveDir = getDirFromFile(theApp.filename);

  if(!isDriveRoot(saveDir))
    saveDir += "\\";

  CString name;
  CFileStatus status;
  CString str;
  time_t time = 0;
  int found = -1;

  for(int i = 0; i < 10; i++) {
    name.Format("%s%s%d.sgm", saveDir, filename, i+1);

    if(emulating && CFile::GetStatus(name, status)) {
if(status.m_mtime.GetTime() > time) {
        time = status.m_mtime.GetTime();
        found = i;
      }
    }
  }
  if(found != -1) {
    OnFileLoadSlot(ID_FILE_LOADGAME_SLOT1+found);
  }
}

void MainWnd::OnUpdateFileLoadgameMostrecent(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(emulating);

  if(pCmdUI->m_pSubMenu != NULL) {
    CMenu *pMenu = pCmdUI->m_pSubMenu;
    CString filename;

    int index = theApp.filename.ReverseFind('\\');

    if(index != -1)
      filename = theApp.filename.Right(theApp.filename.GetLength()-index-1);
    else
      filename = theApp.filename;

    CString saveDir = regQueryStringValue("saveDir", NULL);
	if( saveDir[0] == '.' ) {
		// handle as relative path
		char baseDir[MAX_PATH+1];
		GetModuleFileName( NULL, baseDir, MAX_PATH );
		baseDir[MAX_PATH] = '\0'; // for security reasons
		PathRemoveFileSpec( baseDir ); // removes the trailing file name and backslash
		strcat( baseDir, "\\" );
		strcat( baseDir, saveDir );
		saveDir = baseDir;
	}

    if(saveDir.IsEmpty())
      saveDir = getDirFromFile(theApp.filename);

    if(!isDriveRoot(saveDir))
      saveDir += "\\";

    CString name;
    CFileStatus status;
    CString str;

    for(int i = 0; i < 10; i++) {
      name.Format("%s%s%d.sgm", saveDir, filename, i+1);

      if(emulating && CFile::GetStatus(name, status)) {
        CString timestamp = status.m_mtime.Format("%Y/%m/%d %H:%M:%S");
        str.Format("%d %s", i+1, timestamp);
      } else {
        str.Format("%d ----/--/-- --:--:--", i+1);
      }
      pMenu->ModifyMenu(ID_FILE_LOADGAME_SLOT1+i, MF_STRING|MF_BYCOMMAND, ID_FILE_LOADGAME_SLOT1+i, str);
    }

    theApp.winAccelMgr.UpdateMenu(pMenu->GetSafeHmenu());
  }
}

void MainWnd::OnUpdateFileLoadGameSlot(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(emulating);
}

void MainWnd::OnUpdateFileSaveGameSlot(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(emulating);
}

void MainWnd::OnFileLoadgameAutoloadmostrecent()
{
  theApp.autoLoadMostRecent = !theApp.autoLoadMostRecent;
}

void MainWnd::OnUpdateFileLoadgameAutoloadmostrecent(CCmdUI* pCmdUI)
{
  pCmdUI->SetCheck(theApp.autoLoadMostRecent);
}
