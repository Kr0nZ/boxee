/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include "system.h"
#include "HALPowerSyscall.h"
#include "utils/log.h"

#ifdef HAS_DBUS
#include "Application.h"
#include "LocalizeStrings.h"
#include <dbus/dbus.h>
#include <stdlib.h>

CHALPowerSyscall::CHALPowerSyscall()
{
  m_CanPowerdown = true;
  m_CanSuspend   = QueryCapability("power_management.can_suspend");
  m_CanHibernate = QueryCapability("power_management.can_hibernate");
  m_CanReboot    = true;
}

bool CHALPowerSyscall::Powerdown()
{
  return doPowerCall("Shutdown");
}
bool CHALPowerSyscall::Suspend()
{
  return doPowerCall("Suspend");
}
bool CHALPowerSyscall::Hibernate()
{
  return doPowerCall("Hibernate");
}
bool CHALPowerSyscall::Reboot()
{
  return doPowerCall("Reboot");
}

bool CHALPowerSyscall::CanPowerdown()
{
  return m_CanPowerdown;
}
bool CHALPowerSyscall::CanSuspend()
{
  return m_CanSuspend;
}
bool CHALPowerSyscall::CanHibernate()
{
  return m_CanHibernate;
}
bool CHALPowerSyscall::CanReboot()
{
  return m_CanReboot;
}

bool CHALPowerSyscall::doPowerCall(const char *powerstate)
{
  DBusMessage* msg;
  DBusMessageIter args;
  DBusError error;
  dbus_error_init (&error);
  DBusConnection *connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
  dbus_int32_t int32 = 0;
  if (connection)
  {
    msg = dbus_message_new_method_call("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/computer", "org.freedesktop.Hal.Device.SystemPowerManagement", powerstate);

    if (msg && strcmp(powerstate, "Suspend") == 0)
    {
      dbus_message_iter_init_append(msg, &args);
      if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &int32))
        CLog::Log(LOGERROR, "DBus: Failed to append arguments");
    }
    if (msg == NULL)
      CLog::Log(LOGERROR, "DBus: Create PowerManagement Message failed");
    else
    {
      DBusMessage *reply;
      reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);
      if (dbus_error_is_set(&error))
      {
        CLog::Log(LOGERROR, "DBus: %s - %s", error.name, error.message);
        if (strcmp(error.name, "org.freedesktop.Hal.Device.PermissionDeniedByPolicy") == 0)
          g_application.m_guiDialogKaiToast.QueueNotification(CGUIDialogKaiToast::ICON_EXCLAMATION,g_localizeStrings.Get(257), g_localizeStrings.Get(13020), TOAST_DISPLAY_TIME, KAI_RED_COLOR , KAI_RED_COLOR);

        return false;
      }
      // Need to create a reader for the Message
      dbus_message_unref (reply);
      dbus_message_unref(msg);
      msg = NULL;
    }

    dbus_connection_unref(connection);
    connection = NULL;
    return true;
  }
  return false;
}

bool CHALPowerSyscall::QueryCapability(const char *capability)
{
  DBusMessage* msg;
  DBusMessageIter args;
  DBusError error;
  dbus_error_init (&error);
  DBusConnection *connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
  if (connection)
  {
    msg = dbus_message_new_method_call("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/computer", "org.freedesktop.Hal.Device", "GetProperty");

    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &capability))
      return false;

    if (msg == NULL)
      return false;

    DBusMessage *reply;
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);
    if (reply == NULL)
    {
      if (dbus_error_is_set(&error))
        dbus_error_free(&error);
      
      dbus_message_unref(msg);
      return false;
    }

    dbus_bool_t b = false;
    dbus_message_get_args(reply, &error, DBUS_TYPE_BOOLEAN, &b);
    
    if (dbus_error_is_set(&error))
      dbus_error_free(&error);

    dbus_message_unref(reply);
    dbus_message_unref(msg);
    msg = NULL;
    return b;
  }

  if (dbus_error_is_set(&error))
    dbus_error_free(&error);
  return false;
}

bool CHALPowerSyscall::IsSuspended()
{
  return false;
}
#endif
