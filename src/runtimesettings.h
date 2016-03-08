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
#ifndef __RUNTIME_SETTINGS_HEADER__
#define __RUNTIME_SETTINGS_HEADER__

#include <string>

// forward references
//class Notifier;
class AppPreferences;

namespace RuntimeSettings {

class Locations;
class Notifications;
class Scripting;

Locations& locations();
Notifications& notifications();
Scripting& scripting();

/**
 * settings for the Locations preferences
 */
class Locations
{
// destruction
public:

	virtual ~Locations() {}

// attrubutes
public:

	bool always_ask_for_dl_location() const		{ return m_always_ask; }
	bool move_completed() const					{ return m_move_completed; }
	const std::string& base_output_path() const { return m_base_path; }
	const std::string& moveto_path() const		{ return m_moveto_path; }

// operations
public:

	Locations& load(const AppPreferences& prefs);

// implementation
protected:

	Locations();

	bool m_always_ask{false}, m_move_completed{false};
	std::string m_base_path, m_moveto_path;
};

/**
 * settings for the Notifications preferences
 */
class Notifications
{
// destruction
public:

	virtual ~Notifications() {}

// attrubutes
public:

	bool use_desktop_notifications() const				{ return mb_desktop_notification; }
	bool sound_on_gnzb_finished() const					{ return mb_sound_nzb_finished; }
	bool sound_on_queue_finished() const				{ return mb_sound_queue_finished; }
	const std::string& gnzb_finished_sound() const		{ return m_sound_nzbfinished; }
	const std::string& queue_finished_sound() const		{ return m_sound_queuefinished; }

// operations
public:

	Notifications& load(const AppPreferences& prefs);

// implementation
protected:

	Notifications();

	bool mb_desktop_notification{false}, mb_sound_nzb_finished{false}, mb_sound_queue_finished{false};
	std::string m_sound_nzbfinished, m_sound_queuefinished;
};

/**
 * settings for the Scripting preferences
 */
class Scripting
{
// destruction
public:

	virtual ~Scripting() {}

// attrubutes
public:

	bool scripting_enabled() const				{ return mb_enabled; }
	const std::string& python_module() const	{ return m_python_module; }

	bool on_nzb_added() const		{ return mb_nzb_added; }
	bool on_nzb_finished() const	{ return mb_nzb_finished; }
	bool on_nzb_cancelled() const   { return mb_nzb_cancelled; }

	const std::string& script_on_nzb_added() const		{ return m_script_added; }
	const std::string& script_on_nzb_finished() const   { return m_script_finished; }
	const std::string& script_on_nzb_cancelled() const  { return m_script_cancelled; }

// operations
public:

	Scripting& load(const AppPreferences& prefs);

// implementation
protected:

	Scripting();

	bool mb_enabled{false}, mb_nzb_added{false}, mb_nzb_finished{false}, mb_nzb_cancelled{false};
	std::string m_python_module, m_script_added, m_script_finished, m_script_cancelled;
};

}	   // namespace RuntimeSettings

#endif  /* __RUNTIME_SETTINGS_HEADER__ */