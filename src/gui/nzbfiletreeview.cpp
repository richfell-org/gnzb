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
#include "nzbfiletreeview.h"
#include "nzbfileliststore.h"
#include <gtkmm/builder.h>

NzbFileTreeView::NzbFileTreeView()
:   Gtk::TreeView(),
	Glib::ObjectBase("nzbfiletreeview"),
	m_column("NZB File"),
	m_nzbfile_renderer()
{
	construct();
}

NzbFileTreeView::NzbFileTreeView(GtkTreeView *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder)
:   Gtk::TreeView(p_object),
	Glib::ObjectBase("nzbfiletreeview"),
	m_column("NZB File"),
	m_nzbfile_renderer()
{
	construct();
}

void NzbFileTreeView::construct()
{
	// configure the view
	set_model(Glib::RefPtr<Gtk::TreeModel>(new NzbFileListStore()));
	set_headers_visible(false);
	m_column.property_sizing() = Gtk::TREE_VIEW_COLUMN_FIXED ;

	// configure renderers
	m_icon_renderer.property_xalign() = 0.0;
	m_nzbfile_renderer.property_xpad() = 3;

	get_selection()->property_mode() = Gtk::SELECTION_NONE;

	init_columns();
}

NzbFileTreeView::~NzbFileTreeView()
{
}

Glib::RefPtr<NzbFileListStore> NzbFileTreeView::get_nzbfile_model()
{
	return Glib::RefPtr<NzbFileListStore>::cast_dynamic(get_model());
}

Glib::RefPtr<const NzbFileListStore> NzbFileTreeView::get_nzbfile_model() const
{
	return Glib::RefPtr<const NzbFileListStore>::cast_dynamic(get_model());
}

void NzbFileTreeView::init_columns()
{
	remove_all_columns();

	Glib::RefPtr<NzbFileListStore> model = get_nzbfile_model();

	//m_column.clear();
	m_column.pack_start(m_icon_renderer, false);
	m_column.pack_start(m_nzbfile_renderer);

	//m_column.add_attribute(m_icon_renderer, "pixbuf", model->columns().col_icon());
	m_column.add_attribute(m_icon_renderer, "gicon", model->columns().col_icon());
	m_column.add_attribute(m_nzbfile_renderer, "file_tuple", model->columns().col_file_tuple());
	m_column.add_attribute(m_nzbfile_renderer, "name", model->columns().col_name());

	append_column(m_column);
}