#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#define ANCIEN_UGLY_STYLE

#ifndef __FILESTREAM_H__
#include "filestream.h"
#endif

#ifndef __GLOBALS_H__
#include "globals.h"
#endif

#define FLAG_PRIORITY_MEMORY            true
#define FLAG_PRIORITY_FILE              false

typedef struct tagZIPINFO
  {
    unzFile m_Zip;
    char *m_pName;
  } ZIPINFO;

class CFileStorage
  {
    private:
      VNTFS_FILE_INFO_EX *m_pFileInfoEx;
      //unzFile m_pzip;
      ZIPINFO *m_ppZips;
#ifndef ANCIEN_UGLY_STYLE
      CFileStream *m_pStream;
#endif
      char *m_pString;
      char *m_pDirectoryname;
      int m_iZips;
      int m_iCurZip;
      bool m_fFlag;
      bool IsMatchingString(const char *,const char *)const;
      size_t Unify(std::vector <VNTFS_FIND_RESULT> &);
    protected:
#ifdef _DEBUG
      void *m_hFile;
      virtual long int WriteLog(const char *);
#endif
      virtual long int FindFirstFile(VNTFS_FILE_INFO_EX *,unzFile);
      virtual long int FindNextFile(unzFile);
      virtual long int GlobalSearchFromZip(const char *);
      virtual long int FindFile(const char *);
      virtual long int EnumFromSystem(const char *,std::vector <VNTFS_FIND_RESULT> &);
      virtual long int EnumFromZip(const char *,std::vector <VNTFS_FIND_RESULT> &);
      virtual long int FindCRC(unsigned long int,char *);
    public:
      CFileStorage():
          m_pFileInfoEx(0),
          m_ppZips(0),
#ifndef ANCIEN_UGLY_STYLE
          m_pStream(0),
#endif
          m_pString(0),
          m_pDirectoryname(0),
          m_iZips(0),
          m_iCurZip(-1),
#ifdef _DEBUG
#pragma warning(disable:4312)
          m_hFile((void *)0xffffffff),
#pragma warning(default:4312)
#endif
          m_fFlag(FLAG_PRIORITY_FILE){}
      virtual ~CFileStorage();
      virtual long int Initialize(const char *);
      virtual std::vector <VNTFS_FIND_RESULT> *Enumerate(const char *,/*std::vector <VNTFS_FIND_RESULT> &,*/int fWereToSearch=fBoth,bool fUnify=true);
      virtual void DeleteEnumeration(std::vector <VNTFS_FIND_RESULT> *);
      virtual long int GetFileInfo(const char *,VNTFS_FILE_INFO_EX *);
      virtual bool isFileExist(const char *);
      // Первый параметр входной - имя проверяемого файла, второй - выходной имя найденного файла.
      virtual const char *GetUniqueName(const char *,const char *);
#ifdef ANCIEN_UGLY_STYLE
      virtual CFileStream *Open(const char *);
#else
      virtual long int Open(const char *);
#endif
#ifndef ANCIEN_UGLY_STYLE
      CFileStream *GetFileStream(){return m_pStream;}
      void KillStream(){delete m_pStream;m_pStream=0;}
#endif
      bool SetPriority(bool fFlag){return m_fFlag=fFlag;}
      bool GetPriority(){return m_fFlag;}
  };

/*#ifndef __VERY_NEW_TECHOLOGY_FILESYSTEM_H__
#include "VNTFS.h"
#endif*/

#endif  // __FILESYSTEM_H__