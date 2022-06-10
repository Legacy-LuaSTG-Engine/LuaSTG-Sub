#include "Core/Audio/Decoder.hpp"
#include "Core/Audio/Decoder_WAV.hpp"
#include "Core/Audio/Decoder_VorbisOGG.hpp"

namespace LuaSTG::Core::Audio
{
	bool IDecoder::create(StringView path, IDecoder** pp_decoder)
	{
		try
		{
			*pp_decoder = new Decoder_WAV(path);
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
		}
		try
		{
			*pp_decoder = new Decoder_VorbisOGG(path);
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
		}
		*pp_decoder = nullptr;
		return false;
	}
}
