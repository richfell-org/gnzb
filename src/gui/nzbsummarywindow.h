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
#ifndef __NZB_SUMMARY_WINDOW__
#define __NZB_SUMMARY_WINDOW__

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/alignment.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/frame.h>
#include <pangomm/font.h>
#include <libusenet/binParts.h>
#include "downloadspeedgraph.h"
#include "../gnzb.h"

/*
 * 
 */
class NzbSummaryWindow : public Gtk::EventBox
{
// construction
public:

	NzbSummaryWindow();
	~NzbSummaryWindow() {}

// attributes
public:

	DownloadSpeedGraph& getDownloadSpeeGraph() { return mDlSpeedGraph; }
	const DownloadSpeedGraph& getDownloadSpeeGraph() const { return mDlSpeedGraph; }

// operations
public:

	void clear();
	void setNzbSummary(const GNzb& nzb);

	void pushDlRate(double rate) { mDlSpeedGraph.push_rate(rate); }

// implementation
protected:

	void formatAgeString(time_t t, Glib::ustring& buf);

	void adjustFont(Gtk::Label& label, Pango::Weight weight, int size_offset);

	Gtk::Box mMainBox;
	Gtk::Alignment mInfoGridAlign;
	Gtk::Grid mInfoGrid;
	Gtk::Image mImage;
	Glib::RefPtr<Gdk::Pixbuf> mRefIcon;

	Gtk::Alignment mStateLabelAlign, mStateTextAlign;
	Gtk::Label mStateLabel, mStateText;

	Gtk::Alignment mRepairLabelAlign, mRepairTextAlign;
	Gtk::Label mRepairLabel, mRepairText;

	Gtk::Alignment mTotalLabelAlign, mTotalTextAlign;
	Gtk::Label mTotalLabel, mTotalText;

	Gtk::Alignment mAgeLabelAlign, mAgeTextAlign;
	Gtk::Label mAgeLabel, mAgeText;

	Gtk::Frame mDlSpeedGraphFrame;
	DownloadSpeedGraph mDlSpeedGraph;

	Glib::RefPtr<Gdk::Pixbuf> mFileIcons[BINPARTS_COUNT];
};

#endif  /* __NZB_SUMMARY_WINDOW__ */