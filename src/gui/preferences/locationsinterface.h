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
#ifndef __PREFS_LOCATIONS_INTERFACE_HEADER__
#define __PREFS_LOCATIONS_INTERFACE_HEADER__

#include "preferencespage.h"
#include "../../db/preferences.h"

namespace Glib {
	template <typename T> class RefPtr;
}

namespace Gtk {
	class Builder;
	class ToggleButton;
	class ComboBox;
}

/*
 * 
 */
class PrefsLocationsInterface : public PreferencesPage
{
// construction
public:

	PrefsLocationsInterface();
	~PrefsLocationsInterface();

// attributes
public:

	// access to the download location widget...prior to a call to init these will be null references
	Gtk::ComboBox& getLocationWidget() { return *mpComboLocation; }
	const Gtk::ComboBox& getLocationWidget() const { return *mpComboLocation; }

	// access to the always-ask-for download location widget...prior to a call to init these will be null references
	Gtk::ToggleButton& getDoAlwaysAskForLocationWidget() { return *mpToggleAlwaysAsk; }
	const Gtk::ToggleButton& getDoAlwaysAskForLocationWidget() const { return *mpToggleAlwaysAsk; }

	// access to the move downloaded widget...prior to a call to init these will be null references
	Gtk::ToggleButton& getDoMoveDownloadedWidget() { return *mpToggleMoveDownloaded; }
	const Gtk::ToggleButton& getDoMoveDownloadedWidget() const { return *mpToggleMoveDownloaded; }

	// access to the move-downloaded-to location widget...prior to a call to init these will be null references
	Gtk::ComboBox& getMoveToLocationWidget() { return *mpComboMoveToLocation; }
	const Gtk::ComboBox& getMoveToLocationWidget() const { return *mpComboMoveToLocation; }

// operations
public:

	void init(const AppPreferences& appPrefs, Glib::RefPtr<Gtk::Builder>& refBuilder);
	void save(AppPreferences& app_prefs);

	std::string getDownloadDir() const;
	void setDownloadDir(const std::string& dir);

	bool getDoAlwaysAskForDownloadLocation() const;
	void setDoAlwaysAskForDownloadLocation(bool bDoAlwaysAsk = true);

	bool getDoMoveDownloaded() const;
	void setDoMoveDownloaded(bool bDoMoveDownloaded = true);

	std::string getMoveToDir() const;
	void setMoveToDir(const std::string& dir);

// implementation
private:

	void onDlLocationChanged(Gtk::ComboBox *pComboBox);

	// Widget pointers
	Gtk::ComboBox *mpComboLocation;
	Gtk::ToggleButton *mpToggleAlwaysAsk;
	Gtk::ToggleButton *mpToggleMoveDownloaded;
	Gtk::ComboBox *mpComboMoveToLocation;
};

#endif  /* __PREFS_LOCATIONS_INTERFACE_HEADER__ */