#ifndef DEMUXER_MP4_HPP
#define DEMUXER_MP4_HPP

#include <string>
#include "Ap4.h"

using namespace bento4;

int detect_mp4(const std::string &file) {
	AP4_ByteStream *stream = NULL;
	//open file
	AP4_Result ret = AP4_FileByteStream::Create(file.c_str(), AP4_FileByteStream::STREAM_MODE_READ, stream);
	if ( AP4_FAILED(ret) ) {
		return -1;
	}
	AP4_File container(*stream);
	AP4_MetaData media_data(&container);
	return 0;
}
#endif