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
#include "application.h"
#include "runtimesettings.h"
#include "nntp/fetch.h"
#include "gui/appwindow.h"
#include "gui/guiutil.h"
#include "util/sizeformat.h"
#include "util/sysutil.h"
#include <glibmm/main.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/messagedialog.h>
#include <memory>
#include <cstdlib>


static void queue_files(GNzb& gnzb, bool add_repair_files)
{
	auto& nzb_files = gnzb.nzb_files();
	auto& nzb_files_meta = gnzb.files_meta();
	const int file_count = nzb_files.getFileCount();
	for(int iFile = 0; iFile < file_count; ++iFile)
	{
		// ignore files that are already complete
		if(nzb_files[iFile].getSegmentCount() == nzb_files_meta[iFile].completed_segments())
			continue;

		const int seg_count = nzb_files[iFile].getSegmentCount();
		for(int iSeg = 0; iSeg < seg_count; ++iSeg)
		{
			// don't add PAR2 repair files if thay are not desired
			//if(!add_repair_files && ((BINPARTS_REPAIR_INDEX == nzb_files_meta[iFile].get_type()) || (BINPARTS_REPAIR_BLOCKS == nzb_files_meta[iFile].type)))
			if(!add_repair_files && nzb_files_meta[iFile].is_one_of_type({BINPARTS_REPAIR_INDEX, BINPARTS_REPAIR_BLOCKS}))
				continue;

			NntpFetch::push_message(&gnzb, iFile, iSeg);
		}
	}
}

bool GNzbApplication::chdir_to_output(const GNzb& gnzb)
{
	struct stat dirStat;
	int status = 0;

	// stat the output dir to see if it exists, and if it does is it a dir?
	status = stat(gnzb.output_path().c_str(), &dirStat);
	if((-1 == status) && (ENOENT != errno))
	{
		show_error(
			Glib::ustring::compose(
				"Error checking the status of the output directory %1\nfor NZB file %2\n%3",
			    gnzb.output_path().c_str(), gnzb.path().c_str(), strerror(errno)));
		return false;
	}
	// does not exist, so we will create it
	else if((-1 == status) && (ENOENT == errno))
	{
		if(0 != mkdir(gnzb.output_path().c_str(), 0755))
		{
			show_error(
				Glib::ustring::compose(
					"Error creating output directory %1\nfor NZB file %2\nExists and is not a directory.",
					gnzb.output_path().c_str(), gnzb.path().c_str()));
			return false;
		}
	}
	// OK if already exists and is a directory, else what we want
	// as the directory is already a file of some sort
	else if((0 == status) && !S_ISDIR(dirStat.st_mode))
	{
		show_error(
			Glib::ustring::compose(
				"The output directory %1\nfor NZB file %2\nExists and is not a directory.",
				gnzb.output_path().c_str(), gnzb.path().c_str()));
		return false;
	}

	// make the directory current
	if(0 != chdir(gnzb.output_path().c_str()))
	{
		show_error(
			Glib::ustring::compose(
				"Cannot change to output directory %1\nfor NZB file %2\n%3",
				gnzb.output_path().c_str(), gnzb.path().c_str(), strerror(errno)));
		return false;
	}

	return true;
}

void GNzbApplication::move_completed(GNzb& gnzb)
{
	std::string dest_dir;

	// need to move to a different dir?
	if(gnzb.ptr_group() && gnzb.ptr_group()->doMoveToDir())
		dest_dir = gnzb.ptr_group()->getMoveToDir();
	else if(RuntimeSettings::locations().move_completed())
		dest_dir = RuntimeSettings::locations().moveto_path();

	if(dest_dir.empty()) return;

	std::stringstream cmdstream;
	cmdstream << "mv \"" << gnzb.output_path() << "\" \"" << dest_dir << '\"';

	// set the new location of the downloaded NZB
	if('/' != dest_dir.back()) dest_dir.push_back('/');
	dest_dir.append(parse_file_name(gnzb.path()));
	gnzb.output_path(dest_dir);

	// execute the move command
	system(cmdstream.str().c_str());
}

