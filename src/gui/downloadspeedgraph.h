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
#ifndef __DOWNLOAD_SPEED_GRAPH__
#define __DOWNLOAD_SPEED_GRAPH__

#include <deque>
#include <string>
#include <glibmm/property.h>
#include <gtkmm/drawingarea.h>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>

/*
 * 
 */
class DownloadSpeedGraph : public Gtk::DrawingArea
{
// construction
public:

	DownloadSpeedGraph(unsigned int history_size = 100);
	~DownloadSpeedGraph();

// attributes
public:

	unsigned int getHistorySize() const { return m_history_size; }
	void setHistorySize(unsigned int history_size) { m_history_size = history_size; }

// operations
public:

	void set_peak(double peak_rate);
	void push_rate(double rate);
	void clear();

	void graph_rates();

// implementation
protected:

	bool on_configure(GdkEventConfigure* event);
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

	unsigned int m_history_size;

	double m_peak_rate;
	std::string m_peak_rate_text;

	std::deque<double> m_rate_history;
	Cairo::RefPtr<Cairo::ImageSurface> m_graph_surface;
};

#endif  /* __DOWNLOAD_SPEED_GRAPH__ */