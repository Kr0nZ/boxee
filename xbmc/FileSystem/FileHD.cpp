/*
 * XBMC Media Center
 * Copyright (c) 2002 Frodo
 * Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "system.h"
#include "FileHD.h"
#include "Util.h"
#include "URL.h"
#include "GUISettings.h"
#ifdef _LINUX
#include "XHandle.h"
#endif

#include <sys/stat.h>
#ifdef _LINUX
#include <sys/ioctl.h>
#else
#include "FCNTL.H"
#include <io.h>
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#endif

using namespace XFILE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//*********************************************************************************************
CFileHD::CFileHD()
    : m_hFile(INVALID_HANDLE_VALUE)
{}

//*********************************************************************************************
CFileHD::~CFileHD()
{
  if (m_hFile != INVALID_HANDLE_VALUE) Close();
}
//*********************************************************************************************
CStdString CFileHD::GetLocal(const CURI &url)
{
  CStdString path( url.GetFileName() );

  if( url.GetProtocol().Equals("file", false) )
  {
    // file://drive[:]/path
    // file:///drive:/path
    CStdString host( url.GetHostName() );

    if(host.size() > 0)
    {
      if(host.Right(1) == ":")
        path = host + "/" + path;
      else
        path = host + ":/" + path;
    }
  }

#ifndef _LINUX
  path.Replace('/', '\\');
#endif

  return path;
}

//*********************************************************************************************
bool CFileHD::Open(const CURI& url)
{
  CStdString strFile = GetLocal(url);

#ifdef _WIN32
  CStdStringW strWFile;
  g_charsetConverter.utf8ToW(strFile, strWFile, false);
  m_hFile.attach(CreateFileW(strWFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
#else
  m_hFile.attach(CreateFile(strFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
#endif
  if (!m_hFile.isValid()) return false;

  m_i64FilePos = 0;
  Seek(0, SEEK_SET);

  return true;
}

bool CFileHD::Exists(const CURI& url)
{
  CStdString strFile = GetLocal(url);
#ifdef _WIN32
  struct __stat64 buffer;
  CStdStringW strWFile;
  g_charsetConverter.utf8ToW(strFile, strWFile, false);
  return (_wstat64(strWFile.c_str(), &buffer)==0);
#else
  return (access(strFile.c_str(), F_OK)!=-1);
#endif
}

int CFileHD::Stat(struct __stat64* buffer)
{
  int fd;
#ifdef _LINUX
  fd = (*m_hFile).fd;
#else
  fd = _open_osfhandle((intptr_t)((HANDLE)m_hFile), 0);
  if (fd == -1)
  {
    CLog::Log(LOGERROR, "Stat: fd == -1");
    return -1;
  }
#endif
  return _fstat64(fd, buffer);
}

int CFileHD::Stat(const CURI& url, struct __stat64* buffer)
{
  CStdString strFile = GetLocal(url);
  CUtil::RemoveSlashAtEnd(strFile);

#ifdef _WIN32
  CStdStringW strWFile;
  // win32 can only stat root drives with a slash at the end
  if(strFile.length() == 2 && strFile[1] ==':')
    CUtil::AddSlashAtEnd(strFile);
  g_charsetConverter.utf8ToW(strFile, strWFile, false);
  return _wstat64(strWFile.c_str(), buffer);
#else
  return _stat64(strFile.c_str(), buffer);
#endif
}


//*********************************************************************************************
bool CFileHD::OpenForWrite(const CURI& url, bool bOverWrite)
{
  // make sure it's a legal FATX filename (we are writing to the harddisk)
  CStdString strPath = GetLocal(url);

#ifdef HAS_FTP_SERVER
  if (g_guiSettings.GetBool("servers.ftpautofatx")) // allow overriding
  {
    CStdString strPathOriginal = strPath;
    CUtil::GetFatXQualifiedPath(strPath);
    if (strPathOriginal != strPath)
      CLog::Log(LOGINFO,"CFileHD::OpenForWrite: WARNING: Truncated filename %s %s", strPathOriginal.c_str(), strPath.c_str());
  }
#endif

#ifdef _WIN32
  CStdStringW strWPath;
  g_charsetConverter.utf8ToW(strPath, strWPath, false);
  m_hFile.attach(CreateFileW(strWPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
#else
  m_hFile.attach(CreateFile(strPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
#endif
  if (!m_hFile.isValid()) 
    return false;

  m_i64FilePos = 0;
  Seek(0, SEEK_SET);

  return true;
}

//*********************************************************************************************
unsigned int CFileHD::Read(void *lpBuf, int64_t uiBufSize)
{
  if (!m_hFile.isValid()) return 0;
  DWORD nBytesRead;
  if ( ReadFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &nBytesRead, NULL) )
  {
    m_i64FilePos += nBytesRead;
    return nBytesRead;
  }
  return 0;
}

//*********************************************************************************************
int CFileHD::Write(const void *lpBuf, int64_t uiBufSize)
{
  if (!m_hFile.isValid())
    return 0;
  
  DWORD nBytesWriten;
  if ( WriteFile((HANDLE)m_hFile, (void*) lpBuf, (DWORD)uiBufSize, &nBytesWriten, NULL) )
    return nBytesWriten;
  
  return 0;
}

//*********************************************************************************************
void CFileHD::Close()
{
  m_hFile.reset();
}

//*********************************************************************************************
int64_t CFileHD::Seek(int64_t iFilePosition, int iWhence)
{
  LARGE_INTEGER lPos, lNewPos;
  lPos.QuadPart = iFilePosition;
  int bSuccess;

  int64_t length = GetLength();

  switch (iWhence)
  {
  case SEEK_SET:
    if (iFilePosition <= length || length == 0)
      bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_BEGIN);
    else
      bSuccess = false;
    break;

  case SEEK_CUR:
    if ((GetPosition()+iFilePosition) <= length || length == 0)
      bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_CURRENT);
    else
      bSuccess = false;
    break;

  case SEEK_END:
    bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_END);
    break;

  default:
    return -1;
  }
  if (bSuccess)
  {
    m_i64FilePos = lNewPos.QuadPart;
    return m_i64FilePos;
  }
  else
    return -1;
}

//*********************************************************************************************
int64_t CFileHD::GetLength()
{
  LARGE_INTEGER i64Size;
  GetFileSizeEx((HANDLE)m_hFile, &i64Size);
  return i64Size.QuadPart;
}

//*********************************************************************************************
int64_t CFileHD::GetPosition()
{
  return m_i64FilePos;
}

bool CFileHD::Delete(const CURI& url)
{
  CStdString strFile=GetLocal(url);

#ifdef _WIN32
  CStdStringW strWFile;
  g_charsetConverter.utf8ToW(strFile, strWFile, false);
  return ::DeleteFileW(strWFile.c_str()) ? true : false;
#else
  return ::DeleteFile(strFile.c_str()) ? true : false;
#endif
}

bool CFileHD::Rename(const CURI& url, const CURI& urlnew)
{
  CStdString strFile=GetLocal(url);
  CStdString strNewFile=GetLocal(urlnew);

#ifdef _WIN32
  CStdStringW strWFile;
  CStdStringW strWNewFile;
  g_charsetConverter.utf8ToW(strFile, strWFile, false);
  g_charsetConverter.utf8ToW(strNewFile, strWNewFile, false);
  return ::MoveFileW(strWFile.c_str(), strWNewFile.c_str()) ? true : false;
#else
  return ::MoveFile(strFile.c_str(), strNewFile.c_str()) ? true : false;
#endif
}

void CFileHD::Flush()
{
  ::FlushFileBuffers(m_hFile);
}

int CFileHD::IoControl(int request, void* param)
{ 
#ifdef _LINUX
  return ioctl((*m_hFile).fd, request, param);
#endif
  return -1;
}
