#include "demuxer_factory.h"
#include "flv_demuxer.h"

demuxer_ptr demuxer_factory::create_demuxer( Demuxer_Type demuxer_type, const std::string &file_name )
{
	switch (demuxer_type) {
	case FLV_DEMUXER:
		return demuxer_ptr(new flv_demuxer(file_name));
		break;
	default:
		assert(false);
		break;
	}
	return demuxer_ptr();
}
