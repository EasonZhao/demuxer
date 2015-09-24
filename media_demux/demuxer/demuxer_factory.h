#ifndef DEMUXER_FACTORY_H
#define DEMUXER_FACTORY_H

#include <string>
#include <boost/serialization/singleton.hpp>
#include <boost/shared_ptr.hpp>

#include "demuxer.h"

typedef boost::shared_ptr<demuxer> demuxer_ptr;

class demuxer_factory : public boost::serialization::singleton<demuxer_factory> {
public:
	demuxer_factory() {}
	
	demuxer_ptr create_demuxer( Demuxer_Type demuxer_type, const std::string &file_name);
};

#endif