#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/utility/string_ref.hpp>

namespace t3o
{
namespace detail
{

class text_oserializer
{
	template<typename Serializable>
	static void process_output_data(const Serializable& input_data,
			uint8_t* output_data, std::size_t output_size)
	{
		boost::string_ref output_str(reinterpret_cast<char*>(output_data), output_size);
		using device_t = boost::iostreams::back_insert_device<boost::string_ref>;
		device_t inserter(output_str);
		boost::iostreams::stream<device_t> strm(inserter);
		boost::archive::text_oarchive archive(strm);
		archive << input_data;
		strm.flush();
	}
};

}
}
