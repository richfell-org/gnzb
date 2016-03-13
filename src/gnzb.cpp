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
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#include "gnzb.h"
#include "runtimesettings.h"
#include "db/gnzbgroups.h"
#include <libusenet/nzbparse.h>
#include <gtkmm/treemodel.h>
#include <time.h>

static const std::string state_text[int(GNzbState::STATE_COUNT)] = {
	std::string("Waiting for download"),
	std::string("Paused"),
	std::string("Stopped"),
	std::string("Downloading"),
	std::string("Done"),
	std::string("Conditional"),
};

static const std::string state_unknown("");

const std::string& get_gnzb_state_display_name(GNzbState s)
{
	int state_idx = int(s);
	if((state_idx >= 0) && (state_idx < int(GNzbState::STATE_COUNT)))
		return state_text[state_idx];
	return state_unknown;
}

/*
 * Calculate the total byte size for an NZB::File by totaling
 * the byte count of all its segments.
 */
static unsigned long get_byte_size(const NZB::File& nzbFile)
{
	unsigned long result = 0;
	for(int iSeg = 0; iSeg < nzbFile.getSegmentCount(); ++iSeg)
		result += nzbFile.getSegment(iSeg).getByteCount();
	return result;
}

CollectionMeta::CollectionMeta(CollectionMeta&& that)
:	m_type(that.m_type),
	m_total_segments(that.m_total_segments),
	m_total_size(that.m_total_size)
{
}

CollectionMeta::CollectionMeta(const CollectionMeta& that)
:	m_type(that.m_type),
	m_total_segments(that.m_total_segments),
	m_total_size(that.m_total_size)
{
}

CollectionMeta& CollectionMeta::operator =(CollectionMeta&& that)
{
	m_type = that.m_type;
	m_total_segments = that.m_total_segments;
	m_total_size = that.m_total_size;
	return *this;
}

CollectionMeta& CollectionMeta::operator =(const CollectionMeta& that)
{
	m_type = that.m_type;
	m_total_segments = that.m_total_segments;
	m_total_size = that.m_total_size;
	return *this;
}

FileMeta::FileMeta(FileMeta&& that)
:   m_type(that.m_type),
	m_state(that.m_state),
	m_total_size(that.m_total_size),
	m_dl_size(that.m_dl_size),
	m_complete_segments(that.m_complete_segments),
	m_updated(that.m_updated)
{
}

FileMeta::FileMeta(const FileMeta& that)
:   m_type(that.m_type),
	m_state(that.m_state),
	m_total_size(that.m_total_size),
	m_dl_size(that.m_dl_size),
	m_complete_segments(that.m_complete_segments),
	m_updated(that.m_updated)
{
}

FileMeta& FileMeta::operator =(FileMeta&& that)
{
	m_type = that.m_type;
	m_state = that.m_state;
	m_total_size = that.m_total_size;
	m_dl_size = that.m_dl_size;
	m_complete_segments = that.m_complete_segments;
	m_updated = that.m_updated;
	return *this;
}

FileMeta& FileMeta::operator =(const FileMeta& that)
{
	m_type = that.m_type;
	m_state = that.m_state;
	m_total_size = that.m_total_size;
	m_dl_size = that.m_dl_size;
	m_complete_segments = that.m_complete_segments;
	m_updated = that.m_updated;
	return *this;
}

DownloadData::DownloadData(DownloadData&& that)
:  	m_start_time(that.m_start_time),
	m_size(that.m_size),
	m_current(that.m_current),
	m_session_bytes(that.m_session_bytes),
	m_peak_rate(that.m_peak_rate)
{
}

DownloadData::DownloadData(const DownloadData& that)
:  	m_start_time(that.m_start_time),
	m_size(that.m_size),
	m_current(that.m_current),
	m_session_bytes(that.m_session_bytes),
	m_peak_rate(that.m_peak_rate)
{
}

DownloadData& DownloadData::mark_start_time()
{
	time(&m_start_time);
	return *this;
}

DownloadData& DownloadData::operator =(DownloadData&& that)
{
  	m_start_time = that.m_start_time;
	m_size = that.m_size;
	m_current = that.m_current;
	m_session_bytes = that.m_session_bytes;
	m_peak_rate = that.m_peak_rate;
	return *this;
}

DownloadData& DownloadData::operator =(const DownloadData& that)
{
  	m_start_time = that.m_start_time;
	m_size = that.m_size;
	m_current = that.m_current;
	m_session_bytes = that.m_session_bytes;
	m_peak_rate = that.m_peak_rate;
	return *this;
}

GNzb::GNzb()
:   m_fq_path(),
	m_output_path(),
	m_state(GNzbState::PAUSED)	
{
}

GNzb::GNzb(const std::string& path)
:   m_fq_path(path),
	m_output_path(),
	m_state(GNzbState::PAUSED)	
{
	generate_output_path();
	parse_file();
}

