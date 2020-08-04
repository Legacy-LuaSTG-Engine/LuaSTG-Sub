#include <string>
#include "E2DWaveDecoder.hpp"
#include "fcyIO/fcyBinaryHelper.h"

using namespace std;
using namespace Eyes2D;
using namespace Eyes2D::Sound;

class Wrapper : public Eyes2D::Sound::Stream {
private:
	fcyStream* data;
public:
	size_t Size() { return data->GetLength(); }
	size_t Read(uint8_t* dest, size_t count) {
		fLen read;
		data->ReadBytes(dest, count, &read);
		return (size_t)read;
	};
	bool Seek(int32_t step, SeekBase base) {
		switch (base)
		{
		case SeekBase::BEG:
			return (FCYERR_OK == data->SetPosition(FCYSEEKORIGIN_BEG, step));
		case SeekBase::CUR:
			return (FCYERR_OK == data->SetPosition(FCYSEEKORIGIN_CUR, step));
		case SeekBase::END:
			return (FCYERR_OK == data->SetPosition(FCYSEEKORIGIN_END, step));
		default:
			return false;
		}
	}
	bool IsEOF() { return (data->GetPosition() >= (data->GetLength() - 1u)); }
	size_t Tell() { return data->GetPosition(); }
public:
	Wrapper(fcyStream* src) : data(src) {}
};

WaveDecoder::WaveDecoder() {
	m_Data.Reset();
}

WaveDecoder::~WaveDecoder() {
	m_Data.Free();
}

bool WaveDecoder::DecodeFromStream(fcyStream* stream) {
	if (stream == nullptr) {
		return false;
		//throw E2DException(0, 0, L"Eyes2D::WaveDecoder::WaveDecoder", L"Invalid stream to read.");
	}
	stream->AddRef();
	stream->Lock();
	stream->SetPosition(FCYSEEKORIGIN_BEG, 0);

	{
		Wrapper src(stream);
		if (!Decode(SourceType::UNKOWN, &src, &m_Data)) {
			//清理
			stream->Unlock();
			stream->Release();
			return false;
			//throw E2DException(0, 0, L"Eyes2D::WaveDecoder::WaveDecoder", L"Failed to decode source data.");
		}
	}

	//清理
	stream->Unlock();
	stream->Release();
	return true;
}
