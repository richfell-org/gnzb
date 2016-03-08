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
#include "groupsliststore.h"

GroupsListStoreColumns::GroupsListStoreColumns()
{
	add(m_col_entry_idx);
	add(m_col_color);
	add(m_col_name);
}

GroupsListStore::GroupsListStore()
{
	set_column_types(m_cols);
}

Gtk::TreeIter GroupsListStore::add_group(int entry_idx, GNzbGroup& group)
{
	Gtk::TreeIter iter = append();
	(*iter)[m_cols.col_entry_idx()] = entry_idx;
	(*iter)[m_cols.col_color()] = Gdk::RGBA(group.getColorText());
	(*iter)[m_cols.col_name()] = group.getName();
	return iter;
}