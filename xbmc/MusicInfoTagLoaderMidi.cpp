/*
 *      Copyright (C) 2005-2008 Team XBMC
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
#ifndef WIN32
#include <config.h>
#endif
#ifdef HAS_TIMIDITY_CODEC
#include "MusicInfoTagLoaderMidi.h"
#include "Util.h"
#include "MusicInfoTag.h"
#include "FileSystem/File.h"

using namespace XFILE;
using namespace MUSIC_INFO;


CMusicInfoTagLoaderMidi::CMusicInfoTagLoaderMidi()
  : IMusicInfoTagLoader()
{
}


CMusicInfoTagLoaderMidi::~CMusicInfoTagLoaderMidi()
{
}

// There is no reliable tag information in MIDI files. There is a 'title' field (@T), but it looks
// like everyone puts there song title, artist name, the name of the person who created the lyrics and
// greetings to their friends and family. Therefore we return the song title as file name, and the
// song artist as parent directory.
// A good intention of creating a pattern-based artist/song recognition engine failed greatly. Simple formats
// like %A-%T fail greatly with artists like A-HA and songs like "Ob-la-Di ob-la-Da.mid". So if anyone has
// a good idea which would include cases from above, I'd be happy to hear about it.
bool CMusicInfoTagLoaderMidi::Load(const CStdString & strFileName, CMusicInfoTag & tag)
{
  tag.SetURL(strFileName);

  CStdString path, title;
  CUtil::Split( strFileName, path, title);
  CUtil::RemoveExtension( title );

  tag.SetTitle( title );

  CUtil::RemoveSlashAtEnd(path );

  if ( !path.IsEmpty() )
  {
    CStdString artist = CUtil::GetFileName( path );
    if ( !artist.IsEmpty() )
      tag.SetArtist( artist );
  }

  tag.SetLoaded(true);
  return true;
}
#endif
