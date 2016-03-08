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
#ifndef __GNZB_TOOL_BUTTON_HEADER__
#define __GNZB_TOOL_BUTTON_HEADER__

#include <gtkmm/image.h>
#include <gtkmm/toolbutton.h>

/*
 * 
 */
class GNzbToolButton : public Gtk::ToolButton
{
// construction
public:

	GNzbToolButton(
		const Glib::ustring& tooltip,
	    const std::string& image_resource_path,
		const std::string& image_slected_resource_path,
	    bool create_insensitive_image = false,
		int minWidth = -1);
	GNzbToolButton(
		const Glib::ustring& tooltip,
	    const std::string& image_resource_path,
		const std::string& image_slected_resource_path,
		const std::string&  image_insensitive_resource_path,
		int minWidth = -1);
	~GNzbToolButton() {}

// implementation
protected:

	// signal handlers
	void on_button_press(GdkEventButton *event);
	void on_button_release(GdkEventButton *event);
	void on_state_flags_changed(Gtk::StateFlags previous_state_flags);

	void init_instance(const Glib::ustring& tooltip, int min_width = -1);

	// images
	Gtk::Image m_image, m_image_sel, m_image_insensitive;
};

#endif  /* __GNZB_TOOL_BUTTON_HEADER__ */
