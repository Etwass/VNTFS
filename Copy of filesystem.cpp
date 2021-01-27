#include "filesystem.h"

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifdef _DEBUG

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#define LOG_FILENAME                            "VNTFS_LogFile.log"
#endif  // _DEBUG
#pragma warning(disable:4996)

#define PASSWORD_STR                            "{9EC157CF-A237-40e9-B736-64D2A4E7068F}"

/******************************************************************************
*                             GLOBAL - НАЧАЛО                                 *
******************************************************************************/
#ifdef _DEBUG
inline DWORD GetErrorMessage(HWND hwnd,const TCHAR *Name,DWORD Error)
  {
    LPVOID lpMsgBuf;

    FormatMessage
      (
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        Error,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
      );
    MessageBox(hwnd,(const TCHAR *)lpMsgBuf,Name,MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
    GlobalFree(lpMsgBuf);
    return Error;
  }
#endif  // _DEBUG
int compare(const void *pData1,const void *pData2)
  {
    return _stricmp(((VNTFS_FILE_INFO_EX *)pData1)->m_pFileName,((VNTFS_FILE_INFO_EX *)pData2)->m_pFileName);
  }
#pragma warning(disable:4706)
char *ConvertFilename(char *pFilename)
  {
    char *pTemp=pFilename;

    while(pTemp=strchr(pTemp,'\\'))*pTemp='/';
    return pFilename;
  }
#pragma warning(default:4706)
/******************************************************************************
*                             GLOBAL - КОНЕЦ                                  *
******************************************************************************/
/******************************************************************************
*                             Private - НАЧАЛО                                *
******************************************************************************/
// Если маска длиннее тестируемой строки, то возможен неверный результат. ИСПРАВИТЬ!!!
// P.S. Кажется исправил. Проверить!!!
bool CFileStorage::IsMatchingString(const char *pMask,const char *pString)const
  {
    char *pMaskPointer=(char *)pMask;
    char *pStringPointer=(char *)pString;

    while(*pStringPointer)
      {
        switch(*pMaskPointer)
          {
            case '\0':
              return false;
            case '*':
              if(!(*(pMaskPointer+1)^*pStringPointer))pMaskPointer+=2;
              break;
            case '?':
              pMaskPointer++;
              break;
            default:
              if(*pMaskPointer^*pStringPointer)return false;
              pMaskPointer++;
          }
        pStringPointer++;
      }
    return !*pMaskPointer||(!(*pMaskPointer^'*')&&!*(pMaskPointer+1));//true;
  }
size_t CFileStorage::Unify(std::vector <VNTFS_FIND_RESULT> &filelist)
  {
    std::vector <VNTFS_FIND_RESULT>::iterator Iter1;
    std::vector <VNTFS_FIND_RESULT>::iterator Iter2;

    for(Iter1=filelist.begin();Iter1<filelist.end();Iter1++)
      for(Iter2=Iter1+1;Iter2<filelist.end();Iter2++)
        if(!_stricmp(Iter1->m_stdvFilename.c_str(),Iter2->m_stdvFilename.c_str()))
          switch(m_fFlag)
            {
              case FLAG_PRIORITY_MEMORY:
                if(Iter2->m_iType^fMemory)
                  Iter2->m_iType=fBad;
                 else
                  Iter1->m_iType=fBad;
                continue;
              case FLAG_PRIORITY_FILE:
                if(Iter2->m_iType^fFile)
                  Iter2->m_iType=fBad;
                 else
                  Iter1->m_iType=fBad;
            };
    Iter1=filelist.begin();
    while(Iter1!=filelist.end())
      {
        if(!(Iter1->m_iType^fBad))
          {
            filelist.erase(Iter1);
            Iter1=filelist.begin();
            continue;
          }
        Iter1++;
      }
    return 0;
  }
/******************************************************************************
*                             Private - КОНЕЦ                                 *
******************************************************************************/
/******************************************************************************
*                             Protected - НАЧАЛО                              *
******************************************************************************/
#ifdef _DEBUG
long int CFileStorage::WriteLog(const char *pString)
  {
    if((HANDLE)m_hFile==INVALID_HANDLE_VALUE)return ERROR_FUNCTION_FAILED;
    DWORD dwToWrite=(DWORD)strlen(pString);
    DWORD dwWritten=0;

    try
      {
        if(!WriteFile((HANDLE)m_hFile,(const void *)pString,dwToWrite,&dwWritten,NULL))throw (long int)GetLastError();
      }
    catch(long int liExcpt)
      {
        GetErrorMessage(HWND_DESKTOP,"VNTFS.dll",GetLastError());
        return liExcpt;
      }
    return ERROR_SUCCESS;
  }
#endif
long int CFileStorage::FindFileFromCash(const CASH *pCash,const char *pFilename)
  {
    bool fContinue;
    int iComparResult;
    int iStart=0;
    int iEnd=pCash->m_iEntries-1;

    m_iFile=iEnd/2;
    fContinue=iEnd>=iStart;
    while(fContinue)
      {
        iComparResult=_stricmp(pFilename,pCash->m_pFileInfoEx[m_iFile].m_pFileName);
        if(iComparResult<0)
          {
            iEnd=m_iFile-1;
            fContinue=iEnd>iStart;
            m_iFile=(iStart+iEnd)/2;
          }
         else
          if(iComparResult>0)
            {
              iStart=m_iFile+1;
              fContinue=iEnd>=iStart;
              m_iFile=(iStart+iEnd)/2;
            }
           else
            return ERROR_SUCCESS;
      }
    return ERROR_INVALID_PARAMETER;
  }
#pragma warning(disable:4706)
long int CFileStorage::FindFileFromGlobalCash(const char *pFilename)
  {
    int iPrevZip=m_iCurZip;

    if(m_iCurZip>-1)
      if(FindFileFromCash(&m_pCash[m_iCurZip],pFilename)^ERROR_SUCCESS)
        {
          unzClose(m_pzip);
          m_pzip=0;
        }
       else
        return ERROR_SUCCESS;
    for(m_iCurZip=0;m_iCurZip<m_iZips;m_iCurZip++)
      if(m_iCurZip^iPrevZip)
        if(!(FindFileFromCash(&m_pCash[m_iCurZip],pFilename)^ERROR_SUCCESS))
          {
            if(!(m_pzip=unzOpen(m_pCash[m_iCurZip].m_pZipname)))
              {
                m_iCurZip=-1;
                return ERROR_FUNCTION_FAILED;
              }
            return ERROR_SUCCESS;
          }
    m_iCurZip=-1;
    return ERROR_FILE_NOT_FOUND;
  }
#pragma warning(default:4706)
long int CFileStorage::FindFirstFile(VNTFS_FILE_INFO_EX *pInfoEx,unzFile zip)
  {
    if(!zip)return ERROR_FILE_NOT_FOUND;
    if(unzGoToFirstFile(zip)^UNZ_OK)return ERROR_FUNCTION_FAILED;
#ifndef ANCIEN_UGLY_STYLE
    delete m_pStream;
    m_pStream=0;
#endif
    return unzGetCurrentFileInfo
      (
        zip,
        (unz_file_info *)pInfoEx->m_pInfo,
        pInfoEx->m_pFileName,
        pInfoEx->m_ulFileNameBufferSize,
        pInfoEx->m_pExtraField,
        pInfoEx->m_ulExtraFieldBufferSize,
        pInfoEx->m_pComment,
        pInfoEx->m_ulCommentBufferSize
      )^UNZ_OK?
      ((m_pFileInfoEx=0),ERROR_FUNCTION_FAILED):
      ((m_pFileInfoEx=pInfoEx),ERROR_SUCCESS);
  }
long int CFileStorage::FindNextFile(unzFile zip)
  {
    if(!zip)return ERROR_FILE_NOT_FOUND;
    if(unzGoToNextFile(zip)^UNZ_OK)return ERROR_FUNCTION_FAILED;
#ifndef ANCIEN_UGLY_STYLE
    delete m_pStream;
    m_pStream=0;
#endif
    return unzGetCurrentFileInfo
      (
        zip,
        (unz_file_info *)m_pFileInfoEx->m_pInfo,
        m_pFileInfoEx->m_pFileName,
        m_pFileInfoEx->m_ulFileNameBufferSize,
        m_pFileInfoEx->m_pExtraField,
        m_pFileInfoEx->m_ulExtraFieldBufferSize,
        m_pFileInfoEx->m_pComment,
        m_pFileInfoEx->m_ulCommentBufferSize
      )^UNZ_OK?
      ERROR_FUNCTION_FAILED:ERROR_SUCCESS;
  }
#pragma warning(disable:4706)
long int CFileStorage::GlobalSearchFromZip(const char *pFilename)
  {
    return FindFileFromGlobalCash(pFilename);
  }
#pragma warning(default:4706)
long int CFileStorage::FindFile(const char *pFilename)
  {
    // check os first
    WIN32_FIND_DATA fd;
    bool fExistInSystem=false;
    HANDLE hSearch=::FindFirstFile(pFilename,&fd);

    if(hSearch!=INVALID_HANDLE_VALUE)
      {
        fExistInSystem=true;
        FindClose(hSearch);
      }
    if(m_fFlag==FLAG_PRIORITY_MEMORY)
      {
        if(GlobalSearchFromZip(pFilename)^ERROR_SUCCESS)
          return fExistInSystem?ERROR_SUCCESS:ERROR_FILE_NOT_FOUND;
        return ERROR_FILE_NOT_FOUND;
      }
     else
      {
        if(fExistInSystem)return ERROR_SUCCESS;
        GlobalSearchFromZip(pFilename);
        return ERROR_FILE_NOT_FOUND;
      }
  }
long int CFileStorage::EnumFromSystem(const char *pMask,std::vector <VNTFS_FIND_RESULT> &filelist)
  {
    WIN32_FIND_DATA fd;
    HANDLE hSearch=::FindFirstFile(pMask,&fd);
    VNTFS_FIND_RESULT vntfs_fr;
    long int liCount=0;

    if(hSearch==INVALID_HANDLE_VALUE)return (long int)GetLastError();
    do
      {
        if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            vntfs_fr.m_stdvFilename=fd.cFileName;
            vntfs_fr.m_iType=fFile;
            filelist.push_back(vntfs_fr);
            liCount++;
          }
      }
    while(::FindNextFile(hSearch,&fd));
    FindClose(hSearch);
    return liCount?ERROR_SUCCESS:ERROR_FILE_NOT_FOUND;
  }
