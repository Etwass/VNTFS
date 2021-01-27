#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#ifndef __FILESTREAM_H__
#include "filestream.h"
#endif

#ifndef __GLOBALS_H__
#include "globals.h"
#endif

#define FLAG_PRIORITY_MEMORY            true
#define FLAG_PRIORITY_FILE              false

typedef struct tagCASH
  {
    char *m_pZipname;
    int m_iEntries;
    VNTFS_FILE_INFO_EX *m_pFileInfoEx;
  } CASH;

class CFileStorage
  {
    private:
      VNTFS_FILE_INFO_EX *m_pFileInfoEx;
      unzFile m_pzip;
      char *m_pPassword;
      char *m_pString;
      char *m_pDirectoryname;
      //char **m_ppZips;
      CASH *m_pCash;                                // <<<<<<<----------------
      int m_iZips;
      int m_iCurZip;
      int m_iFile;
      bool m_fFlag;
      bool IsMatchingString(const char *,const char *)const;
      size_t Unify(std::vector <VNTFS_FIND_RESULT> &);
    protected:
#ifdef _DEBUG
      void *m_hFile;
      virtual long int WriteLog(const char *);
#endif
      virtual long int FindFileFromCash(const CASH *pCash,const char *);
      virtual long int IsFileExistInGlobalCash(const char *);
      virtual long int FindFileFromGlobalCash(const char *);
      virtual long int FindFile(const char *);
      virtual long int EnumFromSystem(const char *,std::vector <VNTFS_FIND_RESULT> &);
      virtual long int EnumFromZip(const char *,std::vector <VNTFS_FIND_RESULT> &);
      virtual long int FindCRC(unsigned long int,char *);
    public:
      CFileStorage():
          m_pFileInfoEx(0),
          m_pzip(0),
          m_pPassword(0),
          m_pString(0),
          m_pDirectoryname(0),
          m_pCash(0),
          m_iZips(0),
          m_iCurZip(-1),
          m_iFile(-1),
#ifdef _DEBUG
#pragma warning(disable:4312)
          m_hFile((void *)0xffffffff),
#pragma warning(default:4312)
#endif
          m_fFlag(FLAG_PRIORITY_FILE){}
      virtual ~CFileStorage();
      virtual long int Initialize(const char *,const char *);
      virtual std::vector <VNTFS_FIND_RESULT> *Enumerate(const char *,int fWhereToSearch=fBoth,bool fUnify=true);
      virtual void DeleteEnumeration(std::vector <VNTFS_FIND_RESULT> *);
      virtual long int GetFileInfo(const char *,VNTFS_FILE_INFO_EX *);
      virtual bool isFileExist(const char *);
      // Первый параметр входной - имя проверяемого файла, второй - выходной имя найденного файла.
      virtual const char *GetUniqueName(const char *,const char *);
      virtual CFileStream *Open(const char *);
      bool SetPriority(bool fFlag){return m_fFlag=fFlag;}
      bool GetPriority()const{return m_fFlag;}
  };

#endif  // __FILESYSTEM_H__