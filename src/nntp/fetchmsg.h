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
#ifndef __NNTP_FETCH_MSG_HEADER__
#define __NNTP_FETCH_MSG_HEADER__

// forward references
class GNzb;

namespace NntpFetch {

// message types
enum class MsgType { NONE, QUIT, NZB_FILE, NZB_SEGMENT, };

class Msg
{
public:
	Msg() : Msg(MsgType::NONE) {}
	Msg(MsgType t);
	Msg(GNzb *pNzb, int i_file);
	Msg(GNzb *pNzb, int i_file, int i_segment);
	Msg(const Msg& that);
	Msg(Msg&&) = default;

// attributes
public:

	MsgType type() const { return m_type; }
	Msg& type(const MsgType t) { m_type = t; return *this; }

	GNzb *gnzb_ptr() { return mp_gnzb; }
	const GNzb *gnzb_ptr() const { return mp_gnzb; }
	Msg& gnzb_ptr(GNzb *p_gnzb) { mp_gnzb = p_gnzb; return *this; }

	int file_idx() const { return m_idx_file; }
	Msg& file_idx(const int i_file) { m_idx_file = i_file; return *this; }

	int segment_idx() const { return m_idx_segment; }
	Msg& segment_idx(const int i_segment) { m_idx_segment = i_segment; return *this; }

// operations
public:

	Msg& operator =(const Msg& that);
	Msg& operator =(Msg&&) = default;

// implementation
private:

	MsgType m_type;
	GNzb *mp_gnzb;
	int m_idx_file, m_idx_segment;
};

}   // namespace NntpFetch

#endif  /* __NNTP_FETCH_MSG_HEADER__ */