#pragma warning(disable:4706)
long int CFileStorage::EnumFromZip(const char *pMask,std::vector <VNTFS_FIND_RESULT> &filelist)
  {
    VNTFS_FIND_RESULT vntfs_fr;
    size_t Length=strrchr(pMask,'/')-pMask+1;
/*    long int liResult;
    VNTFS_FILE_INFO_EX fi;

    if(m_iZips)
      {
        unzFile pZip;

        fi.m_pInfo=0;
        fi.m_pFileName=new char[(fi.m_ulFileNameBufferSize=MAX_PATH)+1];
        fi.m_pComment=0;
        fi.m_pExtraField=0;
        fi.m_ulCommentBufferSize=fi.m_ulExtraFieldBufferSize=0;
        for(int i=0;i<m_iZips;i++)
          {
            if(i^m_iCurZip)
              {
                if(!(pZip=unzOpen(m_ppZips[i])))continue;
              }
             else
              pZip=m_pzip;
            liResult=FindFirstFile(&fi,pZip);
            while(!(liResult^ERROR_SUCCESS))
              {
                if(!_strnicmp(pMask,fi.m_pFileName,Length))
                  if(fi.m_pFileName[strlen(fi.m_pFileName)-1]!='/') // ignore folders
                    if(!strchr(fi.m_pFileName+Length,'/'))
                      if(IsMatchingString(pMask+Length,fi.m_pFileName+Length))
                        {
                          vntfs_fr.m_stdvFilename=fi.m_pFileName+Length;
                          vntfs_fr.m_iType=fMemory;
                          filelist.push_back(vntfs_fr);
                        }
                liResult=FindNextFile(pZip);
              }
            if(i^m_iCurZip)unzClose(pZip);
          }
        delete [] fi.m_pFileName;
        return ERROR_SUCCESS;
      }
    return ERROR_FILE_NOT_FOUND;
*/

    for(int i=0;i<m_iZips;i++)
      for(int j=0;j<m_pCash[i].m_iEntries;j++)
        if(!_strnicmp(pMask,m_pCash[i].m_pFileInfoEx[j].m_pFileName,Length))
          if(m_pCash[i].m_pFileInfoEx[j].m_pFileName[strlen(m_pCash[i].m_pFileInfoEx[j].m_pFileName)-1]!='/') // ignore folders
            if(!strchr(m_pCash[i].m_pFileInfoEx[j].m_pFileName+Length,'/'))
              if(IsMatchingString(pMask+Length,m_pCash[i].m_pFileInfoEx[j].m_pFileName+Length))
                {
                  vntfs_fr.m_stdvFilename=m_pCash[i].m_pFileInfoEx[j].m_pFileName+Length;
                  vntfs_fr.m_iType=fMemory;
                  filelist.push_back(vntfs_fr);
                }
    return ERROR_SUCCESS;
  }
