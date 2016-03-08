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
#include "appmenu.h"

AppMenuAction ma_prefs("Preferences", "prefs", "<Control>s");
AppMenuAction ma_help_about("About", "about");
AppMenuAction ma_quit("Quit", "quit", "<Control>x");

WinMenuAction ma_file_open("Open...", "file_open", "<Control>f");
WinMenuAction ma_file_pause_all("Pause All", "pause_all", "<Control>p");
WinMenuAction ma_file_resume_all("Resume All Paused", "resume_all", "<Control>r");
WinMenuAction ma_file_cancel_all("Cancel All", "cancel_all", "<Control>x");
WinMenuAction ma_view_statistics("Statistics", "view_statistics", "<Control>t");
WinMenuAction ma_view_details("Details", "view_details", "<Control>d");
WinMenuAction ma_view_summary("Summary", "view_summary", "<Control>i");