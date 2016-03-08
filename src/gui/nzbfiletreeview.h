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
#ifndef __NZB_FILE_TREEVIEW_HEADER__
#define __NZB_FILE_TREEVIEW_HEADER__

#include <glibmm/refptr.h>
#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrendererpixbuf.h>
#include "cellrenderer/nzbfilecellrenderer.h"

namespace Gtk {
	class builder;
}

class NzbFileListStore;

/**
 * 
 * 
 */
class NzbFileTreeView : public Gtk::TreeView
{
// construction/destruction
public:

	NzbFileTreeView();
	NzbFileTreeView(GtkTreeView *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	~NzbFileTreeView();

// attributes
public:

	Glib::RefPtr<NzbFileListStore> get_nzbfile_model();
	Glib::RefPtr<const NzbFileListStore> get_nzbfile_model() const;

// implementation
protected:

	void init_columns();

	Gtk::TreeViewColumn m_column;
	Gtk::CellRendererPixbuf m_icon_renderer;
	NzbFileCellRenderer m_nzbfile_renderer;

private:

	void construct();
};

#endif  /* __NZB_FILE_TREEVIEW_HEADER__ */