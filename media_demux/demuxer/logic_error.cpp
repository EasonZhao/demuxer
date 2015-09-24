#include "logic_error.h"

namespace boost 
{
	namespace demuxer
	{
		class demuer_error_category_imp : public boost::system::error_category
		{
		public:
			demuer_error_category_imp() : boost::system::error_category() {}

			const char* name() const BOOST_SYSTEM_NOEXCEPT
			{
				return "demuxer";
			}

			boost::system::error_condition default_error_condition( int ev ) const BOOST_SYSTEM_NOEXCEPT
			{
				return ev == success
					? boost::system::error_condition( boost::system::errc::io_error,
					boost::system::generic_category() ) 
					: boost::system::error_condition( ev, boost::demuxer::demuxer_error_category);
			}

			std::string message( int ev) const
			{
				std::string msg("unknown error");
				switch(ev) {
				case success:
					msg = "success";
					break;
				case format_error:
					msg = "format error";
					break;
				case no_more_tag:
					msg = "no more tag in flv";
					break;
				case script_tag_not_support:
					msg = "flv script tag format not support";
					break;
				default:
					break;
				}
				return msg;
			}
			
		};

		const boost::system::error_category & get_demuxer_error_category() BOOST_SYSTEM_NOEXCEPT
		{
			static const demuer_error_category_imp imp;
			return imp;
		}
	}
}