GNzb::~GNzb()
{
}

void GNzb::parse_nzb_file(const std::string& fq_path)
{
	m_fq_path.assign(fq_path);
	generate_output_path();
	parse_file();
}

void GNzb::generate_output_path()
{
	// start with the user defined base output directory
	m_output_path.assign(RuntimeSettings::locations().base_output_path());

	// crop out just the filename of the NZB
	std::string::size_type pos_name = m_fq_path.find_last_of('/');
	if(std::string::npos == pos_name)
		m_output_path.append(m_fq_path);
	else
		m_output_path.append(m_fq_path.substr(pos_name + 1));
}

void GNzb::parse_file()
{
	if(m_fq_path.empty()) return;

	// parse NZB file, store the NZB path and output path
	m_files = NZB::Parse::parseFile(m_fq_path.c_str());

	// generate the meta data
	m_files_meta.clear();
	m_files_meta.reserve(m_files.getFileCount());
	calc_metadata();

	// download calculations
	calc_total_download_size(true);
	calc_download_status();
}

bool GNzb::not_state(std::initializer_list<GNzbState> states) const
{
	// return false if this state is one of the given states
	for(auto s = states.begin(); s != states.end(); ++s)
		if(*s == m_state) return false;
	return true;
}

void GNzb::calc_metadata()
{
	m_collection_meta.segment_count(0).total_size(0);

	// count each file type contained in the NZB and use the type with
	// the maximum total to set the overall type that the NZB "produces"
	int fileTypeTotals[BINPARTS_COUNT] = { 0, };

	NZB::BinPartsOracle partsOracle;

	const int file_count = m_files.getFileCount();
	m_files_meta.resize(file_count);
	for(int iFile = 0; iFile < file_count; ++iFile)
	{
		NZB::File& nzb_file = m_files[iFile];

		// initialize the meta data for thie NZB::File
		m_files_meta[iFile].type(partsOracle.get_file_type(nzb_file.getSubject().c_str()));
		m_files_meta[iFile].state(
			(BINPARTS_REPAIR_BLOCKS == m_files_meta[iFile].type()) ? GNzbState::CONDITIONAL : GNzbState::WAITING);
		m_files_meta[iFile].total_size(0).download_size(0).completed_segments(0).updated(false);

		// determine the NZB::File's byte size
		unsigned long file_size = 0;
		int segmentCount = nzb_file.getSegmentCount();
		m_collection_meta.inc_segment_count(segmentCount);
		for(int iSeg = 0; iSeg < segmentCount; ++iSeg)
		{
			NZB::Segment& segment = nzb_file.getSegment(iSeg);
			file_size += segment.getByteCount();
		}
		m_files_meta[iFile].total_size(file_size);

		// increment the total size for the NZB::File collection
		m_collection_meta.inc_total_size(file_size);

		++fileTypeTotals[m_files_meta[iFile].type()];
	}

	// determine the overall type to set for this NZB
	int highTypeTotalType = 0;
	int highTypeTotal = fileTypeTotals[highTypeTotalType];
	for(int i = 0; i < BINPARTS_COUNT; ++i)
	{
		if(highTypeTotal < fileTypeTotals[i])
		{
			highTypeTotal = fileTypeTotals[i];
			highTypeTotalType = i;
		}
	}
	m_collection_meta.type(highTypeTotalType);
}

/*
 * 
 */
void GNzb::calc_total_download_size(bool include_repair_files/* = false*/)
{
	NZB::BinPartsOracle partsOracle;
	unsigned long size{0};

	for(int i = 0; i < m_files.getFileCount(); ++i)
	{
		switch(partsOracle.get_file_type(m_files[i].getSubject().c_str()))
		{
			default:
				size += get_byte_size(m_files[i]);
				break;
			case BINPARTS_REPAIR_BLOCKS:
				if(include_repair_files)
					size += get_byte_size(m_files[i]);
				break;
		}
	}

	m_dl_data.size(size);
}

void GNzb::calc_download_status()
{
	// check for a download DB in the output dir, if it does not exist
	// then there are no parts of this NZB which have been downloaded yet
	Glib::ustring db_file_name(Glib::ustring::compose("%1/.enzyBee.db", m_output_path));
	if(!NzbDb::does_db_exist(db_file_name.c_str())) return;

	m_db.close();
	m_db.open(db_file_name.c_str());

	for(int iFile = 0; iFile < m_files.getFileCount(); ++iFile)
	{
		NZB::File *pNzbFile = &m_files[iFile];
		FileMeta& file_meta = m_files_meta[iFile];

		int complete_segs;
		file_meta.download_size(m_db.get_size(pNzbFile, &complete_segs));
		if(0 < file_meta.download_size())
		{
			file_meta.completed_segments(complete_segs);
			m_dl_data.current_size(m_dl_data.current_size() + file_meta.download_size());
		}
	}

	m_db.close();
}
