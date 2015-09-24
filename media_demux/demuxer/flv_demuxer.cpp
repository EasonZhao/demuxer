#include "flv_demuxer.h"
#include <boost/assert.hpp>
#include <boost/spirit/home/support/detail/endian/endian.hpp>

template<typename T>
T make_tag(const char* p)
{
	T tag;
	switch (*p) {
	case 8:
		tag.header.type = flv_tag_header::AUDIO_TAG;
		break;
	case 9:
		tag.header.type = flv_tag_header::VIDEO_TAG;
		break;
	case 18:
		tag.header.type = flv_tag_header::SCRIPT_TAG;
		break;
	default:
		tag.header.type = flv_tag_header::OTHER_TAG;
		break;
	}
	p++;
	tag.header.data_len = boost::spirit::detail::load_big_endian<boost::uint32_t, 3>(p);
	p += 3;
	tag.header.timestamp = boost::spirit::detail::load_big_endian<boost::uint64_t, 4>(p);
	p += 4;
	tag.header.streamID = boost::spirit::detail::load_big_endian<boost::uint64_t, 3>(p);
	p += 3;
	tag.data = (const boost::uint8_t*)p;
	tag.fetch();
	return tag;
}

flv_demuxer::flv_demuxer( const std::string &file_name ) 
	: demuxer()
	, fm_(file_name.c_str(), boost::interprocess::read_only)
	, region_(fm_, boost::interprocess::read_only)
	//, file_reader_(file_name.c_str(), std::ios::binary)
	, version_(0)
	, video_enable_(false)
	, audio_enable_(false)
	, header_size_(0)
	, cur_sample_index_(0)
{
	fetch_head();
	fetch_script_tag();
}

boost::uint64_t flv_demuxer::duration()
{
	return 0;
}

boost::system::error_code flv_demuxer::fetch_head()
{
	const char* buf = reinterpret_cast<const char*>(region_.get_address());
	boost::system::error_code ec = boost::demuxer::success;
	if (std::string(buf, 3) != "FLV" ) {
		ec = boost::demuxer::format_error;
	}
	version_ = buf[3];
	boost::uint8_t stream_tag = buf[4];
	boost::uint8_t tmp = stream_tag << 7;
	video_enable_ = (0x80 & tmp) != 0;

	tmp = stream_tag >> 1;
	tmp = tmp << 7;
	BOOST_ASSERT( (tmp|0x00) == 0 && " bit 2 can't zero");
	
	tmp = stream_tag >> 2;
	tmp = tmp << 7;
	audio_enable_ = (0x80 & tmp) != 0;

	tmp = stream_tag >> 3;
	tmp = tmp << 7;
	BOOST_ASSERT( (tmp|0x00) == 0 && " bit 4 can't zero");
	
	header_size_ = boost::spirit::detail::load_big_endian<boost::uint32_t, 4>(buf+5);

	//get tag offset
	boost::uint32_t tag_size = 0;
	const char* head_pos = buf + region_.get_size() - 4;
	do  {
		tag_size = boost::spirit::detail::load_big_endian<boost::uint32_t, 4>(head_pos);
		if (tag_size!=0)
			tag_offsets_.push_back(head_pos - tag_size - buf);
		head_pos -= tag_size;
		head_pos -= 4;
	} while (tag_size>0);
	std::reverse(tag_offsets_.begin(), tag_offsets_.end());
	return ec;
}

boost::system::error_code flv_demuxer::read_sample( Sample &sample )
{
	boost::system::error_code ec = boost::demuxer::success;
	flv_tag_header tag;
	ec = read_tag(tag);
	if (ec) {
		return ec;
	}
	//fill sample
	return ec;
}

