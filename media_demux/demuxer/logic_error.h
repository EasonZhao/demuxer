// LogicError.h

#ifndef LOGIC_ERROR_H_
#define LOGIC_ERROR_H_

#define BOOST_SYSTEM_NO_DEPRECATED

#include <boost/system/error_code.hpp>

namespace boost 
{
	namespace demuxer
	{
		const boost::system::error_category & get_demuxer_error_category() BOOST_SYSTEM_NOEXCEPT;
		static const boost::system::error_category & demuxer_error_category = get_demuxer_error_category();

		enum error
		{
			success = 123,
			format_error,
			no_more_tag,
			script_tag_not_support
		};
	}

	namespace system
	{
		template<> struct is_error_code_enum<boost::demuxer::error>
		{ static const bool value = true; };
	}

	namespace demuxer
	{
		inline boost::system::error_code make_error_code(error e)
		{ return boost::system::error_code(e, demuxer_error_category); }
	}
}

#endif


