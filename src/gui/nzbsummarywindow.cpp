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
#include "nzbsummarywindow.h"
#include "../uiresource.h"
#include "../util/sizeformat.h"
#include <gdkmm/rgba.h>
#include <gtkmm/stock.h>
#include <time.h>
#include <stdlib.h>

NzbSummaryWindow::NzbSummaryWindow()
:   Gtk::EventBox(),
	mMainBox(),
	mInfoGridAlign(0.0, 0.5, 0.0),
	mStateLabelAlign(1.0, 0.5, 0.0), mStateTextAlign(0.0, 0.5, 0.0), mStateLabel("NZB State:"), mStateText(),
	mRepairLabelAlign(1.0, 0.5, 0.0), mRepairTextAlign(0.0, 0.5, 0.0), mRepairLabel("Repair Files:"), mRepairText(),
	mTotalLabelAlign(1.0, 0.5, 0.0), mTotalTextAlign(0.0, 0.5, 0.0), mTotalLabel("Total Files:"), mTotalText(),
	mAgeLabelAlign(1.0, 0.5, 0.0), mAgeTextAlign(0.0, 0.5, 0.0), mAgeLabel("Age:"), mAgeText(),
	mDlSpeedGraph(100)
{
	mFileIcons[BINPARTS_NONE] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/generic.png"));
	mFileIcons[BINPARTS_CONTENT] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/binary.png"));
	mFileIcons[BINPARTS_RAR] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/rar.png"));
	mFileIcons[BINPARTS_ZIP] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/zip.png"));
	mFileIcons[BINPARTS_NFO] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/NFO.png"));
	mFileIcons[BINPARTS_NZB] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon64.png"));
	mFileIcons[BINPARTS_REPAIR_BLOCKS] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/repair.png"));
	mFileIcons[BINPARTS_REPAIR_INDEX] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/repair-idx.png"));
	mFileIcons[BINPARTS_SFV] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/sfv.png"));
	mFileIcons[BINPARTS_MKV] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/mkv.png"));
	mFileIcons[BINPARTS_MP3] = Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/64x64/mp3.png"));

	override_color(Gdk::RGBA("#ffffff"));
	override_background_color(Gdk::RGBA("#5a626f"));
	//mMainBox.override_font(Pango::FontDescription("NanumGothic 8"));

	Pango::FontDescription labelFontDesc = Pango::FontDescription("Liberation Sans Bold 8");
	Pango::FontDescription textFontDesc = Pango::FontDescription("NanumGothic Semi-Bold 8");

	mMainBox.set_margin_left(10);
	mMainBox.set_margin_right(3);
	mMainBox.set_margin_top(7);
	mMainBox.set_margin_bottom(7);
	mMainBox.set_spacing(12);

	// use a directory icon for default/cleared state
	mRefIcon = render_icon_pixbuf(Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_DIALOG);
	mImage.set(mRefIcon);
	mMainBox.pack_start(mImage, false, false);

	mInfoGrid.set_column_spacing(6);
	mInfoGrid.set_row_spacing(2);

	// NZB state
	adjustFont(mStateLabel, Pango::WEIGHT_BOLD, -1);
	mStateLabel.override_font(labelFontDesc);
	mStateLabel.set_justify(Gtk::JUSTIFY_RIGHT);
	adjustFont(mStateText, Pango::WEIGHT_NORMAL, -1);
	mStateText.override_font(textFontDesc);
	mStateText.set_justify(Gtk::JUSTIFY_LEFT);
	mStateLabelAlign.add(mStateLabel);
	mStateTextAlign.add(mStateText);
	mInfoGrid.attach(mStateLabelAlign,0,0,1,1);
	mInfoGrid.attach(mStateTextAlign,1,0,1,1);

	// Repair files
	adjustFont(mRepairLabel, Pango::WEIGHT_BOLD, -1);
	mRepairLabel.override_font(labelFontDesc);
	mRepairLabel.set_justify(Gtk::JUSTIFY_RIGHT);
	adjustFont(mRepairText, Pango::WEIGHT_NORMAL, -1);
	mRepairText.override_font(textFontDesc);
	mRepairText.set_justify(Gtk::JUSTIFY_LEFT);
	mRepairLabelAlign.add(mRepairLabel);
	mRepairTextAlign.add(mRepairText);
	mInfoGrid.attach(mRepairLabelAlign,0,1,1,1);
	mInfoGrid.attach(mRepairTextAlign,1,1,1,1);

	// Total files
	adjustFont(mTotalLabel, Pango::WEIGHT_BOLD, -1);
	mTotalLabel.override_font(labelFontDesc);
	mTotalLabel.set_justify(Gtk::JUSTIFY_RIGHT);
	adjustFont(mTotalText, Pango::WEIGHT_NORMAL, -1);
	mTotalText.override_font(textFontDesc);
	mTotalText.set_justify(Gtk::JUSTIFY_LEFT);
	mTotalLabelAlign.add(mTotalLabel);
	mTotalTextAlign.add(mTotalText);
	mInfoGrid.attach(mTotalLabelAlign,0,2,1,1);
	mInfoGrid.attach(mTotalTextAlign,1,2,1,1);

	// Age
	adjustFont(mAgeLabel, Pango::WEIGHT_BOLD, -1);
	mAgeLabel.override_font(labelFontDesc);
	mAgeLabel.set_justify(Gtk::JUSTIFY_RIGHT);
	adjustFont(mAgeText, Pango::WEIGHT_NORMAL, -1);
	mAgeText.override_font(textFontDesc);
	mAgeText.set_justify(Gtk::JUSTIFY_LEFT);
	mAgeLabelAlign.add(mAgeLabel);
	mAgeTextAlign.add(mAgeText);
	mInfoGrid.attach(mAgeLabelAlign,0,3,1,1);
	mInfoGrid.attach(mAgeTextAlign,1,3,1,1);

	mInfoGridAlign.add(mInfoGrid);
	mMainBox.pack_start(mInfoGridAlign, false, false);

	mMainBox.override_background_color(get_style_context()->get_background_color());
	mInfoGrid.override_background_color(get_style_context()->get_background_color());
	mDlSpeedGraph.override_background_color(get_style_context()->get_background_color());
	mDlSpeedGraph.override_color(Gdk::RGBA("#79f27f"));
	mDlSpeedGraph.set_size_request(100, -1);
	//mDlSpeedGraphFrame.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	mDlSpeedGraphFrame.set_shadow_type(Gtk::SHADOW_IN);
	mDlSpeedGraphFrame.add(mDlSpeedGraph);
	mMainBox.pack_end(mDlSpeedGraphFrame, Gtk::PACK_SHRINK);
	mDlSpeedGraph.show();

	add(mMainBox);
	clear();
}

