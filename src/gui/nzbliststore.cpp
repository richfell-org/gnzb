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
#include "nzbliststore.h"
#include "../gnzb.h"
#include "../db/gnzbgroups.h"
#include <gtkmm/treemodelcolumn.h>

static Glib::ustring get_file_name(const std::string& path)
{
	Glib::ustring result("");
	std::string::size_type i = path.find_last_of('/');
	if(std::string::npos != i)
		result = path.substr(i + 1);
	return result;
}

/* NzbTreeModelColumnRecord ****************************************************/

NzbTreeModelColumnRecord::NzbTreeModelColumnRecord()
{
	add(m_col_nzb);
	add(m_col_name);
	add(m_col_group_color_text);
	add(m_col_action1);
	add(m_col_action2);
	add(m_col_action1_pressed);
	add(m_col_action2_pressed);
}

NzbTreeModelColumnRecord::~NzbTreeModelColumnRecord()
{
}

/* NzbListStore ****************************************************************/

NzbListStore::NzbListStore()
:   Gtk::ListStore(),
	Glib::ObjectBase("nzbliststrore")
{
	set_column_types(m_cols);
}

NzbListStore::~NzbListStore()
{
}

Gtk::TreeIter NzbListStore::insert(const Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb)
{
	Gtk::ListStore::iterator new_iter = Gtk::ListStore::insert(iter);
	set_values(new_iter, ptr_gnzb);
	return new_iter;
}

Gtk::TreeIter NzbListStore::insert_after(const Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb)
{
	Gtk::ListStore::iterator new_iter = Gtk::ListStore::insert_after(iter);
	set_values(new_iter, ptr_gnzb);
	return new_iter;
}

Gtk::TreeIter NzbListStore::prepend(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	Gtk::ListStore::iterator iter = Gtk::ListStore::prepend();
	set_values(iter, ptr_gnzb);
	return iter;
}

Gtk::TreeIter NzbListStore::append(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	Gtk::ListStore::iterator iter = Gtk::ListStore::append();
	set_values(iter, ptr_gnzb);
	return iter;
}

Gtk::TreeIter NzbListStore::find_gnzb(const GNzb *p_gnzb) const
{
	for(auto iter : children())
	{
		if(((std::shared_ptr<GNzb>)(*iter)[m_cols.gnzb()]).get() == p_gnzb)
			return iter;
	}

	return children().end();
}

void NzbListStore::set_values(Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb)
{
	(*iter)[m_cols.gnzb()] = ptr_gnzb;
	(*iter)[m_cols.name()] = get_file_name(ptr_gnzb->path());
	(*iter)[m_cols.group_color_text()] = ptr_gnzb->ptr_group()
		? Glib::ustring(ptr_gnzb->ptr_group()->getColorText())
		: Glib::ustring("rgba(128,128,128,1.0)");
	(*iter)[m_cols.action1()] = GNzbAction::START;
	(*iter)[m_cols.action2()] = GNzbAction::DELETE;
	(*iter)[m_cols.action1_pressed()] = false;
	(*iter)[m_cols.action2_pressed()] = false;
}