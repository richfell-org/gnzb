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
#ifndef NNTP_FETCH_HEADER__
#define NNTP_FETCH_HEADER__

#include <vector>
#include <list>
#include <stdexcept>
#include <libusenet/nntpclient.h>

// forward references
class PrefsNntpServer;
class GNzb;

namespace NntpFetch {

/**
 * NNTP errors besides error responses from NNTP server
 */
class NntpError : public std::runtime_error
{
public:

	NntpError(int err) : std::runtime_error("NNTP Error"), m_err_num(err) {}
	NntpError(const char *msg, int err) : std::runtime_error(std::string(msg)), m_err_num(err) {}
	~NntpError() throw() {}

	int error_num() const { return m_err_num; }

private:

	int m_err_num;
};

/**
 * Error response from NNTP server.
 */
class NntpServerError : public std::runtime_error
{
public:

	NntpServerError(NntpClient::Response& nntp_response)
	:   std::runtime_error("NNTP server Error"),
		m_nntp_response(nntp_response)
	{}
	~NntpServerError() throw() {}

	const NntpClient::Response& nntp_response() const { return m_nntp_response; }

private:

	NntpClient::Response m_nntp_response;
};

/**
 * Connection reset error
 */
class NntpConnectionReset : public std::runtime_error
{
public:

	NntpConnectionReset() : std::runtime_error("connection reset") {}
	~NntpConnectionReset() throw() {}
};

/**
 * 
 */
class ParseError : public std::runtime_error
{
public:

	ParseError(const char *msg) : std::runtime_error(msg) {}
	~ParseError() throw() {}
};

// forward references
class ArticlePool;

// pushing fetch messages into the message queue
void push_message(GNzb *p_gnzb, int i_file);
void push_message(GNzb *p_gnzb, int i_file, int i_segment);

// clear the fetch message queue
void clear_message_queue();

// test for empty message queue
bool is_message_queue_empty();

// get the list of article fetch pools
std::list<ArticlePool*>& get_fetch_pools();

// find an article fetch pool by an NNTP server
std::list<ArticlePool*>::iterator find_pool(const PrefsNntpServer& nntp_server);

// the total number of connections supported by the current pool list
int get_max_connections();

// get the number of tasks actively downloading an article
int get_active_connection_count();

// are any tasks actively downloading?
bool has_active_connection();

// stop and delete server pools in the list which are not present or
// are disabled int the given NNTP servers list
void purge_server_pools(const std::vector<PrefsNntpServer>& valid_servers);

// add a fetch pool or modify an existing fetch pool based on the
// given NNTP server list
void update_server_pools(const std::vector<PrefsNntpServer>& valid_servers);

// tasks in all pools abort their current active download and wait for
// the message queue to transition to empty before returning to the tasks
// normal run loop.
void abort_download();

// shutdown all server pool threads, this will cause
// the article tasks to abort any current download and
// terminate their task thread
void stop_server_pools();

}   // namespace NntpFetch

#endif  /* NNTP_FETCH_HEADER__ */