void NzbSummaryWindow::clear()
{
	mStateText.set_text("No NZB Selected");
	mRepairText.set_text("");
	mTotalText.set_text("");
	mAgeText.set_text("");
	//mDlSpeedGraph.clear();
	mImage.set(mRefIcon);
}

void NzbSummaryWindow::setNzbSummary(const GNzb& nzb)
{
	const int fileCount = nzb.nzb_files().getFileCount();

	NZB::BinPartsOracle partsOracle;
	auto& files = nzb.nzb_files();
	auto files_meta = nzb.files_meta();

	int repairFileCount = 0;
	int repairBlockCount = 0;

	// determine repair file/block info
	const unsigned int meta_count = files_meta.size();
	for(unsigned int i = 0; i < meta_count; ++i)
	{
		if(BINPARTS_REPAIR_BLOCKS == files_meta[i].type())
		{
			++repairFileCount;
			repairBlockCount += partsOracle.get_repair_block_count(files[i].getSubject().c_str());
		}
	}

	// set the icon for the determined "type" of the NZB content
	mImage.set(mFileIcons[nzb.collection_meta().type()]);

	// NZB state
	mStateText.set_text(get_gnzb_state_display_name(nzb.state()));

	if(0 == repairFileCount)
		mRepairText.set_text("None");
	else
	{
		mRepairText.set_text(
             Glib::ustring::compose(
				"%1 file%2 (%3 block%4)",
                repairFileCount, (repairFileCount > 1 ? "s" : ""),
                repairBlockCount, (repairBlockCount > 1 ? "s" : "")));
	}

	// total file count and size
	char szbuf[16];
	formatULongBytes(szbuf, nzb.collection_meta().total_size());
	mTotalText.set_text(Glib::ustring::compose("%1 file%2 (%3)", fileCount, (fileCount > 1 ? "s" : ""), szbuf));

	// age
	Glib::ustring ageText;
	formatAgeString(files[0].getTimestamp(), ageText);
	mAgeText.set_text(ageText);

	// DL rate
	if(nzb.download_data().peak_rate() > 0.0)
		mDlSpeedGraph.set_peak(nzb.download_data().peak_rate());
}

