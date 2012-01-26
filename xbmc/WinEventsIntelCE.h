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

#ifndef WINDOW_EVENTS_OPENKODE_H
#define WINDOW_EVENTS_OPENKODE_H

#pragma once
#include <map>
#include "WinEvents.h"
#include "common/LinuxInputDevices.h"

class CWinEventsIntelCE : public CWinEventsBase
{
public:
  CWinEventsIntelCE();
  static bool MessagePump();
  static void RefreshDevices();
  static bool IsRemoteLowBattery();

private:
  static bool m_initialized;
  static CLinuxInputDevices m_devices;
  static std::map<int, int> m_altMap;
  static std::map<int, int> m_keyPadMap;
};

#endif