boost::system::error_code flv_demuxer::read_tag( flv_tag_header &tag )
{
	boost::system::error_code ec = boost::demuxer::success;
	if ( cur_sample_index_ >= tag_offsets_.size() ) {
		ec = boost::demuxer::no_more_tag;
		return ec;
	}
	const char* buf = reinterpret_cast<const char*>(region_.get_address()) + tag_offsets_[cur_sample_index_];
	cur_sample_index_++;
	switch (*buf) {
	case 8:
		tag.type = flv_tag_header::AUDIO_TAG;
		break;
	case 9:
		tag.type = flv_tag_header::VIDEO_TAG;
		break;
	case 18:
		tag.type = flv_tag_header::SCRIPT_TAG;
		break;
	default:
		tag.type = flv_tag_header::OTHER_TAG;
		break;
	}
	buf++;
	tag.data_len = boost::spirit::detail::load_big_endian<boost::uint32_t, 3>(buf);
	buf +=3;
	tag.timestamp = boost::spirit::detail::load_big_endian<boost::uint64_t, 4>(buf);
	buf +=4;
	tag.streamID = boost::spirit::detail::load_big_endian<boost::uint64_t, 3>(buf);
	return ec;
}

boost::system::error_code flv_demuxer::fetch_script_tag()
{
	boost::system::error_code ec = boost::demuxer::success;
	const char* buf = reinterpret_cast<const char*>(region_.get_address()) + tag_offsets_[0];
	flv_script_tag tag = make_tag<flv_script_tag>(buf);
	tag_offsets_.erase(tag_offsets_.begin());
	return ec;
}

/*****************************************************************************************************/
template<typename T,int x>
T get_AMF_value(const boost::uint8_t *p) {
	return boost::spirit::detail::load_big_endian<T, x>(p);
}

std::string get_AMF_string(const boost::uint8_t *data)
{
	return std::string((const char*)data+2, 
		boost::spirit::detail::load_big_endian<boost::uint32_t, 2>(data));
}

void flv_script_tag::fetch()
{
	const boost::uint8_t *p = data;
	BOOST_ASSERT(*p == 0x02 && "script format error");
	p += 1;
	std::string meta_str = get_AMF_string(p);
	p += 2;
	p += meta_str.size();
	boost::uint32_t arr_size = 0;
	if (*p == 0x08) {
		p += 1;
		arr_size = boost::spirit::detail::load_big_endian<boost::uint32_t, 4>(p);
		p += 4;
	} else {
		BOOST_ASSERT(false && "fetch error");
	}
	for ( int i=0; i<arr_size; i++) {
		//get key
		std::string key = get_AMF_string(p);
		p += 2;
		p += key.size();
		//get value
		boost::uint8_t typ = *p;
		p += 1;
		if (typ == 0x00) {
			double value = boost::spirit::detail::load_big_endian<double, 8>(p);
			p += 8;
		} else if (typ == 0x01) {
			bool value = *p;
			p += 1;
		} else if (typ == 0x02) {
			std::string value = get_AMF_string(p);
			p += 2;
			p += value.size();
		} else if (typ == 0x03) {
			std::string name = get_AMF_string(p);
			p += 2;
			p += name.size();
			boost::uint8_t typ1 = *p;
			p += 1;
			BOOST_ASSERT(typ1 == 0x0a && "fetch error");
			boost::uint32_t pos_count = boost::spirit::detail::load_big_endian<boost::uint32_t, 4>(p);
			p += 4;
			for (int j=0; j<pos_count; j++) {
				BOOST_ASSERT(*p==0x00 && "fetch error");
				p += 1;
				double pos = boost::spirit::detail::load_big_endian<double, 8>(p);
				std::cout << (int)pos << std::endl;
				p += 8;
			}
			//
			std::string time_str = get_AMF_string(p);
			p += 2;
			p += time_str.size();
			typ1 = *p;
			p += 1;
			BOOST_ASSERT(typ1 == 0x0a && "fetch error");
			boost::uint32_t time_count = boost::spirit::detail::load_big_endian<boost::uint32_t, 4>(p);
			p += 4;
			for (int j=0; j<pos_count; j++) {
				BOOST_ASSERT(*p==0x00 && "fetch error");
				p += 1;
				double t_v = boost::spirit::detail::load_big_endian<double, 8>(p);
				std::cout << (int)t_v << std::endl;
				p += 8;
			}
			//仍然有6个byte未处理？？？
			boost::uint32_t tmp = p - data;
			
		} else {
			BOOST_ASSERT(false && "fetch error");
		}
	}
}