void NzbSummaryWindow::formatAgeString(time_t t, Glib::ustring& buf)
{
	struct tm curTime, givenTime;

	time_t tCur = time(0);
	localtime_r(&tCur, &curTime);
	localtime_r(&t, &givenTime);

	int monthDiff = 0;
	int yearDiff = curTime.tm_year - givenTime.tm_year;
	if(0 == yearDiff)
		monthDiff = curTime.tm_mon - givenTime.tm_mon;
	else
	{
		if(curTime.tm_mon == givenTime.tm_mon)
			monthDiff = 0;
		else if(curTime.tm_mon > givenTime.tm_mon)
			monthDiff = (12 - givenTime.tm_mon) + curTime.tm_mon - 12;
		else
		{
			--yearDiff;
			monthDiff = (12 - givenTime.tm_mon) + curTime.tm_mon;
		}
	}

	// less than one month old?
	if((0 == yearDiff) && (0 == monthDiff))
	{
		// only hours old?
		if(curTime.tm_mday == givenTime.tm_mday)
		{
			int hrDiff = curTime.tm_hour - givenTime.tm_hour;
			if(0 < hrDiff)
			{
				buf.append(Glib::ustring::compose("%1 hour", hrDiff));
				if(1 < hrDiff) buf.append("s");
			}
			else
			{
				int minDiff = curTime.tm_min - givenTime.tm_min;
				buf.append(Glib::ustring::compose("%1 minute", minDiff));
				if(1 < minDiff) buf.append("s");
			}
	
		}
		else
		{
			int dayDiff = curTime.tm_mday - givenTime.tm_mday;
			buf.append(Glib::ustring::compose("%1 day", dayDiff));
			if(1 < dayDiff)
				buf.append("s");
		}
	}
	else
	{
		if(0 < yearDiff)
		{
			buf.append(Glib::ustring::compose("%1 year", yearDiff));
			if(1 < yearDiff) buf.append("s");
		}
		if(0 < monthDiff)
		{
			if(0 < yearDiff) buf.append(", ");
			buf.append(Glib::ustring::compose("%1 month", monthDiff));
			if(1 < monthDiff) buf.append("s");
		}

		int dayDiff = (tCur / 86400) - (t / 86400);
		buf.append(Glib::ustring::compose(" (%1 days)", dayDiff));
	}
}

void NzbSummaryWindow::adjustFont(Gtk::Label& label, Pango::Weight weight, int size_offset)
{
	Glib::RefPtr<Gtk::StyleContext> ref_style = label.get_style_context();
	Pango::FontDescription font_desc = ref_style->get_font(Gtk::STATE_FLAG_NORMAL);
	font_desc.set_weight(weight);
	if(0 != size_offset)
		font_desc.set_size(font_desc.get_size() + (size_offset * Pango::SCALE));
	label.override_font(font_desc);
}