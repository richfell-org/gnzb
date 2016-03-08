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
#include "nntpserverliststore.h"
#include "../../../db/preferences.h"

NntpServerListStoreColumns::NntpServerListStoreColumns()
{
	add(m_col_entry_idx);
	add(m_col_enabled);
	add(m_col_name);
}

NntpServerListStoreColumns::~NntpServerListStoreColumns()
{
}

NntpServerListStore::NntpServerListStore()
{
	set_column_types(m_cols);
}

NntpServerListStore::~NntpServerListStore()
{
}

Gtk::TreeIter NntpServerListStore::add_server(int entry_idx, PrefsNntpServer& server_entry)
{
	Gtk::TreeIter iter = append();
	(*iter)[m_cols.col_entry_idx()] = entry_idx;
	(*iter)[m_cols.col_enabled()] = server_entry.isEnabled();
	(*iter)[m_cols.col_name()] = server_entry.getName();
	return iter;
}
