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
#ifndef __NZB_DL_INFO_ITEM_HEADER__
#define __NZB_DL_INFO_ITEM_HEADER__

#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <string>

/**
 * 
 * 
 */
class NzbDlInfoItem : public Gtk::Box
{
public:

	NzbDlInfoItem(const std::string& image_resource_path, const Glib::ustring& text = Glib::ustring(""));
	~NzbDlInfoItem() {}

// attributes
public:

	Gtk::Image& get_icon() { return m_icon; }
	const Gtk::Image& get_icon() const { return m_icon; }

	Gtk::Label& get_label() { return m_label; }
	const Gtk::Label& get_label() const { return m_label; }

// operations
public:

	void set_text(const Glib::ustring& text);
	Glib::ustring get_text() const;

	void set_min_width_by_text(const char *text);

// implementation
protected:

	Gtk::Image m_icon;
	Gtk::Label m_label;
};

#endif  /* __NZB_DL_INFO_ITEM_HEADER__ */