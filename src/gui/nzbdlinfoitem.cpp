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
#include "nzbdlinfoitem.h"
#include <gdkmm/pixbuf.h>
#include <pangomm/font.h>

NzbDlInfoItem::NzbDlInfoItem(const std::string& image_resource_path, const Glib::ustring& text/* = Glib::ustring("")*/)
:   Glib::ObjectBase("nzbdlinfoitem"),
	Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 3),
	m_icon(Gdk::Pixbuf::create_from_resource(image_resource_path)),
	m_label(text)
{
	set_margin_left(25);

	m_label.override_font(Pango::FontDescription("NanumGothic Semi-Bold 8"));
	m_label.set_justify(Gtk::JUSTIFY_LEFT);

	pack_start(m_icon, Gtk::PACK_SHRINK);
	pack_start(m_label, Gtk::PACK_SHRINK);
	show_all_children();
	show();
}

void NzbDlInfoItem::set_text(const Glib::ustring& text)
{
	m_label.set_text(text);
}

Glib::ustring NzbDlInfoItem::get_text() const
{
	return m_label.get_text();
}

void NzbDlInfoItem::set_min_width_by_text(const char *text)
{
	int width, height;

	Glib::RefPtr<Pango::Layout> refPangoLayout = create_pango_layout(Glib::ustring(text));
	refPangoLayout->get_pixel_size(width, height);

	set_size_request(width);
}