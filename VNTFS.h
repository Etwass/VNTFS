#ifndef __VERY_NEW_TECHOLOGY_FILESYSTEM_H__
#define __VERY_NEW_TECHOLOGY_FILESYSTEM_H__

typedef unsigned long int uLong;
typedef unsigned int uInt;

enum {fBad=-1,fMemory,fFileSystem,fBoth};

typedef struct tagINITDATA
  {
    void *m_pExtraData;
    void *m_pData;
    long int m_liLength;
  } INITDATA;

/* VNTFS_DATE_TIME_INFO contain date/time info */
typedef struct tagVNTFS_DATE_TIME_INFO
  {
    uInt tm_sec;                                /* seconds after the minute - [0,59] */
    uInt tm_min;                                /* minutes after the hour - [0,59] */
    uInt tm_hour;                               /* hours since midnight - [0,23] */
    uInt tm_mday;                               /* day of the month - [1,31] */
    uInt tm_mon;                                /* months since January - [0,11] */
    uInt tm_year;                               /* years - [1980..2044] */
  } VNTFS_DATE_TIME_INFO;

// ��������� ������������ ������ � �������� ��������� ���������.
// �� ��������� ����� �� �� �� ���� �������������.
typedef struct tagVNTFS_FILE_INFO
  {
    uLong version;                              /* version made by                 2 bytes */
    uLong version_needed;                       /* version needed to extract       2 bytes */
    uLong flag;                                 /* general purpose bit flag        2 bytes */
    uLong compression_method;                   /* compression method              2 bytes */
    uLong dosDate;                              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                                  /* crc-32                          4 bytes */
    uLong compressed_size;                      /* compressed size                 4 bytes */
    uLong uncompressed_size;                    /* uncompressed size               4 bytes */
    uLong size_filename;                        /* filename length                 2 bytes */
    uLong size_file_extra;                      /* extra field length              2 bytes */
    uLong size_file_comment;                    /* file comment length             2 bytes */

    uLong disk_num_start;                       /* disk number start               2 bytes */
    uLong internal_fa;                          /* internal file attributes        2 bytes */
    uLong external_fa;                          /* external file attributes        4 bytes */

    VNTFS_DATE_TIME_INFO tmu_date;
  } VNTFS_FILE_INFO;

// ��������� ������������ ������ � �������� ��������� ���������.
// ������������� ��������� ������ �� ������ ��������� ������ ����������
// � ���������� ��������������� ���������� ������� ���������� ������.
//
// P.S. ������ ��������� �������� ������ � ������, ���� ��������� ��������
//      ��������������� ����������. ����� ��������� �������� ���������.
typedef struct tagVNTFS_FILE_INFO_EX
  {
    VNTFS_FILE_INFO *m_pInfo;                   // ���� ��������� ���������, ��������� ����� ��������� ��������� ���������� � �����.
    char *m_pFileName;                          // ���� ��������� ���������, ����� �������� ��� ����� ������ ������.
    uLong m_ulFileNameBufferSize;               // ������ ������ ���������� ��� m_pFileName � ������.
    void *m_pExtraField;                        // ���� ��������� ���������, ����� �������� �������������� ����������.
    uLong m_ulExtraFieldBufferSize;             // ������ ������ ���������� ��� m_pExtraField � ������.
    char *m_pComment;                           // ���� ��������� �� �������, ����� ������� ����������� � �����.
    uLong m_ulCommentBufferSize;                // ������ ������ ���������� ��� m_pComment � ������.
  } VNTFS_FILE_INFO_EX;

#include <vector>
#include <string>

typedef struct tagVNTFS_FIND_RESULT
  {
    std::string m_stdvFilename;                 // ��� ���������� �����.
    int m_iType;                                // ���: fFileSystem - ���� ��������� � �������� �������,
                                                //      fMemory     - ���� ��������� � ������.
  } VNTFS_FIND_RESULT;

class CFileStorage;

typedef CFileStorage * (*GET_FILESTORAGE_OBJECT)();
typedef void (*DESTROY_FILESTORAGE_OBJECT)(/*CFileStorage ** */);

#define DLL_EXPORTS
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" DLL_API CFileStorage *vntfsGetFileStorageObject();
extern "C" DLL_API void vntfsDestroyFileStorageObject();

#endif  // __VERY_NEW_TECHOLOGY_FILESYSTEM_H__