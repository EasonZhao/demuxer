#include <iostream>
#include "demuxer_factory.h"

using namespace std;

int main(int argc, char**argv)
{
	if (argc < 2) {
		cout << "error param! " << endl;
		return -1;
	}
	string file = argv[1];
	demuxer_ptr dmx = demuxer_factory::get_mutable_instance().create_demuxer(FLV_DEMUXER, file);
	Sample sample;
	boost::system::error_code ec;
	while (!ec) {
		ec = dmx->read_sample(sample);
	}
	return 0;
}