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
#ifndef __PREFERENCES_PAGE_HEADER__
#define __PREFERENCES_PAGE_HEADER__

#include <glibmm/refptr.h>

namespace Gtk {
	class Builder;
}

class AppPreferences;

/**
 * Interface for Gtk::Notebook pages within the
 * preferences dialog.
 * 
 */
class PreferencesPage
{
public:

	PreferencesPage() {}
	virtual ~PreferencesPage() {}

	virtual void init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& refBuilder) = 0;
	virtual void save(AppPreferences& app_prefs) = 0;

	virtual bool is_modified() const;

protected:

	void set_modified(bool modified = true) { m_modified = modified; }

	bool m_modified{false};
};

#endif  /* __PREFERENCES_PAGE_HEADER__ */