/*
	gnzb Usenet downloads using NZB index files

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#include <memory>
#include "runtimesettings.h"
#include "db/preferences.h"
#include "db/gnzbdb.h"

namespace RuntimeSettings {

Locations::Locations()
{
}

Locations& Locations::load(const AppPreferences& prefs)
{
	m_always_ask = prefs.getAlwayAskForDir();
	m_move_completed = prefs.getDoMoveToDir();

	// base download path
	m_base_path = prefs.getDownloadDir();
	if(!m_base_path.empty() && ('/' != m_base_path.back()))
		m_base_path.push_back('/');

	// move completed downloads path
	m_moveto_path.clear();
	if(m_move_completed)
	{
		m_moveto_path = prefs.getMoveToDir();
		if(!m_moveto_path.empty() && ('/' != m_moveto_path.back()))
			m_moveto_path.push_back('/');
	}

	return *this;
}

class LocationsImpl : public Locations
{
public:

	LocationsImpl() : Locations() {}
	~LocationsImpl() {}
} locations_impl;

Locations& locations() { return locations_impl; }

Notifications::Notifications()
{
}

Notifications& Notifications::load(const AppPreferences& prefs)
{
	mb_desktop_notification = prefs.getShowNotifications();
	mb_sound_nzb_finished = prefs.getSoundOnNzbFinish();
	mb_sound_queue_finished = prefs.getSoundOnQueueFinish();

	// sound file on GNzb finished
	m_sound_nzbfinished.clear();
	if(mb_sound_nzb_finished)
		m_sound_nzbfinished = prefs.getNzbFinishSound();

	// sound file on GNzb queue finished
	m_sound_queuefinished.clear();
	if(mb_sound_queue_finished)
		m_sound_queuefinished = prefs.getQueueFinishSound();

	return *this;
}

class NotificationsImpl : public Notifications
{
public:

	NotificationsImpl() {}
	~NotificationsImpl() {}
} notifications_impl;

Notifications& notifications() { return notifications_impl; }

Scripting::Scripting()
{
}

Scripting& Scripting::load(const AppPreferences& prefs)
{
	mb_enabled = prefs.getScriptingEnabled();
	mb_nzb_added = prefs.getRunScriptOnNzbAdded();
	mb_nzb_finished = prefs.getRunScriptOnNzbFinished();
	mb_nzb_cancelled = prefs.getRunScriptOnNzbCancelled();
	m_python_module = prefs.getSysAttrString(SA_PYMODULE);
	m_script_added.clear();
	m_script_finished.clear();
	m_script_cancelled.clear();
	if(prefs.getRunScriptOnNzbAdded())
		m_script_added = prefs.getNzbAddedScript();
	if(prefs.getRunScriptOnNzbFinished())
		m_script_finished = prefs.getNzbFinishedScript();
	if(prefs.getRunScriptOnNzbCancelled())
		m_script_cancelled = prefs.getNzbCancelledScript();

	return *this;
}

class ScriptingImpl : public Scripting
{
public:

	ScriptingImpl() {}
	~ScriptingImpl() {}
} scripting_impl;

Scripting& scripting() { return scripting_impl; }

}   // namespace EnzybeeSettings