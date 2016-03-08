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
#include "downloadspeedgraph.h"
#include "../util/sizeformat.h"

DownloadSpeedGraph::DownloadSpeedGraph(unsigned int history_size/* = 100*/)
:   Gtk::DrawingArea(),
	m_history_size(history_size),
	m_peak_rate(0.0),
	m_peak_rate_text(""),
	m_rate_history()
{
	m_rate_history.clear();
	m_graph_surface = ::Cairo::ImageSurface::create(::Cairo::FORMAT_ARGB32, 1, 1);
	signal_configure_event().connect(sigc::mem_fun(this, &DownloadSpeedGraph::on_configure));
	signal_draw().connect(sigc::mem_fun(this, &DownloadSpeedGraph::on_draw));
}

DownloadSpeedGraph::~DownloadSpeedGraph()
{
}

void DownloadSpeedGraph::set_peak(double peak_rate)
{
	m_peak_rate = peak_rate;

	char txt[32];
	formatBytePerSecond(txt, m_peak_rate);
	m_peak_rate_text.assign(Glib::ustring::compose("Peak %1", txt));
}

void DownloadSpeedGraph::push_rate(double rate)
{
	if(m_peak_rate < rate)
		set_peak(rate);

	if(m_history_size == m_rate_history.size())
		m_rate_history.pop_back();
	m_rate_history.push_front(rate);

	graph_rates();
	queue_draw();
}

void DownloadSpeedGraph::clear()
{
	m_peak_rate_text.clear();
	m_peak_rate = 0.0;
	m_rate_history.clear();

	graph_rates();
	queue_draw();
}

void DownloadSpeedGraph::graph_rates()
{
	Glib::RefPtr<Gtk::StyleContext> refStyleContext = get_style_context();
	Gdk::RGBA foreground = refStyleContext->get_color();
	Gdk::RGBA background = refStyleContext->get_background_color();

	::Cairo::RefPtr< ::Cairo::Context > ctx = ::Cairo::Context::create(m_graph_surface);

	const int imgWidth = m_graph_surface->get_width();
	const int imgHeight = m_graph_surface->get_height();

	// clear image
	ctx->set_source_rgba(background.get_red(), background.get_green(), background.get_blue(), 1.0/*background.get_alpha()*/);
	ctx->rectangle(0.0, 0.0, imgWidth, imgHeight);
	ctx->fill();

	ctx->translate(0, 2);
	ctx->scale(1, imgHeight - 2);

	// graph the points
	std::deque<double>::iterator rateIter = m_rate_history.begin();
	for(int i = imgWidth - 1; (rateIter != m_rate_history.end()) && (i >= 0); --i, ++rateIter)
	{
		double value = *rateIter / m_peak_rate;

		Cairo::RefPtr<Cairo::LinearGradient> refGradient = Cairo::LinearGradient::create(0, 1.0, 0, 1.0 - value);
		refGradient->add_color_stop_rgba(0, foreground.get_red(), foreground.get_green(), foreground.get_blue(), 0.08);
		refGradient->add_color_stop_rgba(0.65, foreground.get_red(), foreground.get_green(), foreground.get_blue(), 0.20);
		refGradient->add_color_stop_rgba(0.95, foreground.get_red(), foreground.get_green(), foreground.get_blue(), 0.60);
		refGradient->add_color_stop_rgba(1.0, foreground.get_red(), foreground.get_green(), foreground.get_blue(), 1.0);
		ctx->set_source(refGradient);
		ctx->rectangle(i, 1.0 - value, 2, value);
		ctx->fill();
	}

	// set the peak text
	if(!m_peak_rate_text.empty())
	{
		//ctx->set_identity_matrix();
		ctx = ::Cairo::Context::create(m_graph_surface);
		ctx->set_source_rgb(1.0, 1.0, 1.0);
		//ctx->select_font_face("NanumGothic", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
		ctx->set_font_size(10.0);
		ctx->move_to(2, imgHeight - 5);
		ctx->show_text(m_peak_rate_text);
	}
}

bool DownloadSpeedGraph::on_configure(GdkEventConfigure* event)
{
	m_graph_surface = ::Cairo::ImageSurface::create(::Cairo::FORMAT_ARGB32, event->width, event->height);
	graph_rates();
	return true;
}

bool DownloadSpeedGraph::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	cr->set_source(m_graph_surface, 0, 0);
	cr->paint();
	return true;
}