#include "Core/Audio/Decoder.hpp"
#include "Core/Audio/Decoder_WAV.hpp"
#include "Core/Audio/Decoder_VorbisOGG.hpp"

namespace LuaSTG::Core::Audio
{
	bool IDecoder::create(StringView path, IDecoder** pp_decoder)
	{
		ScopeObject<IDecoder> p_decoder;

		try
		{
			p_decoder = new Decoder_WAV(path);
			*pp_decoder = p_decoder.get();
			return true;
		}
		catch (std::exception const& e)
		{
			spdlog::error("[core] {}", e.what());
		}

		try
		{
			p_decoder = new Decoder_VorbisOGG(path);
			*pp_decoder = p_decoder.get();
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
