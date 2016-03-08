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
#include "nzbfileliststore.h"
#include "guiutil.h"
#include "../util/nzbutil.h"
#include "../uiresource.h"

NzbFileListStoreColumnRecord::NzbFileListStoreColumnRecord()
:   Gtk::TreeModelColumnRecord()
{
	add(m_col_file_tuple);
	add(m_col_name);
	add(m_col_icon);
}

NzbFileListStoreColumnRecord::~NzbFileListStoreColumnRecord()
{
}

NzbFileListStore::NzbFileListStore()
:   Glib::ObjectBase("nzbfileliststore"),
	Gtk::ListStore()
{
	set_column_types(m_cols);

	/*
	m_file_icons[BINPARTS_NONE] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/generic.png"));
	m_file_icons[BINPARTS_CONTENT] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/binary.png"));
	m_file_icons[BINPARTS_RAR] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/rar.png"));
	m_file_icons[BINPARTS_ZIP] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/zip.png"));
	m_file_icons[BINPARTS_NFO] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/NFO.png"));
	m_file_icons[BINPARTS_NZB] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon24.png"));
	m_file_icons[BINPARTS_REPAIR_BLOCKS] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/repair.png"));
	m_file_icons[BINPARTS_REPAIR_INDEX] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/repair-idx.png"));
	m_file_icons[BINPARTS_SFV] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/sfv.png"));
	m_file_icons[BINPARTS_MKV] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/mkv.png"));
	m_file_icons[BINPARTS_MP3] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/mp3.png"));
	*/
}

NzbFileListStore::~NzbFileListStore()
{
}

void NzbFileListStore::append_nzb_files(const GNzb& gnzb)
{
	const int file_count = gnzb.nzb_files().getFileCount();
	for(int i = 0; i < file_count; ++i)
	{
		Gtk::TreeIter iter = Gtk::ListStore::append();
		set_values(iter, gnzb, i);
	}
}

Gtk::TreeModel::iterator NzbFileListStore::append(const GNzb& gnzb, int file_idx)
{
	Gtk::TreeModel::iterator iter = Gtk::ListStore::append();
	set_values(iter, gnzb, file_idx);
	return iter;
}

void NzbFileListStore::set_values(Gtk::TreeIter& iter, const GNzb& gnzb, int file_idx)
{
	NzbFileSubjectTool name_tool;

	const NZB::File *p_nzb_file = &gnzb.nzb_files()[file_idx];
	const FileMeta *p_file_meta = &gnzb.files_meta()[file_idx];

	Glib::ustring name = name_tool.get_file_name(*p_nzb_file);

	(*iter)[m_cols.col_file_tuple()] = NzbFileTuple(p_nzb_file, p_file_meta);
	(*iter)[m_cols.col_name()] = name;
	(*iter)[m_cols.col_icon()] = get_icon_for_name(name);
}