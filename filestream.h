#ifndef __FILESTREAM_H__
#define __FILESTREAM_H__

#include "VNTFS.h"

class CFileStream
  {
    private:
      unsigned long int m_fFlag;
      long int m_liSize;
      void *m_pBuffer;
      long int m_liOffset;
      void *m_pExtraData;
    protected:
      virtual long int SetMemoryPointer(unsigned long int uliMethod,long int liOffset);
      virtual long int SetFilePointer(unsigned long int uliMethod,long int liOffset);
      virtual long int ReadFromMemory(void *pBuffer,long int liLength,long int *pReadden);
      virtual long int ReadFromFile(void *pBuffer,long int liLength,long int *pReadden);
    public:
      CFileStream(unsigned long int fFlag):m_fFlag(fFlag),m_liSize(0),m_pBuffer(0),m_liOffset(0){}
      virtual ~CFileStream();
      virtual long int Initialize(const void *);
      virtual long int fseek(unsigned long int uliMethod,long int liOffset);
      virtual long int fread(void *pBuffer,long int liLength,long int *pReadden);
      virtual long int fclose();
  };

#endif  // __FILESTREAM_H__