long int CFileStorage::FindCRC(unsigned long int uliCRC,char *pFilename)
  {
/*    VNTFS_FILE_INFO_EX fi;
    long int liResult;
    if(m_iZips)
      {
        unzFile pZip;

        fi.m_pInfo=new VNTFS_FILE_INFO;
        fi.m_ulFileNameBufferSize=MAX_PATH;
        fi.m_pFileName=pFilename;//new char[(fi.m_ulFileNameBufferSize=MAX_PATH)+1];
        fi.m_pComment=0;
        fi.m_pExtraField=0;
        fi.m_ulCommentBufferSize=fi.m_ulExtraFieldBufferSize=0;
        for(int i=0;i<m_iZips;i++)
          {
            if(i^m_iCurZip)
              {
                if(!(pZip=unzOpen(m_ppZips[i])))continue;
              }
             else
              pZip=m_pzip;
            liResult=FindFirstFile(&fi,pZip);
            while(!(liResult^ERROR_SUCCESS))
              {
                if(fi.m_pFileName[strlen(fi.m_pFileName)-1]!='/') // ignore folders
                  //if(!strchr(fi.m_pFileName+Length,'/'))
                    {
                      if(!(uliCRC^fi.m_pInfo->crc))
                        {
                          delete fi.m_pInfo;
                          if(i^m_iCurZip)unzClose(pZip);
                          return ERROR_SUCCESS;
                        }
                    }
                liResult=FindNextFile(pZip);
              }
            if(i^m_iCurZip)unzClose(pZip);
          }
        delete fi.m_pInfo;
      }
    return ERROR_FILE_NOT_FOUND;
*/

    for(int i=0;i<m_iZips;i++)
      for(int j=0;j<m_pCash[i].m_iEntries;j++)
        if(m_pCash[i].m_pFileInfoEx[j].m_pFileName[strlen(m_pCash[i].m_pFileInfoEx[j].m_pFileName)-1]!='/') // ignore folders
          if(!(uliCRC^m_pCash[i].m_pFileInfoEx[j].m_pInfo->crc))
            {
              strcpy(pFilename,m_pCash[i].m_pFileInfoEx[j].m_pFileName);
              return ERROR_SUCCESS;
            }
    return ERROR_FILE_NOT_FOUND;
  }
