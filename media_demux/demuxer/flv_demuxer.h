#ifndef FLV_DEMUXER_H
#define FLV_DEMUXER_H

#include <string>
#include <fstream>
#include <vector>
#include <boost/system/error_code.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "demuxer.h"

struct flv_tag_header {
	enum tag_type {
		AUDIO_TAG = 8,
		VIDEO_TAG = 9,
		SCRIPT_TAG = 18,
		OTHER_TAG
	};
	tag_type type;
	boost::uint32_t data_len;
	boost::uint64_t timestamp;
	boost::uint32_t streamID;
};

struct flv_script_tag {
	flv_tag_header header;
	const boost::uint8_t *data;
	std::vector<double> key_poss;
	std::vector<double> time_poss;
	boost::uint32_t height;
	boost::uint32_t width;
	boost::uint32_t duraion;
	void fetch();
};

class flv_demuxer : public demuxer
{
public:
	flv_demuxer(const std::string &file_name);

	boost::system::error_code read_sample(Sample &sample);

	boost::uint64_t duration();

private:
	boost::system::error_code fetch_head();

	boost::system::error_code fetch_script_tag();

	boost::system::error_code read_tag(flv_tag_header &tag);

private:
	boost::uint8_t version_;
	boost::uint32_t header_size_;
	bool video_enable_;
	bool audio_enable_;
	std::vector<boost::uint32_t> tag_offsets_;
	boost::interprocess::file_mapping fm_;
	boost::interprocess::mapped_region region_;
	boost::uint32_t cur_sample_index_;
};

#endif


