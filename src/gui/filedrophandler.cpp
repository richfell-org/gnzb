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
#include "filedrophandler.h"
#include <time.h>
#include <sstream>
#include <gtkmm/widget.h>
#include <gtkmm/selectiondata.h>

FileDropHandler::FileDropHandler(Gtk::Widget& drop_target)
{
	m_dnd_target_entries.push_back(Gtk::TargetEntry("STRING"));
	drop_target.drag_dest_set(m_dnd_target_entries);
	m_connection_dropped = drop_target.signal_drag_data_received().connect(
		sigc::mem_fun(*this, &FileDropHandler::on_drag_receive_data));
}

FileDropHandler::~FileDropHandler()
{
	m_connection_dropped.disconnect();
}

/*
 * 
 */
void FileDropHandler::on_drag_receive_data(
	const Glib::RefPtr<Gdk::DragContext>& context,
	int x,
	int y,
    const Gtk::SelectionData& selection_data,
	guint info,
	guint time)
{
	std::stringstream selection_stream(selection_data.get_text());

	while(!selection_stream.eof())
	{
		// remove the URL prefix and any whitespace at the end of the filename string
		std::string drop_url, drop_text;
		std::getline(selection_stream, drop_url);
		if(drop_url.empty())
			continue;

		drop_text = url2String(drop_url);

		// emit signal for dropped file name
		m_sig_dropped(drop_text);
	}

	// allow the drag source to clean up any drag resources
	context->drag_finish(true, true, (guint)::time(0));
}

/*
 * 
 */
std::string FileDropHandler::url2String(const std::string& url)
{
	std::stringstream strstream;
	Glib::ustring::size_type pos;
	char hexByteTxt[5] = { '0', 'x', 0, };

	if(Glib::ustring::npos != (pos = url.find_first_of("//")))
		pos += 2;
	else
		pos = 0;

	for(; (pos != Glib::ustring::npos) && (pos < url.length()); ++pos)
	{
		switch(url[pos])
		{
			case '\n':
			case '\r':
				break;
			case '%':
				hexByteTxt[2] = url[++pos];
				hexByteTxt[3] = url[++pos];
				strstream << char(strtol(hexByteTxt, 0, 0) & 0xff);
				break;
			default:
				strstream << char(url[pos]);
				break;
		}
	}

	return strstream.str();
}