#pragma warning(default:4706)
/******************************************************************************
*                             Protected - КОНЕЦ                               *
******************************************************************************/
/******************************************************************************
*                             Public - НАЧАЛО                                 *
******************************************************************************/
CFileStorage::~CFileStorage()
  {
#ifndef ANCIEN_UGLY_STYLE
      delete m_pStream;
#endif
      if(m_pzip)unzClose(m_pzip);
      delete [] m_pString;
      delete [] m_pDirectoryname;
      /*if(m_ppZips)
        {
          for(int i=0;i<m_iZips;i++)delete [] m_ppZips[i];
          delete [] m_ppZips;
        }*/
      for(int i=0;i<m_iZips;i++)
        {
          for(int j=0;j<m_pCash[i].m_iEntries;j++)
            {
              delete [] m_pCash[i].m_pFileInfoEx[j].m_pFileName;
              delete [] m_pCash[i].m_pFileInfoEx[j].m_pComment;
              delete [] m_pCash[i].m_pFileInfoEx[j].m_pExtraField;
              delete m_pCash[i].m_pFileInfoEx[j].m_pInfo;
            }
          delete [] m_pCash[i].m_pFileInfoEx;
          delete [] m_pCash[i].m_pZipname;
        }
      delete [] m_pCash;
#ifdef _DEBUG
      CloseHandle((HANDLE)m_hFile);
#endif
  }
