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
#ifndef __GNZB_MENU_ACTIONS_HEADER__
#define __GNZB_MENU_ACTIONS_HEADER__

#include "appmenu.h"

extern AppMenuAction ma_prefs;
extern AppMenuAction ma_help_about;
extern AppMenuAction ma_quit;

extern WinMenuAction ma_file_open;
extern WinMenuAction ma_file_pause_all;
extern WinMenuAction ma_file_resume_all;
extern WinMenuAction ma_file_cancel_all;
extern WinMenuAction ma_view_statistics;
extern WinMenuAction ma_view_details;
extern WinMenuAction ma_view_summary;

#endif  /* __GNZB_MENU_ACTIONS_HEADER__ */