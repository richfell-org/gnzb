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
#include "articlefetchtask.h"
#include "fetch.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>
#include "../gnzb.h"

namespace NntpFetch {

#define IS_EOT(nntpResponse) (0 == (nntpResponse).get_length())

/*
 * Opens the given file and makes it size length.
 *
 * returns: The file descriptor or -1 on error
 */
static int openOutputFile(const char *path, size_t size)
{
	int result = -1;

	/* open/create the file */
	result = open(path, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR|S_IRGRP|S_IWGRP|S_IROTH);

	/* make the file a certain size, if given in the size argument */
	if((-1 != result) && (0 < size))
	{
		struct stat statbuf;
		if((-1 != fstat(result, &statbuf)) && (size > statbuf.st_size))
		{
			const unsigned char b = 0;
			if(-1 != lseek(result, size - 1, SEEK_SET))
				write(result, &b, sizeof(b));
		}
	}

	return result;
}

ArticleTask::ArticleTask(NntpClient::ServerAddr& nntpServer, MoveQueue<NntpFetch::Msg>& msg_queue)
:   mServer(nntpServer),
	m_msg_queue(msg_queue),
	mb_quit(false),
	mb_abort(false),
	mb_downloading(false)
{
}

bool ArticleTask::openDb()
{
	bool result = true;

	try { m_db.open(".enzyBee.db"); }
	catch(const Sqlite3::Error& e)
	{
		std::cerr << "ArticleTask::openDb: " << e.what() << std::endl;
		result = false;
	}

	return result;
}

bool ArticleTask::checkDbForSegment(NZB::Segment *pSegment)
{
	try
	{
		return m_db.is_article_present(pSegment);
	}
	catch(const Sqlite3::Error& e)
	{
		std::cerr << "ArticleTask::checkDbForSegment: " << e.what() << std::endl;
	}

	return false;
}

void ArticleTask::dbInsertSegment(NZB::Segment *pSegment, unsigned long position, unsigned long size)
{
	try	{ m_db.insert_article_record(pSegment, position, size); }
	catch(const Sqlite3::Error& e)
	{
		std::cerr << "ArticleTask::dbInsertSegment: " << e.what() << std::endl;
	}
}

void ArticleTask::updateNzb(GNzb *p_gnzb, int i_file, unsigned long bytes)
{
	std::lock_guard<std::mutex> update_lock(p_gnzb->update_mutex());

	auto& file_meta = p_gnzb->files_meta()[i_file];
	auto& dl_data = p_gnzb->download_data();
	file_meta.update_download_data(bytes, 1);
	file_meta.state(p_gnzb->nzb_files()[i_file].getSegmentCount() == file_meta.completed_segments()
		? GNzbState::COMPLETE
		: GNzbState::DOWNLOADING);
	file_meta.updated(true);
	dl_data.update_download_size(bytes);
}

void ArticleTask::run()
{
	NntpFetch::Msg qMsg;

	while(!mb_quit)
	{
		// flags set for aborting current download?
		if(mb_abort)
		{
			// close the NNTP and DB connactions
			if(m_ptr_conn) closeServerConnection();
			if(m_db.isOpen()) m_db.close();

			// wait for the queue to empty before continuing
			// with the run loop 
			while(!m_msg_queue.empty())
				std::this_thread::sleep_for(std::chrono::microseconds(250));

			// reset the abort flag and continue with run loop
			mb_abort = false;
			continue;
		}

		// check for a message in the queue
		if(!m_msg_queue.try_remove(qMsg, 250))
		{
			// no more messages, close connections if needed
			if(m_ptr_conn) closeServerConnection();
			if(m_db.isOpen()) closeDb();

			// continue with run loop
			continue;
		}

		// check message type 
		if(NntpFetch::MsgType::QUIT == qMsg.type())
		{
			// re-queue quit message for other (if any) threads
			m_msg_queue.insert(qMsg);
			break;
		}
		else if(NntpFetch::MsgType::NZB_SEGMENT == qMsg.type())
		{
			NZB::Segment *p_segment = &qMsg.gnzb_ptr()->nzb_files()[qMsg.file_idx()].getSegment(qMsg.segment_idx());

			try
			{
				if(!m_db.isOpen()) openDb();

				if(checkDbForSegment(p_segment))
					continue;

				if(!m_ptr_conn)
				{
					try { openServerConnection(); }
					catch(const std::exception& e)
					{
						std::cerr
							<< "ArticleTask::run: can't open a connection to "
							<< mServer.get_canon_name()
							<< std::endl;
						return;
					}
				}

				for(int tries = 3; 0 < tries; --tries)
				{
					try
					{
						mb_downloading = true;
						if(0 < fetchNzbSegment(p_segment))
							updateNzb(qMsg.gnzb_ptr(), qMsg.file_idx(), p_segment->getByteCount());
						else if(mb_quit)
							m_msg_queue.insert_front(qMsg);
					}
					catch(const NntpConnectionReset& e)
					{
						// re-connect to the NNTP server and keep trying
						try { openServerConnection(); }
						catch(const std::exception& e)
						{
							std::cerr
								<< "ArticleTask::run: can't re-open a connection to "
								<< mServer.get_canon_name()
								<< std::endl;
							return;
						}
						continue;
					}

					break;
				}
			}
			catch(const NntpError& e)
			{
				std::cerr << "ArticleTask::run: " << e.what() << std::endl;
			}
			catch(const NntpServerError& e)
			{
				std::cerr << "ArticleTask::run: ";
				const NntpClient::Response& response = e.nntp_response();
				if(0 < response.get_length())
					std::cerr.write(response.get_buffer(), response.get_length()) << std::endl;
				else
					std::cerr << e.what() << std::endl;
			}
			catch(const ParseError& e)
			{
				std::cerr
					<< "ArticleTask::run: (ParseError) id: "
					<< p_segment->getMessageId() << ' '
					<< e.what()
					<< std::endl;
			}
			catch(const std::runtime_error& e)
			{
				std::cerr << "ArticleTask::run: (runtimne_error) " << e.what() << std::endl;
			}

			// reset the downloading flag
			mb_downloading = false;
		}
	}

	closeServerConnection();
}

void ArticleTask::openServerConnection()
{
	if(m_ptr_conn)
		closeServerConnection(false);

	allocate_connection();

	try
	{
		NntpClient::Response response;
		m_ptr_conn->open(mServer, response);
		if(response.get_status() != NntpClient::CMD_OK)
		{
			std::cerr << mServer.get_canon_name() << ": ";
			std::cerr.write(response.get_buffer(), response.get_length());
			throw NntpServerError(response);
		}
	}
	catch(const std::system_error& e)
	{
		std::cerr << "ArticleTask::openServerConnection ("
			<< mServer.get_canon_name() << "): " << e.code().message() << std::endl;
		throw NntpError(e.code().message().c_str(), e.code().value());
	}
}

void ArticleTask::closeServerConnection(bool issueQuit/* = true*/)
{
	if(m_ptr_conn)
	{
		if(issueQuit && bool(*m_ptr_conn))
		{
			NntpClient::Response response;
			try
			{
				m_ptr_conn->send("QUIT");
				m_ptr_conn->read_response(response);
			}
			catch(const std::system_error& e)
			{
				std::cerr
					<< "ArticleTask::closeServerConnection ("
					<< mServer.get_canon_name() << "): "
					<< e.code().message()
					<< std::endl;
			}

			m_ptr_conn->close();
		}
	}

	m_ptr_conn.reset(0);
}

void ArticleTask::allocate_connection()
{
	m_ptr_conn.reset(new NntpClient::Connection);
}

int ArticleTask::parseToFileInfo(yEnc::Decoder& yDecoder)
{
	int result = 0;
	int waitForPart = 0;

	int linelen = 0;
	char linebuf[1024];

	int decodelen;
	unsigned char decodeBuf[512];
	try
	{
		while(0 < (linelen = m_ptr_conn->read_line(linebuf, 1024)))
		{
			result += linelen;

			// if we start hitting yEnc (or some other) data then we have a missing yEnc header - no good
			decodelen = 0;
			if(yEnc::DecodeResult::YENC_NONE != yDecoder.decode(decodeBuf, &decodelen, linebuf, linelen - 2))
				throw ParseError("ArticleTask::parseToFileInfo: Not a yEnc article");

			// if the main header was decoded see if we need to wait for the part header
			// if no part header then we are done with parsing the yEnc headers 
			if((0 == waitForPart) && yDecoder.is_header())
			{
				/* if no part number is given in the header then we're done */
				waitForPart = (0 < yDecoder.get_header_part());
				if(0 == waitForPart)
					return result;
			}

			// else we have previously discovered the need to wait for a yEnc part header
			// and we can return after parsing the part header
			else if((1 == waitForPart) && yDecoder.is_part())
				return result;
		}
	}
	catch(const std::system_error& e)
	{
		closeServerConnection(false);
		throw NntpConnectionReset();
	}

	/* else we failed to get yenc headers */
	throw ParseError("ArticleTask::parseToFileInfo: failed to parse yEnc headers");
}

unsigned long ArticleTask::fetchNzbSegment(NZB::Segment *pNzbSegment)
{
	unsigned long result = 0;
	int outFd = -1;
	NntpClient::Response nntpResponse;

	errno = 0;

	// request the article body for this segment
	try { m_ptr_conn->body(pNzbSegment->getMessageId().c_str(), nntpResponse); }
	catch(const std::system_error& e)
	{
		throw NntpServerError(nntpResponse);
	}

	// Will the text response follow?
	if(NntpClient::CMD_OK != nntpResponse.get_status())
		return 0;

	// reset Yenc decode data
	yEnc::Decoder yDecoder;

	// need to parse up to the ybegin/ypart lines to get filename and part offset
	try { result += parseToFileInfo(yDecoder); }
	catch(const ParseError& e)
	{
		register int rdsz = 0;
		char linebuf[1024];
		do
		{
			try { rdsz = m_ptr_conn->read_line(linebuf, 1024); }
			catch(...)
			{
				closeServerConnection();
				break;
			}
		} while(0 < rdsz);
		throw e;
	}

	// open/create the output file is needed
	if(-1 == (outFd = openOutputFile(yDecoder.get_file_name().c_str(), /*yencDecodeData.headSize*/-1)))
	{
		std::cerr << "create/open: " << strerror(errno) << std::endl;
		closeServerConnection();
		throw std::runtime_error(strerror(errno));
	}

	// read and parse the Yenc data, once the Yenc trailer line is read
	// the rest of the text lines from the artice are ignored
	int bufCountWriteTrigger = (1024*1024) - yDecoder.get_lines();
	int bufByteCount = 0;
	int rdResult = 0;
	lseek(outFd, yDecoder.get_part_begin() - 1, SEEK_SET);
	try
	{
		int rdlen = 0;
		char linebuf[1024];

		while(!mb_quit && !mb_abort && (0 < (rdlen = m_ptr_conn->read_line(linebuf, 1024))))
		{
			result += rdlen;
			if(!yDecoder.is_trailer())
			{
				yDecoder.decode(&m_io_buf[bufByteCount], &bufByteCount, linebuf, rdlen - 2);
				if(bufByteCount >= bufCountWriteTrigger)
				{
					write(outFd, &m_io_buf[0], bufByteCount);
					bufByteCount = 0;
				}
			}
		}
	}
	catch(const std::system_error& e)
	{
		rdResult = e.code().value();
	}

	if(0 < bufByteCount)
		write(outFd, &m_io_buf[0], bufByteCount);

	/* close the output file */
	if(0 <= outFd)
		close(outFd);

	// connection reset? try again if so
	if(ECONNRESET == rdResult)
	{
		closeServerConnection(false);
		throw NntpConnectionReset();
	}

	if(yDecoder.is_trailer())
		dbInsertSegment(pNzbSegment, yDecoder.get_part_begin() - 1, yDecoder.get_trailer_size());
	else if(mb_abort || mb_quit)
		result = 0;

	return result;
}

#ifdef LIBUSENET_USE_SSL

void SslArticleTask::allocate_connection()
{
	m_ptr_conn.reset(new NntpClient::SslConnection);
}

#endif  /* LIBUSENET_USE_SSL */

}   // namespace nntpFetch