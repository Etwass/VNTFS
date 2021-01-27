#include "filestream.h"

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef __FILESYSTEM_H__
#include "filesystem.h"
#endif

/******************************************************************************
*                             Private - Õ¿◊¿ÀŒ                                *
******************************************************************************/
/******************************************************************************
*                             Private -  ŒÕ≈÷                                 *
******************************************************************************/
/******************************************************************************
*                             Protected - Õ¿◊¿ÀŒ                              *
******************************************************************************/
long int CFileStream::SetMemoryPointer(unsigned long int uliMethod,long int liOffset)
  {
    switch(uliMethod)
      {
        case FILE_BEGIN:
          return liOffset>m_liSize||liOffset<0?INVALID_SET_FILE_POINTER:m_liOffset=liOffset;
        case FILE_CURRENT:
          return liOffset+m_liOffset>m_liSize||liOffset+m_liOffset<0?INVALID_SET_FILE_POINTER:m_liOffset+=liOffset;
        case FILE_END:
          return m_liSize+liOffset<0||liOffset>0?INVALID_SET_FILE_POINTER:m_liOffset=m_liSize+liOffset;
      }
    return (long int)INVALID_SET_FILE_POINTER;
  }
long int CFileStream::SetFilePointer(unsigned long int uliMethod,long int liOffset)
  {
    return ::SetFilePointer((HANDLE)m_pBuffer,liOffset,NULL,(DWORD)uliMethod);
  }
long int CFileStream::ReadFromMemory(void *pBuffer,long int liLength,long int *pReadden)
  {
    *pReadden=m_liOffset+liLength>m_liSize?m_liSize-m_liOffset:liLength;
    memcpy(pBuffer,(const void *)((char *)m_pBuffer+m_liOffset),*pReadden);
    m_liOffset+=*pReadden;
    return ERROR_SUCCESS;
  }
long int CFileStream::ReadFromFile(void *pBuffer,long int liLength,long int *pReadden)
  {
    if(!ReadFile((HANDLE)m_pBuffer,pBuffer,(DWORD)liLength,(DWORD *)pReadden,NULL))return (long int)GetLastError();
    return ERROR_SUCCESS;
  }
/******************************************************************************
*                             Protected -  ŒÕ≈÷                               *
******************************************************************************/
/******************************************************************************
*                             Public - Õ¿◊¿ÀŒ                                 *
******************************************************************************/
CFileStream::~CFileStream()
  {

  }
long int CFileStream::Initialize(const void *pData)
  {
    switch(m_fFlag)
      {
        case fMemory:
          m_pBuffer=((INITDATA *)pData)->m_pData;
          m_liSize=((INITDATA *)pData)->m_liLength;
          m_pExtraData=((INITDATA *)pData)->m_pExtraData;
          return ERROR_SUCCESS;
        case fFileSystem:
          {
            HANDLE hFile=CreateFile((const char *)((INITDATA *)pData)->m_pData,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

            if(hFile==INVALID_HANDLE_VALUE)return (long int)GetLastError();
            m_pBuffer=hFile;
            m_pExtraData=((INITDATA *)pData)->m_pExtraData;
            return ERROR_SUCCESS;
          }
      }
    return ERROR_INVALID_PARAMETER;
  }
long int CFileStream::fseek(unsigned long int uliMethod,long int liOffset)
  {
    return m_fFlag^fMemory?SetFilePointer(uliMethod,liOffset):SetMemoryPointer(uliMethod,liOffset);
  }
long int CFileStream::fread(void *pBuffer,long int liLength,long int *pReadden)
  {
    return m_fFlag^fMemory?ReadFromFile(pBuffer,liLength,pReadden):ReadFromMemory(pBuffer,liLength,pReadden);
  }
long int CFileStream::fclose()
  {
    long int liResult=m_fFlag^fMemory?(CloseHandle((HANDLE)m_pBuffer)?ERROR_SUCCESS:(long int)GetLastError()):((delete [] m_pBuffer),ERROR_SUCCESS);

    m_pBuffer=0;
    delete this;
    return liResult;
  }
/******************************************************************************
*                              Public -  ŒÕ≈÷                                 *
******************************************************************************/
