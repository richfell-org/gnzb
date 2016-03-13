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
#ifndef __GNZB_HEADER__
#define __GNZB_HEADER__

#include <vector>
#include <string>
#include <mutex>
#include <libusenet/nzb.h>
#include <libusenet/binParts.h>
#include "db/nzbdb.h"

// state of an NZB in the tree view and an NZB file in the file tree view
enum class GNzbState { WAITING, PAUSED, STOPPED, DOWNLOADING, COMPLETE, CONDITIONAL, STATE_COUNT, };

/*
 * Gets the display name for a State value.
 */
const std::string& get_gnzb_state_display_name(GNzbState s);

class GNzb;
class GNzbGroup;

/*
 *
 */
class CollectionMeta
{
// construction/destruction
public:

	CollectionMeta() {}
	CollectionMeta(CollectionMeta&& that);
	CollectionMeta(const CollectionMeta& that);
	~CollectionMeta() {}

// attributes
public:

	int type() const { return m_type; }
	CollectionMeta& type(int type) { m_type = type; return *this; }

	unsigned long segment_count() const { return m_total_segments; }
	CollectionMeta& segment_count(unsigned long count) { m_total_segments = count; return *this; }

	unsigned long total_size() const { return m_total_size; }
	CollectionMeta& total_size(unsigned long size) { m_total_size = size; return *this; }

// operations
public:

	CollectionMeta& inc_segment_count(unsigned long count) { m_total_segments += count; return *this; }
	CollectionMeta& inc_total_size(unsigned long size) { m_total_size += size; return *this; }

	CollectionMeta& operator =(CollectionMeta&& that);
	CollectionMeta& operator =(const CollectionMeta& that);

// implementation
private:

	int m_type{BINPARTS_NONE};
	unsigned long m_total_segments{0};
	unsigned long m_total_size{0};
};

/*
 *
 */
class FileMeta
{
public:

	FileMeta() {}
	FileMeta(FileMeta&& that);
	FileMeta(const FileMeta& that);
	~FileMeta() {}

// attributes
public:

	// the NZB::BinParts type for the associated NZB::File
	int type() const { return m_type; }
	FileMeta& type(int type) { m_type = type; return *this; }

	// the download state of the associated NZB::File
	GNzbState state() const { return m_state; }
	FileMeta& state(GNzbState s) { m_state = s; return *this; }

	// the total size of the associated NZB::File
	unsigned long total_size() const { return m_total_size; }
	FileMeta& total_size(unsigned long size) { m_total_size = size; return *this; }

	// the size currently downloaded
	unsigned long download_size() const { return m_dl_size; }
	FileMeta& download_size(unsigned long size) { m_dl_size = size; return *this; }

	// the number of NZB::Segments (NNTP articles) completely downloaded
	int completed_segments() const { return m_complete_segments; }
	FileMeta& completed_segments(int count) { m_complete_segments = count; return *this; }

	// read/set the updated flag
	bool updated() const { return m_updated; }
	FileMeta& updated(bool update_state) { m_updated = update_state; return *this; }

// operations
public:

	// NZB::BinParts type checking
	bool is_type(int type) const { return m_type == type; }
	bool is_one_of_type(std::initializer_list<int> types) const;

	// State value checking
	bool is_state(GNzbState s) const { return m_state == s; }
	bool is_one_of_state(std::initializer_list<GNzbState> states) const;
	
	// add/subtract the current download size value
	FileMeta& update_download_size(long size) { m_dl_size += size; return *this; }

	// add/subtract the current downloaded segment count
	FileMeta& update_segment_count(int amount = 1) { m_complete_segments += amount; return *this; }

	// update all download data
	FileMeta& update_download_data(long size, int segment_count);
	FileMeta& update_download_data(unsigned long size, int segment_count);

	FileMeta& operator =(FileMeta&& that);
	FileMeta& operator =(const FileMeta& that);

// implementation
private:

	int m_type{BINPARTS_NONE};
	GNzbState m_state{GNzbState::WAITING};
	unsigned long m_total_size{0};
	unsigned long m_dl_size{0};
	int m_complete_segments{0};
	bool m_updated{false};
};

inline bool FileMeta::is_one_of_type(std::initializer_list<int> types) const
{
	for(int t : types)
		if(t == m_type) return true;
	return false;
}

inline bool FileMeta::is_one_of_state(std::initializer_list<GNzbState> states) const
{
	// return true if there is a match in the list
	for(GNzbState s : states)
		if(s == m_state) return true;

	// otherwise false
	return false;
}

inline FileMeta& FileMeta::update_download_data(long size, int segment_count)
{
	m_dl_size += size;
	m_complete_segments += segment_count;
	return *this;
}

inline FileMeta& FileMeta::update_download_data(unsigned long size, int segment_count)
{
	m_dl_size += size;
	m_complete_segments += segment_count;
	return *this;
}

/*
 * 
 */
class DownloadData
{
// construction/destruction
public:

	DownloadData()  {}
	DownloadData(DownloadData&& that);
	DownloadData(const DownloadData& that);
	~DownloadData() {}

// attributes
public:

