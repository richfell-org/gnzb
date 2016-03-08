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
#ifndef __GNZB_TOGGLE_TOOL_BUTTON_HEADER__
#define __GNZB_TOGGLE_TOOL_BUTTON_HEADER__

#include <gtkmm/image.h>
#include <gtkmm/toggletoolbutton.h>
#include <string>

class GNzbToggleToolButton : public Gtk::ToggleToolButton
{
public:

	GNzbToggleToolButton(
		const Glib::ustring& tooltip,
		const std::string& image_resource_path,
	    const std::string& image_selected_resource_path,
		int minWidth = -1);
	~GNzbToggleToolButton() {}

protected:

	void on_toggled();

	Gtk::Image m_image, m_image_sel;
};

#endif  /* __GNZB_TOGGLE_TOOL_BUTTON_HEADER__ */