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
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#ifndef __PREFERENCES_INTERFACE_HEADER__
#define __PREFERENCES_INTERFACE_HEADER__

#include <string>
#include <vector>
#include <bitset>
#include <sigc++/sigc++.h>
#include <glibmm/refptr.h>
#include "../../db/preferences.h"
#include "nntpserverinterface.h"
#include "locationsinterface.h"
#include "notificationsinterface.h"
#include "scriptsinterface.h"

namespace Glib {
	class ustring;
}

namespace Gtk {
	class Dialog;
	class Window;
}

/*
 *
 */
class PreferencesInterface : public sigc::trackable
{
// construction
public:

	PreferencesInterface();
	~PreferencesInterface();

// attributes
public:

	enum PreferenceCategory { PREF_SERVERS, PREF_LOCATION, PREF_NOTIFICATIONS, PREF_SCRIPTS, PREF_CATEGORY_COUNT, };
	typedef std::bitset<PREF_CATEGORY_COUNT> CategoryBitset;

// operations
public:

	CategoryBitset run(Gtk::Window *pParentWin = nullptr);

protected:

	void on_save_preferences(Gtk::Dialog *pPrefDialog);
	void on_cancel_preferences(Gtk::Dialog *pPrefDialog);

// implementation
private:

	// the preferences database interface
	AppPreferences m_app_prefs;

	// bitset to track which preferences setion has been modified
	CategoryBitset mDirtyCategories;

	// dialog box pinter
	Gtk::Dialog *mpPrefsDialog;

	// the tabs within the preferences dialog
	NntpServerInterface m_servers_tab;
	PrefsLocationsInterface mLocationsTab;
	PrefsNotificationsInterface mNotificationsTab;
	PrefsScriptsInterface mScriptsTab;
};

#endif  /* __PREFERENCES_INTERFACE_HEADER__ */