	// get/set the download start time this is used by the callback
	// routine in its DL speed calculations.  It is reset to the
	// current time each time a download begins
	time_t start_time() const { return m_start_time; }
	DownloadData& start_time(time_t& t) { m_start_time = t; return *this; }

	// get/set the total size
	unsigned long size() const { return m_size; }
	DownloadData& size(unsigned long size) { m_size = size; return *this; }

	// the current downloaded size
	unsigned long current_size() const { return m_current; }
	DownloadData& current_size(unsigned long size) { m_current = size; return *this; }

	// the current size downloaded "this time", this is used
	// by the callback routine in its DL speed calculations.
	// Usually this value and "get_current" are the same but
	// will differ if the download was paused/stopped and then
	// resumed.  In that case this value holds the number of
	// bytes downloaded since the resumption
	unsigned long session_bytes() const { return m_session_bytes; }
	DownloadData& sesstion_bytes(unsigned long bytes) { m_session_bytes = bytes; return *this; }

	// get/set the peak download rate
	double peak_rate() const { return m_peak_rate; }
	DownloadData& peak_rate(double rate) { m_peak_rate = rate; return *this; }

// operations
public:

	// set the download start time to the current time
	DownloadData& mark_start_time();

	// resets the per-download session items
	DownloadData& reset_session() { m_session_bytes = 0; mark_start_time(); return *this; }

	// increment/decrement download size, updates both the session
	// size and the total size, callers beware...no bounds checking
	DownloadData& update_download_size(long amount) { m_current += amount; m_session_bytes += amount; return *this; }

	// download completeness
	bool is_complete() const { return m_current >= m_size; }
	unsigned long get_remaining() const { return m_size - m_current; }
	double get_fraction_complete() const { return (double)m_current / (double)m_size; }
	double get_percentage_complete() const { return get_fraction_complete() * 100.0; }

	DownloadData& operator =(DownloadData&& that);
	DownloadData& operator =(const DownloadData& that);

// implementation
private:

	time_t m_start_time{0};
	unsigned long m_size{0};
	unsigned long m_current{0};
	unsigned long m_session_bytes{0};
	double m_peak_rate{0.0};
};

/**
 * 
 * 
 */
class GNzb
{
// construction/destruction
public:

	GNzb();
	GNzb(const std::string& path);
	~GNzb();

// attributes
public:

	const std::string& path() const { return m_fq_path; }

	const std::string& output_path() const { return m_output_path; }
	GNzb& output_path(const std::string& path) { m_output_path.assign(path); return *this; }

	NZB::FileCollection& nzb_files() { return m_files; }
	const NZB::FileCollection& nzb_files() const { return m_files; }

	CollectionMeta& collection_meta() { return m_collection_meta; }
	const CollectionMeta& collection_meta() const { return m_collection_meta; }
	GNzb& collection_meta(CollectionMeta&& meta_data) { m_collection_meta = meta_data; return *this; }
	GNzb& collection_meta(const CollectionMeta& meta_data) { m_collection_meta = meta_data; return *this; }

	std::vector<FileMeta>& files_meta() { return m_files_meta; }
	const std::vector<FileMeta>& files_meta() const { return m_files_meta; }
	GNzb& files_meta(std::vector<FileMeta>&& files_meta) { m_files_meta = std::move(files_meta); return *this; }
	GNzb& files_meta(const std::vector<FileMeta>& files_meta) { m_files_meta = files_meta; return *this; }

	NzbDb& download_db() { return m_db; }
	const NzbDb& download_db() const { return m_db; }

	GNzbGroup *ptr_group() { return mp_group; }
	const GNzbGroup *ptr_group() const { return mp_group; }
	GNzb& ptr_group(GNzbGroup *p_group) { mp_group = p_group; return *this; }

	GNzbState state() const { return m_state; }
	GNzb& state(GNzbState state) { m_state = state; return *this; }

	DownloadData& download_data() { return m_dl_data; }
	const DownloadData& download_data() const { return m_dl_data; }
	GNzb& download_data(DownloadData&& data) { m_dl_data = data; return *this; }
	GNzb& download_data(const DownloadData& data) { m_dl_data = data; return *this; }

	std::mutex& update_mutex() { return m_mutex; }
	const std::mutex& update_mutex() const { return m_mutex; }

// operations
public:

	// parse an NZB file from the file system
	void parse_nzb_file(const std::string& fq_path);

	// check the current state value
	bool is_state(GNzbState s) const { return m_state == s; }
	bool not_state(GNzbState s) const { return m_state != s; }
	bool not_state(std::initializer_list<GNzbState> states) const;

// implementation
private:

	void generate_output_path();
	void parse_file();
	void calc_metadata();
	void calc_total_download_size(bool include_repair_files = false);
	void calc_download_status();

	// path info
	std::string m_fq_path;
	std::string m_output_path;

	// NZB files and meta-data
	NZB::FileCollection m_files;
	CollectionMeta m_collection_meta;
	std::vector<FileMeta> m_files_meta;

	// download database for this GNzb
	NzbDb m_db;

	GNzbGroup *mp_group{nullptr};

	GNzbState m_state;

	DownloadData m_dl_data;

	std::mutex m_mutex;
};

#endif  /* __GNZB_HEADER__ */