#ifndef DEMUXER_H
#define DEMUXER_H

#include <boost/cstdint.hpp>
#include "logic_error.h"

enum Demuxer_Type {
	FLV_DEMUXER = 0
};

typedef struct tag_Sample
{
	boost::uint32_t stream_index;     
	boost::uint32_t start_time;       
	boost::uint64_t offset_in_file;
	boost::uint32_t buffer_length;    
	boost::uint32_t duration;
	boost::uint32_t desc_index;
	boost::uint64_t decode_time;
	boost::uint32_t composite_time_delta;
	bool is_sync;
	boost::uint8_t const * buffer;    
} Sample;

class demuxer {
public:
	virtual ~demuxer() {}

	virtual boost::uint64_t duration() = 0;

	virtual boost::system::error_code read_sample(Sample &sample) = 0;
};


#endif