bool GNzbApplication::is_download_active()
{
	return bool(m_ptr_active_gnzb);
}

bool GNzbApplication::start_download(std::shared_ptr<GNzb>& ptr_gnzb)
{
	// make (if needed) and chdir to the output directory
	// any error is displayed before the function returns
	if(!chdir_to_output(*ptr_gnzb))
		return false;

	// initialized the download database if needed
	if(!ptr_gnzb->download_db().isOpen())
	{
		ptr_gnzb->download_db().open(".enzyBee.db");
		ptr_gnzb->download_db().close();
	}

	// reset the DL stats
	ptr_gnzb->download_data().reset_session();

	// add all of the NZB file's segments to the queue
	queue_files(*ptr_gnzb, true);
	m_ptr_active_gnzb = ptr_gnzb;

	m_last_update_seconds = 0;
	m_ptr_mainwin->move_gnzb_to_state(ptr_gnzb.get(), GNzbState::DOWNLOADING);
	if(!m_idle_connection.connected())
		m_idle_connection = Glib::signal_idle().connect(sigc::mem_fun(*this, &GNzbApplication::on_download_progress));

	// send notification if needed

	return true;
}

void GNzbApplication::pause_download()
{
	if(!is_download_active()) return;

	WaitCursorTool waitCursor;

	// stop the download threads & remove all NZB segments from the download queue
	NntpFetch::abort_download();
	NntpFetch::clear_message_queue();

	// reset the active NZB state
	m_ptr_active_gnzb.reset();
}

bool GNzbApplication::start_next_download()
{
	Gtk::TreeIter iter_next = m_ptr_mainwin->find_next_queued_nzb();
	if(!iter_next)
		m_idle_connection.disconnect();
	else
	{
		std::shared_ptr<GNzb> ptr_gnzb_next = m_ptr_mainwin->get_gnzb(iter_next);
		if(!start_download(ptr_gnzb_next))
		{
			show_error(
				*m_ptr_mainwin,
				Glib::ustring(m_ptr_mainwin->get_display_name(iter_next)),
				Glib::ustring("Failed to start download!"));
		}
		else
		{
			m_ptr_mainwin->move_gnzb_to_top(iter_next);
		}

		return true;
	}

	return false;
}

void GNzbApplication::on_gnzb_action(const Gtk::TreePath& gnzb_path, GNzbAction action)
{
	int response;
	bool delete_files = false;

	// access the GNzb
	std::shared_ptr<GNzb> ptr_gnzb = m_ptr_mainwin->get_gnzb(gnzb_path);

	switch(action)
	{
		// Delete the GNzb from the list
		case GNzbAction::DELETE:
			// confirm with user
			response = get_yes_or_no_or_yeswithdelete(
				*m_ptr_mainwin,
				Glib::ustring::compose("Are you sure you want to remove %1 ?", m_ptr_mainwin->get_display_name(gnzb_path)),
				Glib::ustring("Choose \"Yes\" to remove the NZB from the queue. Use \"Yes and delete files\" to delete downloaded files as well"));
			if(Gtk::RESPONSE_NO == response) return;

			// deleting the active download?
			if(m_ptr_active_gnzb == ptr_gnzb)
			{
				pause_download();
				m_ptr_mainwin->move_gnzb_to_state(gnzb_path, GNzbState::PAUSED);
				start_next_download();
				m_ptr_mainwin->update_connection_info();
			}

			delete_files = (YES_WITH_DELETE == response);
			// fall through to GNzb removal
		case GNzbAction::FINISHED:
			m_ptr_mainwin->remove_gnzb(gnzb_path, delete_files);
			break;

		case GNzbAction::START:
			if(is_download_active())
				m_ptr_mainwin->move_gnzb_to_state(gnzb_path, GNzbState::WAITING);
			else if(!start_download(ptr_gnzb))
				show_error(*m_ptr_mainwin, Glib::ustring("Failed to start download!"));
			break;

		// pause a GNzb (may or may not be the active download)
		case GNzbAction::PAUSE:
			m_ptr_mainwin->move_gnzb_to_state(gnzb_path, GNzbState::PAUSED);
			if(m_ptr_active_gnzb == ptr_gnzb)
			{
				pause_download();
				start_next_download();
				m_ptr_mainwin->update_connection_info();
			}
			break;

		// show the files in GUI file system browser
		case GNzbAction::SHOW:
			system(Glib::ustring::compose("nautilus \"%1\" &", ptr_gnzb->output_path()).c_str());
			break;
		default:
			break;
	}
}

