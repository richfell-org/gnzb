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
#include "appwindow.h"
#include "../gnzbops.h"
#include "../runtimesettings.h"
#include "../util/sizeformat.h"
#include "gui/nzbtreeview.h"
#include "gui/nzbliststore.h"
#include "gui/guiutil.h"
#include "nntp/fetch.h"
#include <glibmm/main.h>

static unsigned long last_update_seconds = 0;
static sigc::connection on_idle_sig_connection;

void GNzbApplicationWindow::connect_download_progress()
{
	// connect the "status update" function to the on-idle signal
	if(!on_idle_sig_connection.connected())
	{
		last_update_seconds = time(nullptr);
		on_idle_sig_connection = Glib::signal_idle().connect(
			sigc::mem_fun(*this, &GNzbApplicationWindow::on_download_progress));
	}
}

void GNzbApplicationWindow::disconnect_download_progress()
{
	if(on_idle_sig_connection.connected())
		on_idle_sig_connection.disconnect();
}

bool GNzbApplicationWindow::on_download_progress()
{
	if(!GNzbOps::ptr_active_gnzb || (GNzbOps::ptr_active_gnzb->state() == GNzbState::COMPLETE))
		return true;

	// only update once a second
	unsigned long seconds = time(nullptr) - GNzbOps::ptr_active_gnzb->download_data().start_time();
	if((0 >= seconds) || (last_update_seconds == seconds))
	   return true;

	last_update_seconds = seconds;
	unsigned long secondsRemaining;
	unsigned long bytesRemaining;
	unsigned long dlCount;

	std::unique_lock<std::mutex> nzbLock(GNzbOps::ptr_active_gnzb->update_mutex());
	bytesRemaining = GNzbOps::ptr_active_gnzb->download_data().get_remaining();
	dlCount = GNzbOps::ptr_active_gnzb->download_data().session_bytes();
	nzbLock.unlock();

	const int connsCount = NntpFetch::get_active_connection_count();

	// calculate DL rate
	unsigned long bytesPerSecond = dlCount / seconds;
	if(0 < bytesPerSecond)
		secondsRemaining = bytesRemaining / bytesPerSecond;

	// update graph and peak rate value
	double dlRate = (double)bytesPerSecond;
	if(GNzbOps::ptr_active_gnzb->download_data().peak_rate() < dlRate)
		GNzbOps::ptr_active_gnzb->download_data().peak_rate(dlRate);
	m_win_summary.pushDlRate(dlRate);

	// get the itererator to the active GNzb
	Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
	Gtk::TreeIter active_iter = ref_model->find_gnzb(GNzbOps::ptr_active_gnzb.get());

	// check for completion
	if(NntpFetch::is_message_queue_empty() && (0 == connsCount))
	{
		// set the state to COMPLETE or STOPPED if we have incomplete download
		if(!GNzbOps::ptr_active_gnzb->download_data().is_complete())
			move_gnzb_to_state(active_iter, GNzbState::STOPPED);
		else
		{
			// move to another directory?
			GNzbOps::move_completed(*GNzbOps::ptr_active_gnzb);
			move_gnzb_to_state(active_iter, GNzbState::COMPLETE);
		}

		// set stats to 0
		bytesRemaining = 0;
		secondsRemaining = 0;

		// update the summary info if selected
		//if(mp_treeview->isNzbSelected(pActiveNzb))
		//	m_win_summary->setNzbSummary(*pActiveNzb);

		// do notifications if needed
		//Notifier *pNotifier = EnzybeeSettings::get_notifier();
		//if(nullptr != pNotifier)
		//{
			// completed
		//	if(pActiveNzb->is_state(AppNzb::State::COMPLETE))
		//		pNotifier->sendNotification(pActiveNzb->get_name(), Glib::ustring("Done"));
			// incomplete (failed)
		//	else
		//		pNotifier->sendNotification(pActiveNzb->get_name(), Glib::ustring("Failed"));
		//}

		// sound?
		if(RuntimeSettings::notifications().sound_on_gnzb_finished())
			play_sound(RuntimeSettings::notifications().gnzb_finished_sound());

		// call script?
		//scripting_nzb_finished(pActiveNzb);

		// update GUI elements
		ref_model->row_changed(Gtk::TreePath(active_iter), active_iter);

		// no active download as of right now, but check for the next ready NZB
		Gtk::TreeIter iter_next = find_next_queued_nzb();
		if(iter_next == mp_treeview->get_model()->children().end())
		{
			GNzbOps::ptr_active_gnzb.reset();
			update_gui_state();

			// no NZB is ready, just go to idle state
			//chdir(startupDir);
			on_idle_sig_connection.disconnect();

			// sound?
			if(RuntimeSettings::notifications().sound_on_queue_finished())
				play_sound(RuntimeSettings::notifications().queue_finished_sound());
		}
		else
		{
			// move the next ready NZB to the top
			Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
			Gtk::TreeIter iter_first = ref_model->children().begin();
			Gtk::TreeIter iter_end = ref_model->children().end();

			ref_model->move(iter_next, iter_first);

			// move the just finished NZB down below the queued NZBs (if any)
			Gtk::TreeIter firstNotWaitingIter = iter_first;
			for(; firstNotWaitingIter != iter_end; ++firstNotWaitingIter)
			{
				if(active_iter == firstNotWaitingIter) continue;

				std::shared_ptr<GNzb> ptr_gnzb = (*firstNotWaitingIter)[ref_model->columns().gnzb()];
				if(ptr_gnzb->not_state({GNzbState::WAITING, GNzbState::PAUSED, GNzbState::COMPLETE}))
					break;
			}

			ref_model->move(active_iter, firstNotWaitingIter);

			// found the next ready NZB, get it started
			mp_treeview->get_selection()->select(iter_next);
			std::shared_ptr<GNzb> ptr_gnzb_next = (*iter_next)[ref_model->columns().gnzb()];
			GNzbOps::start_download(ptr_gnzb_next);
			move_gnzb_to_state(iter_next, GNzbState::DOWNLOADING);
			ref_model->row_changed(Gtk::TreePath(iter_next), iter_next);
		}
	}
	// else current DL is not complete yet
	else
	{
		// update the file tree info if it is showing
		if(is_details_view())
		{
			Gtk::TreeView *p_files_treeview = find_file_treeview(GNzbOps::ptr_active_gnzb);
			if(p_files_treeview != nullptr)
			{
				Glib::RefPtr<Gtk::TreeModel> ref_files_model = p_files_treeview->get_model();

				int file_count = GNzbOps::ptr_active_gnzb->nzb_files().getFileCount();
				std::vector<FileMeta>& file_meta = GNzbOps::ptr_active_gnzb->files_meta();
				std::lock_guard<std::mutex> updateLock(GNzbOps::ptr_active_gnzb->update_mutex());
				for(int i = 0; i < file_count; ++i)
				{
					if(file_meta[i].updated())
					{
						file_meta[i].updated(false);
						Gtk::TreeModel::iterator file_iter = ref_files_model->children()[i];
						ref_files_model->row_changed(Gtk::TreePath(file_iter), file_iter);
					}
				}
			}
		}

		// update NZB file's GUI with latest values
		mp_treeview->get_model()->row_changed(Gtk::TreePath(active_iter), active_iter);
	}

	// format stat values
	char dlSpeedTxt[32] = { '-', 0 }, timeRemainTxt[32] = { '-', 0, }, byteRemainTxt[32] = { '-', 0, };
	if(GNzbOps::ptr_active_gnzb)
	{
		formatBytePerSecond(dlSpeedTxt, dlCount, seconds);
		if(0 < bytesPerSecond)
			formatTimeFromSeconds(timeRemainTxt, secondsRemaining);
		formatULongBytes(byteRemainTxt, bytesRemaining);
	}

	// update the info bar
	m_info_bar.remaining_time().set_text(timeRemainTxt);
	m_info_bar.dl_speed().set_text(dlSpeedTxt);
	m_info_bar.remaining_size().set_text(byteRemainTxt);
	update_connection_info();

	return true;
}