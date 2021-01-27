#include <windows.h>

#ifndef __FILESYSTEM_H__
#include "filesystem.h"
#endif

CFileStorage *pStorage_=0;

extern "C" DLL_API CFileStorage *vntfsGetFileStorageObject()
  {
    return pStorage_=pStorage_?pStorage_:new CFileStorage();
  }
extern "C" DLL_API void vntfsDestroyFileStorageObject()
  {
    delete pStorage_;
    pStorage_=0;
  }
BOOL APIENTRY DllMain(HANDLE /*hModule*/,DWORD ul_reason_for_call,LPVOID)
  {
    switch(ul_reason_for_call)
      {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
          break;
        case DLL_PROCESS_DETACH:
          vntfsDestroyFileStorageObject();
      }
    return TRUE;
  }
