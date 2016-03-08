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
#ifndef __FILE_DROP_HANDLER_HEADER__
#define __FILE_DROP_HANDLER_HEADER__

namespace Glib {
	template<typename T> class RefPtr;
}
namespace Gdk {
	class DragContext;
}
namespace Gtk {
	class Widget;
	class SelectionData;
}

#include <string>
#include <vector>
#include <sigc++/sigc++.h>
#include <gtkmm/targetentry.h>

/*
 * Class for handling drag and drop.
 */
class FileDropHandler
{
// construction
public:

	FileDropHandler(Gtk::Widget& drop_target);
	~FileDropHandler();

// attribtes
public:

	sigc::signal<void, const std::string&>& signal_dropped() { return m_sig_dropped; }
	const sigc::signal<void, const std::string&>& signal_dropped() const { return m_sig_dropped; }

// operations
public:

	void on_drag_receive_data(
		const Glib::RefPtr<Gdk::DragContext>& context,
		int x,
		int y,
		const Gtk::SelectionData& selection_data,
		guint info,
		guint time);

	std::string url2String(const std::string& url);

protected:

	sigc::signal<void, const std::string&> m_sig_dropped;
	sigc::connection m_connection_dropped;
	std::vector<Gtk::TargetEntry> m_dnd_target_entries;
};


#endif  /* __FILE_DROP_HANDLER_HEADER__ */