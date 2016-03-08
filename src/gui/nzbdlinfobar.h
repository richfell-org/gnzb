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
#ifndef __NZB_DL_INFO_BAR_HEADER__
#define __NZB_DL_INFO_BAR_HEADER__

#include <gtkmm/box.h>
#include "nzbdlinfoitem.h"

/**
 * 
 * 
 */
class NzbDlInfoBar : public Gtk::Box
{
// construction/destruction
public:

	NzbDlInfoBar();
	~NzbDlInfoBar();

// attributes
public:

	NzbDlInfoItem& remaining_time() { return m_remaining_time; }
	const NzbDlInfoItem& remaining_time() const { return m_remaining_time; }

	NzbDlInfoItem& dl_speed() { return m_dl_speed; }
	const NzbDlInfoItem& dl_speed() const { return m_dl_speed; }

	NzbDlInfoItem& remaining_size() { return m_remaining_size; }
	const NzbDlInfoItem& remaining_size() const { return m_remaining_size; }

	NzbDlInfoItem& transferred() { return m_transfered; }
	const NzbDlInfoItem& transferred() const { return m_transfered; }

	NzbDlInfoItem& connections() { return m_conns; }
	const NzbDlInfoItem& connections() const { return m_conns; }

// implementation
private:

	Gtk::Box m_item_box;
	NzbDlInfoItem m_remaining_time, m_dl_speed, m_remaining_size, m_transfered, m_conns;
};

#endif  /* __NZB_DL_INFO_BAR_HEADER__ */