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
#ifndef __NNTP_ARTICLE_FETCH_TASK_HEADER__
#define __NNTP_ARTICLE_FETCH_TASK_HEADER__

#include <atomic>
#include <memory>
#include <libusenet/nzb.h>
#include <libusenet/yenc.h>
#include <libusenet/nntpclient.h>
#include "../msgqueue.h"
#include "../db/nzbdb.h"
#include "fetchmsg.h"

namespace NntpFetch {

/*
 * Standard, non-encrypted network communication fetch task. 
 */
class ArticleTask
{
public:

	ArticleTask(NntpClient::ServerAddr& nntpServer, MsgQueue<NntpFetch::Msg>& msg_queue);
	ArticleTask(const ArticleTask&) = delete;
	virtual ~ArticleTask() { closeDb(); }

// attributes
public:

	bool isDownloading() const { return mb_downloading; }

// operations
public:

	void run();

	void request_quit()  { mb_quit = true; }
	void request_abort() { mb_abort = true; }
	void reset_abort()   { mb_abort = false; }

	ArticleTask& operator =(const ArticleTask&) = delete;

protected:

	void openServerConnection();
	void closeServerConnection(bool issueQuit = true);
	int parseToFileInfo(yEnc::Decoder& yDecode);
	unsigned long fetchNzbSegment(NZB::Segment *pNzbSegment);

	bool openDb();
	void closeDb() { m_db.close(); }
	bool checkDbForSegment(NZB::Segment *pSegment);
	void dbInsertSegment(NZB::Segment *pSegment, unsigned long position, unsigned long size);

	void updateNzb(GNzb *pNzb, int i_file, unsigned long bytes);

	virtual void allocate_connection();

	NntpClient::ServerAddr& mServer;
	std::unique_ptr<NntpClient::Connection> m_ptr_conn;

	MsgQueue<NntpFetch::Msg>& m_msg_queue;

	NzbDb m_db;

	std::atomic<bool> mb_quit;
	std::atomic<bool> mb_abort;
	std::atomic<bool> mb_downloading;

	unsigned char m_io_buf[1024 * 1024];
};

#ifdef LIBUSENET_USE_SSL
/*
 * Encrypted netowrk communication fetch task
 */
class SslArticleTask : public ArticleTask
{
// construction
public:

	using ArticleTask::ArticleTask;

// implementation
protected:

	void allocate_connection();
};

#endif  /* LIBUSENET_USE_SSL */

}   // namespace NntpFetch

#endif  /* __NNTP_ARTICLE_FETCH_TASK_HEADER__ */