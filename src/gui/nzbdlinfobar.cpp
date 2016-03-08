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
#include "nzbdlinfobar.h"
#include "../uiresource.h"

NzbDlInfoBar::NzbDlInfoBar()
:   Glib::ObjectBase(""),
	Gtk::Box(),
	m_item_box(),
	m_remaining_time(ImageResourcePath("icons/infobar/remaining-time.png"), "-"),
	m_dl_speed(ImageResourcePath("icons/infobar/download-speed.png"), "-"),
	m_remaining_size(ImageResourcePath("icons/infobar/remaining-size.png"), "-"),
	m_transfered(ImageResourcePath("icons/infobar/transferred.png"), "0 GB"),
	m_conns(ImageResourcePath("icons/infobar/conns.png"), "0/0")
{
	// size the info item widgets by longest text
	m_remaining_time.set_min_width_by_text("00hr, 00min, 00sec");
	m_dl_speed.set_min_width_by_text("   00.00 MB/s");
	m_remaining_size.set_min_width_by_text("9000 GB");
	m_transfered.set_min_width_by_text("9000 GB");
	m_conns.set_min_width_by_text("99/99");

	// add info items to layout
	m_item_box.pack_start(m_remaining_time);
	m_item_box.pack_start(m_dl_speed);
	m_item_box.pack_start(m_remaining_size);
	m_item_box.pack_start(m_transfered);

	pack_start(m_item_box, false, true);

	// add connection info item
	pack_end(m_conns, false, false);
}

NzbDlInfoBar::~NzbDlInfoBar()
{
}