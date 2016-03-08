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
#ifndef __GUI_GNZB_HEADER__
#define __GUI_GNZB_HEADER__

namespace NZB { class File; }
class FileMeta;

enum class GNzbAction { START, PAUSE, DELETE, SHOW, FINISHED, ACTION_COUNT, };

/**
 * This class wraps an NZB::File and a
 * FileMeta pointers for use with Glib::Property
 * and GUI CellRenderer classes.
 * 
 * Instances of this class take no ownership
 * of these pointers and the pointers are
 * assumed to point to objects within a managed
 * block of memory.
 *
 * Glib::Propery, TreeViewColumn and CellRenderer derived
 * classes using properties like Glib::Property<NZB::File*>
 * just don't work...many messages like this are seen when
 * a view using pointer properties and a TreeViewColumn
 * to map properties from a TreeModel:
 * 
 * (gnzb:11565): GLib-GObject-CRITICAL **: g_value_copy: assertion 'g_value_type_compatible (G_VALUE_TYPE (src_value), G_VALUE_TYPE (dest_value))' failed
 */
class NzbFileTuple
{
public:

	NzbFileTuple() {}
	NzbFileTuple(NzbFileTuple&& that);
	NzbFileTuple(const NzbFileTuple& that);
	NzbFileTuple(const NZB::File *p_file, const FileMeta *p_meta);
	~NzbFileTuple() {}

public:

	const NZB::File *nzb_file() const { return mp_nzb_file; }
	const FileMeta *file_meta() const { return mp_file_meta; }

	NzbFileTuple& operator =(NzbFileTuple&& that);
	NzbFileTuple& operator =(const NzbFileTuple& that);

 private:

	const NZB::File *mp_nzb_file{nullptr};
	const FileMeta *mp_file_meta{nullptr};
};

#endif  /* __GUI_GNZB_HEADER__ */