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
#ifndef __PREFS_NOTIFICATIONS_INTERFACE_HEADER__
#define __PREFS_NOTIFICATIONS_INTERFACE_HEADER__

#include <string>
#include "preferencespage.h"

class AppPreferences;
class FileListStore;

namespace Glib {
	template <typename T> class RefPtr;
}

namespace Gtk {
	class Builder;
	class Button;
	class ToggleButton;
	class ComboBox;
}

/*
 * 
 */
class PrefsNotificationsInterface : public PreferencesPage
{
// construction
public:

	PrefsNotificationsInterface();
	~PrefsNotificationsInterface();

// attributes
public:

	Gtk::ToggleButton& getDoShowNotificationsWidget() { return *mpToggleShowNotifications; }
	const Gtk::ToggleButton& getDoShowNotificationsWidget() const { return *mpToggleShowNotifications; }

	Gtk::ToggleButton& getDoSoundOnNzbFinishedWidget() { return *mpToggleSoundOnNzbFinished; }
	const Gtk::ToggleButton& getDoSoundOnNzbFinishedWidget() const { return *mpToggleSoundOnNzbFinished; }

	Gtk::ComboBox& getSoundOnNzbFinishedWidget() { return *mpComboNzbFinishedSound; }
	const Gtk::ComboBox& getSoundOnNzbFinishedWidget() const { return *mpComboNzbFinishedSound; }

	Gtk::Button& getPreviewNzbFinishedSoundWidget() { return *mpBtnPlayNzbFinishedSound; }
	const Gtk::Button& getPreviewNzbFinishedSoundWidget() const { return *mpBtnPlayNzbFinishedSound; }

	Gtk::ToggleButton& getDoSoundOnQueueFinishedWidget() { return *mpToggleSoundOnQueueFinished; }
	const Gtk::ToggleButton& getDoSoundOnQueueFinishedWidget() const { return *mpToggleSoundOnQueueFinished; }

	Gtk::ComboBox& getSoundOnQueueFinishedWidget() { return *mpComboQueueFinishedSound; }
	const Gtk::ComboBox& getSoundOnQueueFinishedWidget() const { return *mpComboQueueFinishedSound; }

	Gtk::Button& getPreviewQueueFinishedSoundWidget() { return *mpBtnPlayQueueFinishedSound; }
	const Gtk::Button& getPreviewQueueFinishedSoundWidget() const { return *mpBtnPlayQueueFinishedSound; }

// operations
public:

	void init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder);
	void save(AppPreferences& app_prefs);

	bool getDoShowNotifications() const;
	void setDoShowNotifications(bool bDoNotifications = true);

	bool getDoSoundOnNzbFinished() const;
	void setDoSoundOnNzbFinished(bool bDoSound = true);

	std::string getSoundOnNzbFinished() const;
	void setSoundOnNzbFinished(const std::string& filename);

	bool getDoSoundOnQueueFinished() const;
	void setDoSoundOnQueueFinished(bool bDoSound = true);

	std::string getSoundOnQueueFinished() const;
	void setSoundOnQueueFinished(const std::string& filename);

// implementation
private:

	void on_file_selection_changed(Gtk::ComboBox *p_combo);

	Gtk::ToggleButton *mpToggleShowNotifications;
	Gtk::ToggleButton *mpToggleSoundOnNzbFinished;
	Gtk::ComboBox *mpComboNzbFinishedSound;
	Gtk::Button *mpBtnPlayNzbFinishedSound;
	Gtk::ToggleButton *mpToggleSoundOnQueueFinished;
	Gtk::ComboBox *mpComboQueueFinishedSound;
	Gtk::Button *mpBtnPlayQueueFinishedSound;
};

#endif  /* __PREFS_NOTIFICATIONS_INTERFACE_HEADER__ */