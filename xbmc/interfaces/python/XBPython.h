#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "cores/IPlayerCallback.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"
#include "threads/Thread.h"
#include "interfaces/IAnnouncer.h"
#include "interfaces/generic/ILanguageInvocationHandler.h"
#include "addons/IAddon.h"

#include <boost/shared_ptr.hpp>
#include <vector>

class CPythonInvoker;

typedef struct {
  int id;
  bool bDone;
  CPythonInvoker* pyThread;
}PyElem;

class LibraryLoader;

namespace XBMCAddon
{
  namespace xbmc
  {
    class Monitor;
  }
}

template <class T> struct LockableType : public T, public CCriticalSection
{ bool hadSomethingRemoved; };

typedef LockableType<std::vector<void*> > PlayerCallbackList;
typedef LockableType<std::vector<XBMCAddon::xbmc::Monitor*> > MonitorCallbackList;
typedef LockableType<std::vector<PyElem> > PyList;
typedef std::vector<LibraryLoader*> PythonExtensionLibraries;

class XBPython :
  public IPlayerCallback,
  public ANNOUNCEMENT::IAnnouncer,
  public ILanguageInvocationHandler
{
public:
  XBPython();
  virtual ~XBPython();
  virtual void OnPlayBackEnded();
  virtual void OnPlayBackStarted();
  virtual void OnPlayBackPaused();
  virtual void OnPlayBackResumed();
  virtual void OnPlayBackStopped();
  virtual void OnPlayBackSpeedChanged(int iSpeed);
  virtual void OnPlayBackSeek(int iTime, int seekOffset);
  virtual void OnPlayBackSeekChapter(int iChapter);
  virtual void OnQueueNextItem();

  virtual void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data);
  void RegisterPythonPlayerCallBack(IPlayerCallback* pCallback);
  void UnregisterPythonPlayerCallBack(IPlayerCallback* pCallback);
  void RegisterPythonMonitorCallBack(XBMCAddon::xbmc::Monitor* pCallback);
  void UnregisterPythonMonitorCallBack(XBMCAddon::xbmc::Monitor* pCallback);
  void OnSettingsChanged(const CStdString &strings);
  void OnScreensaverActivated();
  void OnScreensaverDeactivated();
  void OnDatabaseUpdated(const std::string &database);
  void OnDatabaseScanStarted(const std::string &database);
  void OnAbortRequested(const CStdString &ID="");

  virtual void Process();
  virtual void Uninitialize();
  virtual void OnScriptStarted(ILanguageInvoker *invoker);
  virtual void OnScriptEnded(ILanguageInvoker *invoker);
  virtual ILanguageInvoker* CreateInvoker();

  bool InitializeEngine();
  void FinalizeScript();

  void PulseGlobalEvent();
  bool WaitForEvent(CEvent& hEvent, unsigned int milliseconds);

  // inject xbmc stuff into the interpreter.
  // should be called for every new interpreter
  void InitializeInterpreter(ADDON::AddonPtr addon);

  // remove modules and references when interpreter done
  void DeInitializeInterpreter();

  void RegisterExtensionLib(LibraryLoader *pLib);
  void UnregisterExtensionLib(LibraryLoader *pLib);
  void UnloadExtensionLibs();

private:
  void Finalize();

  CCriticalSection    m_critSection;
  bool              FileExist(const char* strFile);

  void*             m_mainThreadState;
  ThreadIdentifier  m_ThreadId;
  bool              m_bInitialized;
  int               m_iDllScriptCounter; // to keep track of the total scripts running that need the dll
  unsigned int      m_endtime;

  //Vector with list of threads used for running scripts
  PyList              m_vecPyList;
  PlayerCallbackList  m_vecPlayerCallbackList;
  MonitorCallbackList m_vecMonitorCallbackList;
  LibraryLoader*      m_pDll;

  // any global events that scripts should be using
  CEvent m_globalEvent;

  // in order to finalize and unload the python library, need to save all the extension libraries that are
  // loaded by it and unload them first (not done by finalize)
  PythonExtensionLibraries m_extensions;
};

extern XBPython g_pythonParser;
