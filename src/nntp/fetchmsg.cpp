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
#include "fetchmsg.h"

namespace NntpFetch {

Msg::Msg(const Msg& that)
:   m_type(that.m_type),
	mp_gnzb(that.mp_gnzb),
	m_idx_file(that.m_idx_file),
	m_idx_segment(that.m_idx_segment)
{
}

Msg::Msg(MsgType t)
:   m_type(t), mp_gnzb(nullptr), m_idx_file(-1), m_idx_segment(-1)
{
}

Msg::Msg(GNzb *p_gnzb, int i_file)
:   m_type(MsgType::NZB_FILE),
	mp_gnzb(p_gnzb),
	m_idx_file(i_file),
	m_idx_segment(-1)
{
}

Msg::Msg(GNzb *p_gnzb, int i_file, int i_segment)
:   m_type(MsgType::NZB_SEGMENT),
	mp_gnzb(p_gnzb),
	m_idx_file(i_file),
	m_idx_segment(i_segment)
{
}

Msg& Msg::operator =(const Msg& that)
{
	m_type = that.m_type;
	mp_gnzb = that.mp_gnzb;
	m_idx_file = that.m_idx_file;
	m_idx_segment = that.m_idx_segment;
	return *this;
}

}   // namespace NntpFetch