#pragma warning(disable:4706)
long int CFileStorage::Initialize(const char *pDirectoryname)
  {
#ifdef _DEBUG
    (HANDLE)m_hFile=CreateFile(LOG_FILENAME,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(m_hFile==INVALID_HANDLE_VALUE)GetErrorMessage(HWND_DESKTOP,"VNTFS.dll",GetLastError());
#endif
    size_t Length=lstrlenA(pDirectoryname);

    delete [] m_pDirectoryname;
    m_pDirectoryname=new char[Length+2];
    lstrcpyA(m_pDirectoryname,pDirectoryname);
    if(!(*(m_pDirectoryname+Length-1)^'\\'&&*(m_pDirectoryname+Length-1)^'/'))*(m_pDirectoryname+Length-1)='\0';
    {
      HANDLE hSearch;
      WIN32_FIND_DATA fd;
      char *pMask=new char[MAX_PATH];

      sprintf(pMask,"%s%s*.pak",(*m_pDirectoryname?m_pDirectoryname:""),(*m_pDirectoryname?"/":""));
      hSearch=::FindFirstFile(pMask,&fd);
      if(hSearch==INVALID_HANDLE_VALUE)
        {
          delete [] pMask;
          return ERROR_SUCCESS;
        }
      do
        {
          if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
          m_iZips++;
        }
      while(::FindNextFile(hSearch,&fd));
      FindClose(hSearch);
      //m_ppZips=new char*[m_iZips];
      m_pCash=new CASH[m_iZips];
      //memset((void *)m_ppZips,0,m_iZips*sizeof(char *));
      hSearch=::FindFirstFile(pMask,&fd);
      if(hSearch==INVALID_HANDLE_VALUE)
        {
          delete [] pMask;
          return ERROR_FUNCTION_FAILED;
        }
      {
        int i=0;

        do
          {
            if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
            sprintf
              (
                //(m_ppZips[i]=new char[strlen(m_pDirectoryname)+strlen(fd.cFileName)+1+1]),
                (m_pCash[i].m_pZipname=new char[strlen(m_pDirectoryname)+strlen(fd.cFileName)+1+1]),
                "%s%s%s",
                (*m_pDirectoryname?m_pDirectoryname:""),
                (*m_pDirectoryname?"/":""),
                fd.cFileName
              );
            ConvertFilename(m_pCash[i].m_pZipname);
            {
              int j;
              unzFile pZip;
              unz_global_info gi;

              if(!(pZip=unzOpen(m_pCash[i].m_pZipname)))return ERROR_FUNCTION_FAILED;
              if(unzGetGlobalInfo(pZip,&gi)^UNZ_OK)return ERROR_FUNCTION_FAILED;
              if(unzGoToFirstFile(pZip)^UNZ_OK)return ERROR_FUNCTION_FAILED;
              m_pCash[i].m_pFileInfoEx=new VNTFS_FILE_INFO_EX[m_pCash[i].m_iEntries=gi.number_entry];
              for(j=0;/*j<gi.number_entry*/;j++)
                {
                  m_pCash[i].m_pFileInfoEx[j].m_pInfo=new VNTFS_FILE_INFO;
                  if(unzGetCurrentFileInfo(pZip,(unz_file_info *)m_pCash[i].m_pFileInfoEx[j].m_pInfo,0,0,0,0,0,0)^UNZ_OK)return ERROR_FUNCTION_FAILED;
                  m_pCash[i].m_pFileInfoEx[j].m_pFileName=new char[m_pCash[i].m_pFileInfoEx[j].m_ulFileNameBufferSize=m_pCash[i].m_pFileInfoEx[j].m_pInfo->size_filename+1];
                  m_pCash[i].m_pFileInfoEx[j].m_pComment=new char[m_pCash[i].m_pFileInfoEx[j].m_ulCommentBufferSize=m_pCash[i].m_pFileInfoEx[j].m_pInfo->size_file_comment+1];
                  m_pCash[i].m_pFileInfoEx[j].m_pExtraField=new char[m_pCash[i].m_pFileInfoEx[j].m_ulExtraFieldBufferSize=m_pCash[i].m_pFileInfoEx[j].m_pInfo->size_file_extra];
                  if(unzGetCurrentFileInfo(pZip,(unz_file_info *)m_pCash[i].m_pFileInfoEx[j].m_pInfo,m_pCash[i].m_pFileInfoEx[j].m_pFileName,m_pCash[i].m_pFileInfoEx[j].m_ulFileNameBufferSize,m_pCash[i].m_pFileInfoEx[j].m_pExtraField,m_pCash[i].m_pFileInfoEx[j].m_ulExtraFieldBufferSize,m_pCash[i].m_pFileInfoEx[j].m_pComment,m_pCash[i].m_pFileInfoEx[j].m_ulCommentBufferSize)^UNZ_OK)return ERROR_FUNCTION_FAILED;
                  if(unzGoToNextFile(pZip)^UNZ_OK)break;
                }
              if(j^(gi.number_entry-1))return ERROR_FUNCTION_FAILED;
              unzClose(pZip);
              qsort(m_pCash[i++].m_pFileInfoEx,gi.number_entry,sizeof(VNTFS_FILE_INFO_EX),compare);
#ifdef _DEBUG
              {
                char *pLogString=new char[10*MAX_PATH];

                sprintf(pLogString,"Found Files in archive %s\r\n\r\n",m_pCash[i-1].m_pZipname);
                WriteLog(pLogString);
                for(int k=0;k<m_pCash[i-1].m_iEntries;k++)
                  {
                    sprintf(pLogString,"%s\r\n",m_pCash[i-1].m_pFileInfoEx[k].m_pFileName);
                    WriteLog(pLogString);
                  }
                sprintf(pLogString,"%d file(s)\r\n\r\n\r\n",m_pCash[i-1].m_iEntries);
                WriteLog(pLogString);
                delete [] pLogString;
              }
#endif  // _DEBUG
            }
          }
        while(::FindNextFile(hSearch,&fd));
        FindClose(hSearch);
      }
      delete [] pMask;
    }
    return ERROR_SUCCESS;
  }
#pragma warning(default:4706)
std::vector <VNTFS_FIND_RESULT> *CFileStorage::Enumerate(const char *pMask,/*std::vector <VNTFS_FIND_RESULT> &filelist,*/int fWereToSearch,bool fUnify)
  {
    std::vector <VNTFS_FIND_RESULT> *pFilelist=new std::vector <VNTFS_FIND_RESULT>();

    delete [] m_pString;
    m_pString=new char[lstrlenA(pMask)+1];
    lstrcpyA(m_pString,pMask);
    ConvertFilename(m_pString);
    switch(fWereToSearch)
      {
        case fFile:
          EnumFromSystem(m_pString,*pFilelist);
          return pFilelist;
        case fMemory:
          EnumFromZip(m_pString,*pFilelist);
          return pFilelist;
        case fBoth:
          EnumFromSystem(m_pString,*pFilelist);
          EnumFromZip(m_pString,*pFilelist);
          if(fUnify)Unify(*pFilelist);
      }
    return pFilelist;//ERROR_INVALID_PARAMETER;
  }
void CFileStorage::DeleteEnumeration(std::vector <VNTFS_FIND_RESULT> *pList)
  {
    delete pList;
  }
int getValue(int fromBit,int toBit,WORD value)
  {
    return ((WORD)(value<<(15-toBit)))>>(15-toBit+fromBit);
  }
long int CFileStorage::GetFileInfo(const char *pFilename,VNTFS_FILE_INFO_EX *pInfoEx)
  {
#ifndef ANCIEN_UGLY_STYLE
    delete m_pStream;
    m_pStream=0;
#endif
    delete [] m_pString;
    m_pString=new char[lstrlen(pFilename)+1];
    lstrcpy(m_pString,pFilename);
    switch(FindFile(ConvertFilename(m_pString)))
      {
        case ERROR_SUCCESS:
          if(pInfoEx->m_pFileName)lstrcpyn(pInfoEx->m_pFileName,m_pString,pInfoEx->m_ulFileNameBufferSize);
          if(pInfoEx->m_pComment)lstrcpyn(pInfoEx->m_pComment,"No comments",pInfoEx->m_ulCommentBufferSize);
          if(pInfoEx->m_pExtraField)memset(pInfoEx->m_pExtraField,0,pInfoEx->m_ulExtraFieldBufferSize);
          if(pInfoEx->m_pInfo)
            {
              pInfoEx->m_pInfo->version=(uLong)-1;
              pInfoEx->m_pInfo->version_needed=(uLong)-1;
              pInfoEx->m_pInfo->flag=0;
              pInfoEx->m_pInfo->compression_method=0;
              pInfoEx->m_pInfo->crc=0;//(uLong)-1;
              pInfoEx->m_pInfo->size_filename=lstrlen(m_pString);
              pInfoEx->m_pInfo->size_file_extra=pInfoEx->m_pExtraField?pInfoEx->m_ulExtraFieldBufferSize:0;
              pInfoEx->m_pInfo->size_file_comment=pInfoEx->m_pComment?pInfoEx->m_ulCommentBufferSize:0;

              pInfoEx->m_pInfo->disk_num_start=(uLong)-1;
              pInfoEx->m_pInfo->internal_fa=pInfoEx->m_pInfo->external_fa=GetFileAttributes(m_pString);
              // time
              {
                HANDLE hFile=CreateFile(m_pString,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
                /*FILETIME ftCreation;
                FILETIME ftLastAccess;*/
                FILETIME ftLastWrite;
                unsigned __int16 lo=LOWORD(pInfoEx->m_pInfo->dosDate);
                unsigned __int16 hi=HIWORD(pInfoEx->m_pInfo->dosDate);

                if(hFile==INVALID_HANDLE_VALUE)return ERROR_FILE_NOT_FOUND;
                pInfoEx->m_pInfo->compressed_size=pInfoEx->m_pInfo->uncompressed_size=GetFileSize(hFile,NULL);
                if(!GetFileTime(hFile,NULL/*&ftCreation*/,NULL/*&ftLastAccess*/,&ftLastWrite))
                  {
                    CloseHandle(hFile);
                    return (long int)GetLastError();
                  }
                CloseHandle(hFile);
                if(!FileTimeToDosDateTime(&ftLastWrite,&lo,&hi))return (long int)GetLastError();
                pInfoEx->m_pInfo->dosDate=MAKELONG(lo,hi);
                pInfoEx->m_pInfo->tmu_date.tm_year=1980+getValue(9,15,lo);
                pInfoEx->m_pInfo->tmu_date.tm_mon=getValue(5,8,lo)-1;
                pInfoEx->m_pInfo->tmu_date.tm_mday=getValue(0,4,lo);
                pInfoEx->m_pInfo->tmu_date.tm_hour=getValue(11,15,hi);
                pInfoEx->m_pInfo->tmu_date.tm_min=getValue(5,10,hi);
                pInfoEx->m_pInfo->tmu_date.tm_sec=getValue(0,4,hi)*2;
              }
            }
          return ERROR_SUCCESS;
        default:
          /*if(!m_pzip)
            return ERROR_FILE_NOT_FOUND;
          if(unzLocateFile(m_pzip,m_pString,0)!=UNZ_OK)
            return ERROR_FILE_NOT_FOUND;
          return unzGetCurrentFileInfo
            (
              m_pzip,
              (unz_file_info *)pInfoEx->m_pInfo,
              pInfoEx->m_pFileName,
              pInfoEx->m_ulFileNameBufferSize,
              pInfoEx->m_pExtraField,
              pInfoEx->m_ulExtraFieldBufferSize,
              pInfoEx->m_pComment,
              pInfoEx->m_ulCommentBufferSize
            )!=UNZ_OK?ERROR_FUNCTION_FAILED:ERROR_SUCCESS;*/
          if(m_iCurZip>-1)
            {
              memcpy((void *)pInfoEx->m_pInfo,m_pCash[m_iCurZip].m_pFileInfoEx[m_iFile].m_pInfo,sizeof(VNTFS_FILE_INFO));
              if(pInfoEx->m_pFileName)lstrcpyn(pInfoEx->m_pFileName,m_pCash[m_iCurZip].m_pFileInfoEx[m_iFile].m_pFileName,pInfoEx->m_ulFileNameBufferSize);
              if(pInfoEx->m_pComment)lstrcpyn(pInfoEx->m_pComment,m_pCash[m_iCurZip].m_pFileInfoEx[m_iFile].m_pComment,pInfoEx->m_ulCommentBufferSize);
              if(pInfoEx->m_pExtraField)memcpy(pInfoEx->m_pExtraField,m_pCash[m_iCurZip].m_pFileInfoEx[m_iFile].m_pExtraField,pInfoEx->m_ulExtraFieldBufferSize);
              return ERROR_SUCCESS;
            }
           else return ERROR_FILE_NOT_FOUND;
      }
  }
bool CFileStorage::isFileExist(const char* filename)
  {
    VNTFS_FILE_INFO_EX fi;

    memset(&fi,0,sizeof(VNTFS_FILE_INFO_EX));
    fi.m_pInfo=new VNTFS_FILE_INFO;
    bool bExist=(GetFileInfo(filename,&fi)==ERROR_SUCCESS);

    delete fi.m_pInfo;
    return bExist;
  }
const char *CFileStorage::GetUniqueName(const char *pTest,const char *pFilename)
  {
    VNTFS_FILE_INFO_EX fi;
    char *pResult=(char *)pFilename;

    *pResult='\0';
    memset((void *)&fi,0,sizeof(fi));
    fi.m_pInfo=new VNTFS_FILE_INFO;
    if(GetFileInfo(pTest,&fi)==ERROR_SUCCESS)
      if(fi.m_pInfo->crc)
        FindCRC(fi.m_pInfo->crc,pResult);
       else strcpy(pResult,pTest);
    delete fi.m_pInfo;
    return pFilename;
  }
#ifdef ANCIEN_UGLY_STYLE
CFileStream *CFileStorage::Open(const char *pFilename)
  {
    INITDATA id;
    CFileStream *m_pStream=0;
#ifdef _DEBUG
    char *pLogString=new char[4*MAX_PATH];
#endif

#ifdef _DEBUG
    *pLogString='\0';
#endif
    delete [] m_pString;
    m_pString=new char[lstrlen(pFilename)+1];
    lstrcpy(m_pString,pFilename);
    switch(FindFile(ConvertFilename(m_pString)))
      {
        case ERROR_SUCCESS:
          id.m_pData=(void *)pFilename;
          id.m_pExtraData=this;
          m_pStream=new CFileStream(fFile);
#ifdef _DEBUG
          sprintf(pLogString+strlen(pLogString),"File System\t-> %s -> ",m_pString);
#endif
          break;
        default:
#ifdef _DEBUG
          sprintf(pLogString+strlen(pLogString),"%s\t-> %s -> ",m_pCash[m_iCurZip].m_pZipname,m_pString);
          if(!m_pzip)
            {
              strcpy(pLogString+strlen(pLogString),"zip-file not found\r\n");
              WriteLog(pLogString);
              delete [] pLogString;
              return 0;
            }
          if(unzLocateFile(m_pzip,m_pString,0)^UNZ_OK)
            {
              sprintf(pLogString+strlen(pLogString),"file %s not found in archive %s\r\n",m_pString,m_pCash[m_iCurZip].m_pZipname);
              WriteLog(pLogString);
              delete [] pLogString;
              return 0;
            }
          if(unzOpenCurrentFilePassword(m_pzip,PASSWORD_STR)^UNZ_OK)
            {
              sprintf(pLogString+strlen(pLogString),"cant open file %s in archive %s\r\n",m_pString,m_pCash[m_iCurZip].m_pZipname);
              WriteLog(pLogString);
              delete [] pLogString;
              return 0;
            }
#else   // _DEBUG
          if(!m_pzip)return 0;
          if(unzLocateFile(m_pzip,m_pString,0)^UNZ_OK)return 0;
          if(unzOpenCurrentFilePassword(m_pzip,PASSWORD_STR)^UNZ_OK)return 0;
#endif  // _DEBUG
          {
            unz_file_info fi;

#ifdef _DEBUG
            if(unzGetCurrentFileInfo(m_pzip,&fi,NULL,0,NULL,0,NULL,0)^UNZ_OK)
              {
                sprintf(pLogString+strlen(pLogString),"cant get info from file %s in archive %s\r\n",m_pString,m_pCash[m_iCurZip].m_pZipname);
                WriteLog(pLogString);
                delete [] pLogString;
                return 0;
              }
#else   // _DEBUG
            if(unzGetCurrentFileInfo(m_pzip,&fi,NULL,0,NULL,0,NULL,0)^UNZ_OK)return 0;
#endif  // _DEBUG
            id.m_pData=(void *)new char[id.m_liLength=fi.uncompressed_size];
            id.m_pExtraData=this;
            if(!id.m_pData)
              {
                unzCloseCurrentFile(m_pzip);
#ifdef _DEBUG
                strcpy(pLogString+strlen(pLogString),"allocating memory error\r\n");
                WriteLog(pLogString);
                delete [] pLogString;
#endif  // _DEBUG
                return 0;
              }
            unzReadCurrentFile(m_pzip,id.m_pData,id.m_liLength);
            unzCloseCurrentFile(m_pzip);
            m_pStream=new CFileStream(fMemory);
          }
      }
    if(m_pStream&&m_pStream->Initialize((const void *)&id)^ERROR_SUCCESS)
      {
        delete m_pStream;
#ifdef _DEBUG
        strcpy(pLogString+strlen(pLogString),"initialization file stream object error\r\n");
        WriteLog(pLogString);
        delete [] pLogString;
#endif  // _DEBUG
        return 0;
      }
#ifdef _DEBUG
    strcpy(pLogString+strlen(pLogString),"success\r\n");
    WriteLog(pLogString);
    delete [] pLogString;
#endif  // _DEBUG
    return m_pStream;
  }
#else
long int CFileStorage::Open(const char *pFilename)
  {
    INITDATA id;

    delete m_pStream;
    m_pStream=0;
    delete [] m_pString;
    m_pString=new char[lstrlen(pFilename)+1];
    lstrcpy(m_pString,pFilename);
    switch(FindFile(ConvertFilename(m_pString)))
      {
        case ERROR_SUCCESS:
          id.m_pData=(void *)pFilename;
          id.m_pExtraData=this;
          m_pStream=new CFileStream(fFile);
          return m_pStream->Initialize((const void *)&id);
        default:
          if(unzLocateFile(m_pzip,m_pString,0)^UNZ_OK)return ERROR_FILE_NOT_FOUND;
          if(unzOpenCurrentFilePassword(m_pzip)^UNZ_OK)return ERROR_CANTOPEN;
          {
            unz_file_info fi;

            if(unzGetCurrentFileInfo(m_pzip,&fi,NULL,0,NULL,0,NULL,0)^UNZ_OK)return ERROR_FUNCTION_FAILED;
            id.m_pData=(void *)new char[id.m_liLength=fi.uncompressed_size];
            id.m_pExtraData=this;
            if(!id.m_pData)
              {
                unzCloseCurrentFile(m_pzip);
                return ERROR_NOT_ENOUGH_MEMORY;
              }
            unzReadCurrentFile(m_pzip,id.m_pData,id.m_liLength);
            unzCloseCurrentFile(m_pzip);
            m_pStream=new CFileStream(fMemory);
            return m_pStream->Initialize((const void *)&id);
          }
      }
  }
#endif
/******************************************************************************
*                              Public - КОНЕЦ                                 *
******************************************************************************/
#pragma warning(default:4996)