void GNzbApplication::on_download_complete()
{
	// set the state to COMPLETE or STOPPED if we have incomplete download
	if(!m_ptr_active_gnzb->download_data().is_complete())
		m_ptr_mainwin->move_gnzb_to_state(m_ptr_active_gnzb.get(), GNzbState::STOPPED);
	else
	{
		// move to another directory?
		move_completed(*m_ptr_active_gnzb);
		m_ptr_mainwin->move_gnzb_to_state(m_ptr_active_gnzb.get(), GNzbState::COMPLETE);
	}

	// sound?
	if(RuntimeSettings::notifications().sound_on_gnzb_finished())
		play_sound(RuntimeSettings::notifications().gnzb_finished_sound());
}

bool GNzbApplication::on_download_progress()
{
	if(!m_ptr_active_gnzb || (m_ptr_active_gnzb->state() == GNzbState::COMPLETE))
		return true;

	// only update once a second
	unsigned long seconds = time(nullptr) - m_ptr_active_gnzb->download_data().start_time();
	if((0 >= seconds) || (m_last_update_seconds == seconds))
	   return true;

	m_last_update_seconds = seconds;
	unsigned long remaining_seconds;
	unsigned long remaining_bytes;
	unsigned long dl_count;

	std::unique_lock<std::mutex> nzbLock(m_ptr_active_gnzb->update_mutex());
	remaining_bytes = m_ptr_active_gnzb->download_data().get_remaining();
	dl_count = m_ptr_active_gnzb->download_data().session_bytes();
	nzbLock.unlock();

	// calculate DL rate
	unsigned long Bps = dl_count / seconds;
	if(0 < Bps)
		remaining_seconds = remaining_bytes / Bps;

	// update graph and peak rate value
	double dl_rate = double(Bps);
	if(m_ptr_active_gnzb->download_data().peak_rate() < dl_rate)
		m_ptr_active_gnzb->download_data().peak_rate(dl_rate);
	m_ptr_mainwin->summary_window().pushDlRate(dl_rate);

	// check for completion
	if(NntpFetch::is_message_queue_empty() && !NntpFetch::has_active_connection())
	{
		on_download_complete();

		// set stats to 0
		remaining_bytes = 0;
		remaining_seconds = 0;

		if(!start_next_download())
		{
			// reset the active download pointer
			m_ptr_active_gnzb.reset();

			// sound?
			if(RuntimeSettings::notifications().sound_on_queue_finished())
				play_sound(RuntimeSettings::notifications().queue_finished_sound());
		}
	}
	// else not complete yet
	else
	{
		m_ptr_mainwin->update_gnzb_filesview(m_ptr_active_gnzb);
		m_ptr_mainwin->gnzb_updated(m_ptr_active_gnzb.get());
	}

	// format stat values
	char dl_speed_txt[32] = { '-', 0 }, remaining_time_txt[32] = { '-', 0, }, remaining_bytes_txt[32] = { '-', 0, };
	if(m_ptr_active_gnzb)
	{
		formatBytePerSecond(dl_speed_txt, dl_count, seconds);
		if(0 < Bps)
			formatTimeFromSeconds(remaining_time_txt, remaining_seconds);
		formatULongBytes(remaining_bytes_txt, remaining_bytes);
	}

	// update the info bar
	NzbDlInfoBar& info_bar = m_ptr_mainwin->info_bar();
	info_bar.remaining_time().set_text(remaining_time_txt);
	info_bar.dl_speed().set_text(dl_speed_txt);
	info_bar.remaining_size().set_text(remaining_bytes_txt);
	m_ptr_mainwin->update_connection_info();

	return true;
}
