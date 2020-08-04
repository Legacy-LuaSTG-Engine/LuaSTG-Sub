#include "E2DXAudio2SoundPlayer.hpp"
#include "E2DXAudio2Impl.hpp"
#include "E2DWaveDecoder.hpp"

#include "XFFT.h"

using namespace std;
using namespace Eyes2D;
using namespace Eyes2D::Sound;

XAudio2SoundPlayerImpl::XAudio2SoundPlayerImpl(fcyStream* stream, unsigned int mixerid) {
	m_FFTWorkset = nullptr;
	m_FFTOutComplex = nullptr;
	
	m_Status = AudioStatus::ERROR;//出错状态，还未准备好数据
	m_Decoder = nullptr;
	m_Voice = nullptr;

	m_PlayPos = 0;
	m_LoopStart = 0;
	m_LoopEnd = 0;
	m_LoopCount = 0;//默认不循环

	HRESULT hr;

	//检查混音音轨
	if (mixerid != 0) {
		IXAudio2SubmixVoice* mixer = GetInstance().GetMixerVoiceByID(mixerid);
		if (mixer == nullptr)
			throw E2DException(0, 0, L"Eyes2D::Sound::XAudio2SoundPlayerImpl::XAudio2SoundPlayerImpl", L"Invalid MixerVoice ID.");

	}

	//尝试解码数据
	try {
		WaveDecoder* decoder = new WaveDecoder();
		decoder->DecodeFromStream(stream);
		m_Decoder = decoder;
	}
	catch (E2DException & e) {
		throw E2DException(0, 0, L"Eyes2D::Sound::XAudio2SoundPlayerImpl::XAudio2SoundPlayerImpl", L"Failed to decode audio file.");
	}

	//音源数据格式
	WAVEFORMATEX waveformat;
	waveformat.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.nChannels = m_Decoder->GetChannels();
	waveformat.nSamplesPerSec = m_Decoder->GetSamplesPerSec();
	waveformat.nAvgBytesPerSec = m_Decoder->GetAvgBytesPerSec();
	waveformat.nBlockAlign = m_Decoder->GetBlockAlign();
	waveformat.wBitsPerSample = m_Decoder->GetBitsPerSample();
	waveformat.cbSize = 0;
	hr = GetInstance().GetXAudio2()->CreateSourceVoice(&m_Voice, &waveformat);
	if (FAILED(hr)) {
		delete m_Decoder; m_Decoder = nullptr;
		throw E2DException(0, hr, L"Eyes2D::Sound::XAudio2SoundPlayerImpl::XAudio2SoundPlayerImpl", L"Failed to create SourceVoice.");
	}

	//绑定到混音音轨
	if (mixerid != 0) {
		XAUDIO2_SEND_DESCRIPTOR sendunit;
		sendunit.Flags = 0;
		sendunit.pOutputVoice = m_Voice;
		XAUDIO2_VOICE_SENDS sendlist;
		sendlist.SendCount = 1;
		sendlist.pSends = &sendunit;
		hr = m_Voice->SetOutputVoices(&sendlist);
		if (FAILED(hr)) {
			m_Voice->DestroyVoice(); m_Voice = nullptr;
			delete m_Decoder; m_Decoder = nullptr;
			throw E2DException(0, hr, L"Eyes2D::Sound::XAudio2SoundPlayerImpl::XAudio2SoundPlayerImpl", L"Failed to bind SourceVoice to MixerVoice.");
		}
	}

	m_Status = AudioStatus::STOP;//这时候总算可以处于停止播放状态了
}

XAudio2SoundPlayerImpl::~XAudio2SoundPlayerImpl() {
	if (m_Voice != nullptr) {
		m_Voice->DestroyVoice();
	}
	if (m_Decoder != nullptr) {
		delete m_Decoder;
	}
}

bool XAudio2SoundPlayerImpl::pushSoundBuffer() {
	//准备描述结构
	XAUDIO2_BUFFER XAbuffer;
	ZeroMemory(&XAbuffer, sizeof(XAUDIO2_BUFFER));
	XAbuffer.AudioBytes = m_Decoder->GetDataSize();
	XAbuffer.pAudioData = m_Decoder->GetDataBuffer();
	//计算数据
	unsigned int totalsample = GetTotalTime();
	unsigned int playlength = totalsample - m_PlayPos;
	if (playlength <= 0) {
		return false;//不需要填充数据
	}
	//填充数据
	if (m_LoopStart != 0 && m_LoopEnd != 0 && m_LoopCount != 0) {
		if (m_PlayPos < m_LoopStart) {
			//播放位置位于循环节前
			XAbuffer.Flags = XAUDIO2_END_OF_STREAM;
			XAbuffer.PlayBegin = m_PlayPos;
			XAbuffer.PlayLength = playlength;
			XAbuffer.LoopBegin = m_LoopStart;
			XAbuffer.LoopLength = m_LoopEnd;
			XAbuffer.LoopCount = m_LoopCount;
		}
		else if (m_PlayPos >= m_LoopStart && m_PlayPos < m_LoopEnd) {
			//填充第一段
			XAbuffer.Flags = 0;//还不是流末尾
			XAbuffer.PlayBegin = m_PlayPos;
			XAbuffer.PlayLength = m_LoopEnd - m_PlayPos;// 播放到循环节尾
			XAbuffer.LoopBegin = 0;
			XAbuffer.LoopLength = 0;
			XAbuffer.LoopCount = 0;//不循环
			if (m_Voice->SubmitSourceBuffer(&XAbuffer) != S_OK) {
				m_Status = AudioStatus::ERROR;
				return false;
			}
			//填充第二段
			XAbuffer.Flags = XAUDIO2_END_OF_STREAM;
			XAbuffer.PlayBegin = m_LoopStart;//回到循环节开始处播放
			XAbuffer.PlayLength = totalsample - m_LoopStart;
			XAbuffer.LoopBegin = m_LoopStart;
			XAbuffer.LoopLength = m_LoopEnd;
			XAbuffer.LoopCount = m_LoopCount;
		}
		else if (m_PlayPos >= m_LoopEnd) {
			//处于循环节后面了
			XAbuffer.Flags = XAUDIO2_END_OF_STREAM;
			XAbuffer.PlayBegin = m_PlayPos;
			XAbuffer.PlayLength = playlength;
			XAbuffer.LoopBegin = 0;
			XAbuffer.LoopLength = 0;
			XAbuffer.LoopCount = 0;//不循环，播放到末尾
		}
	}
	else {
		//不循环
		XAbuffer.Flags = XAUDIO2_END_OF_STREAM;
		XAbuffer.PlayBegin = m_PlayPos;
		XAbuffer.PlayLength = playlength;
		XAbuffer.LoopBegin = 0;
		XAbuffer.LoopLength = 0;
		XAbuffer.LoopCount = 0;
	}
	if (m_Voice->SubmitSourceBuffer(&XAbuffer) != S_OK) {
		m_Status = AudioStatus::ERROR;
		return false;
	}
	else {
		return true;
	}
}

void XAudio2SoundPlayerImpl::Play() {
	if (m_Status != AudioStatus::PLAY && m_Status != AudioStatus::ERROR) {
		if (m_Status == AudioStatus::STOP) {
			if (!pushSoundBuffer()) {
				return;
			}
		}
		if (m_Voice->Start() == S_OK) {
			m_Status = AudioStatus::PLAY;
		}
		else {
			m_Status = AudioStatus::ERROR;
		}
	}
}

void XAudio2SoundPlayerImpl::Pause() {
	if (m_Status == AudioStatus::PLAY && m_Status != AudioStatus::ERROR) {
		if (m_Voice->Stop() == S_OK) {
			m_Status = AudioStatus::PAUSE;
		}
		else {
			m_Status = AudioStatus::ERROR;
		}
	}
}

void XAudio2SoundPlayerImpl::Stop() {
	if ((m_Status == AudioStatus::PLAY || m_Status == AudioStatus::PAUSE) && m_Status != AudioStatus::ERROR) {
		if (m_Voice->Stop() == S_OK && m_Voice->FlushSourceBuffers() == S_OK) {
			m_Status = AudioStatus::STOP;
		}
		else {
			m_Status = AudioStatus::ERROR;
		}
	}
}

AudioStatus XAudio2SoundPlayerImpl::GetStatus() {
	return m_Status;
}

void XAudio2SoundPlayerImpl::SetLoop(unsigned int start, unsigned int end, unsigned int loopcount) {
	unsigned int totalsample = GetTotalTime();
	m_LoopStart = (start > totalsample) ? totalsample : start;
	m_LoopEnd = (end > totalsample) ? totalsample : end;
	m_LoopCount = (loopcount > XAUDIO2_LOOP_INFINITE) ? XAUDIO2_LOOP_INFINITE : loopcount;
	//检查循环节头尾是否填反了……
	if (m_LoopStart > m_LoopEnd) {
		swap<unsigned int, unsigned int>(m_LoopStart, m_LoopEnd);
	}
	//总体检查
	if (m_LoopStart == m_LoopEnd) {
		//没有循环节，默认播放整首曲子
		m_LoopStart = 0;
		m_LoopEnd = 0;
		m_LoopCount = 0;
	}
	else if (m_LoopStart == 0 && m_LoopEnd == 0) {
		//不循环，则循环时循环次数不能大于0
		m_LoopCount = 0;
	}
	else if (m_LoopCount == 0) {
		//不循环时不能有循环节
		m_LoopStart = 0;
		m_LoopEnd = 0;
	}
}

void XAudio2SoundPlayerImpl::GetLoop(unsigned int* outstart, unsigned int* outend, unsigned int* outloopcount) {
	*outstart = m_LoopStart;
	*outend = m_LoopEnd;
	*outloopcount = m_LoopCount;
}

bool XAudio2SoundPlayerImpl::SetTime(unsigned int sample) {
	//检查是不是超出去了
	unsigned int totalsample = GetTotalTime();
	if (sample > totalsample) {
		m_PlayPos = totalsample;
	}
	else {
		m_PlayPos = sample;
	}
	//需要的时候重新播放
	if (m_Status == AudioStatus::PLAY || m_Status == AudioStatus::PAUSE) {
		Stop();//先停止
		Play();//然后播放
	}
	return true;
}

unsigned int XAudio2SoundPlayerImpl::GetTime() {
	unsigned long long all = GetPlayerdTime();//已采样数（包括循环的部分）
	if (m_LoopStart != 0 && m_LoopEnd != 0 && m_LoopCount != 0) {
		if (m_PlayPos < m_LoopEnd) {
			//播放位置位于循环节尾前面
			if (all <= (m_LoopEnd - m_PlayPos)) {
				//还没有进入循环，直接返回是准确的
				return m_PlayPos + (unsigned int)all;
			}
			else {
				//已经进入过循环
				//                           减去前导部分                对循环节长度进行取余
				unsigned int pos = (all - (m_LoopStart - m_PlayPos)) % (m_LoopEnd - m_LoopStart);
				//        循环节头 + 余数
				return m_LoopStart + pos;
			}
		}
		else if (m_PlayPos >= m_LoopEnd) {
			//处于循环节尾后面，不是循环播放，直接返回也是准确的
			return m_PlayPos + (unsigned int)all;
		}
	}
	else {
		//不是循环播放，直接返回是准确的
		return (unsigned int)all;
	}
}

unsigned long long XAudio2SoundPlayerImpl::GetPlayerdTime() {
	XAUDIO2_VOICE_STATE state;
	m_Voice->GetState(&state);
	return state.SamplesPlayed;
}

unsigned int XAudio2SoundPlayerImpl::GetTotalTime() {
	return m_Decoder->GetDataSize() / (unsigned int)m_Decoder->GetBlockAlign();
}

bool XAudio2SoundPlayerImpl::SetTimeSec(float sec) {
	return SetTimeSec(sec * m_Decoder->GetSamplesPerSec());
}

float XAudio2SoundPlayerImpl::GetTimeSec() {
	return GetTimeSec() / m_Decoder->GetSamplesPerSec();
}

float XAudio2SoundPlayerImpl::GetPlayerdTimeSec() {
	return GetPlayerdTime() / m_Decoder->GetSamplesPerSec();
}

float XAudio2SoundPlayerImpl::GetTotalTimeSec() {
	return GetTotalTime() / m_Decoder->GetSamplesPerSec();
}

void XAudio2SoundPlayerImpl::SetVolume(float volume) {
	volume = (volume > 1.0f) ? 1.0f : volume;
	volume = (volume < -1.0f) ? -1.0f : volume;
	m_Voice->SetVolume(volume);
}

float XAudio2SoundPlayerImpl::GetVolume() {
	float v;
	m_Voice->GetVolume(&v);
	return v;
}

void XAudio2SoundPlayerImpl::SetBalance(float balance) {
	float* mat = new float[m_Decoder->GetChannels() * 2];
	if (m_Decoder->GetChannels() == 1) {
		if (balance < 0.0f) {
			//负值，右声道声音小
			mat[1] = balance + 1.0f;
			mat[0] = 1.0f;
		}
		else if (balance > 0.0f) {
			//正值，左声道声音小
			mat[0] = 1.0f - balance;
			mat[1] = 1.0f;
		}
		else {
			mat[1] = 1.0f;
			mat[0] = 1.0f;
		}
	}
	else if (m_Decoder->GetChannels() == 2) {
		mat[1] = mat[2] = 0.0f;
		if (balance < 0.0f) {
			//负值，右声道声音小
			mat[3] = balance + 1.0f;
			mat[0] = 1.0f;
		}
		else if (balance > 0.0f) {
			//正值，左声道声音小
			mat[0] = 1.0f - balance;
			mat[3] = 1.0f;
		}
		else {
			mat[3] = 1.0f;
			mat[0] = 1.0f;
		}
	}
	m_Voice->SetOutputMatrix(NULL, m_Decoder->GetChannels(), 2, mat);
	delete[] mat;
}

float XAudio2SoundPlayerImpl::GetBalance() {
	float* mat = new float[m_Decoder->GetChannels() * 2];
	float ret = 0.0f;
	m_Voice->GetOutputMatrix(NULL, m_Decoder->GetChannels(), 2, mat);
	if (m_Decoder->GetChannels() == 1) {
		if (mat[0] < 1.0f) {
			//左声道声音小，应该是在右边，为正
			ret = 1.0f - mat[0];
		}
		else if (mat[1] < 1.0f) {
			//右声道声音小，应该是在左边，为负
			ret = -(1.0f - mat[1]);
		}
	}
	else if (m_Decoder->GetChannels() == 2) {
		if (mat[0] < 1.0f) {
			//左声道声音小，应该是在右边，为正
			ret = 1.0f - mat[0];
		}
		else if (mat[3] < 1.0f) {
			//右声道声音小，应该是在左边，为负
			ret = -(1.0f - mat[3]);
		}
	}
	delete[] mat;
	return ret;
}

void XAudio2SoundPlayerImpl::SetSpeed(float speed) {
	speed = (speed > 2.0f) ? 2.0f : speed;
	speed = (speed < 0.0005f) ? 0.0005f : speed;
	m_Voice->SetFrequencyRatio(speed);
}

float XAudio2SoundPlayerImpl::GetSpeed() {
	float r;
	m_Voice->GetFrequencyRatio(&r);
	return r;
}

bool XAudio2SoundPlayerImpl::GetFFT(float** outdata, unsigned int* outsize, unsigned int channel) {
	static const unsigned int ms_DataBufferSize = 4096;//FFT数据缓冲区大小
	static const unsigned int ms_VaveValueSize = 512;//FFT波形数值大小
	//准备工作内存
	if (m_FFTWorkset == nullptr) {
		m_FFTWorkset = (char*)malloc(xmath::fft::getNeededWorksetSize(ms_VaveValueSize));
	}
	if (m_FFTOutComplex == nullptr) {
		m_FFTOutComplex = (float*)malloc(ms_VaveValueSize * sizeof(float) * 2);//512 * 2 = 1024
	}
	//离散窗（函数）数列
	static float fftWindow[ms_VaveValueSize];
	xmath::fft::getWindow(ms_VaveValueSize, fftWindow);
	//复制一段要进行FFT的数据
	unsigned int readsize = 0;
	static unsigned char databuffer[ms_DataBufferSize];
	{
		ZeroMemory(databuffer, ms_DataBufferSize * sizeof(unsigned char));
		unsigned char* dataptr = m_Decoder->GetDataBuffer();
		unsigned int datasize = m_Decoder->GetDataSize();
		unsigned int pos = GetTime() * m_Decoder->GetBlockAlign();
		for (unsigned int offset = 0; offset < ms_DataBufferSize; offset++) {
			if ((pos + offset) < datasize) {
				databuffer[offset] = dataptr[pos + offset];
				readsize++;
			}
			else {
				break;
			}
		}
	}
	//解析成浮点数据
	uint32_t getsize = 0;
	static float wavevalue[ms_VaveValueSize];
	{
		const auto nBA = m_Decoder->GetBlockAlign();
		const auto nCh = m_Decoder->GetChannels();
		const auto factor = 1u << (nBA / nCh * 8 - 1);
		if (nCh == 1) {
			int64_t n;//中间变量
			for (uint32_t select = 0; select < ms_VaveValueSize; select++) {
				if (select * nBA >= readsize) {
					break;
				}

				int64_t val = 0;
				for (size_t j = 0; j < nBA; ++j) {
					n = databuffer[j + select * nBA];
					val += n << (j * 8);
				}
				if (val > factor - 1) {
					n = factor;
					val = val - n * 2;
				}
				wavevalue[select] = double(val) / factor * fftWindow[select];

				getsize++;
			}
		}
		else if (nCh == 2) {
			int64_t n;//中间变量
			for (uint32_t select = 0; select < ms_VaveValueSize; select++) {
				if (select * nBA >= readsize) {
					break;
				}

				int64_t val = 0;
				if (channel == 1) {
					for (size_t j = 0; j < nBA / nCh; ++j) {
						n = databuffer[j + select * nBA];
						val ^= n << (j * 8);
					}
				}
				else if (channel == 2) {
					for (size_t j = (nBA / nCh); j < nBA; ++j) {
						n = databuffer[j + select * nBA];
						val ^= n << (j * 8);
					}
				}
				if (val > factor - 1) {
					n = factor;
					val = val - n * 2;
				}
				wavevalue[select] = double(val) / factor * fftWindow[select];

				getsize++;
			}
		}
	}
	//FFT
	float* out = new float[ms_VaveValueSize / 2];
	xmath::fft::fft(ms_VaveValueSize, m_FFTWorkset, wavevalue, m_FFTOutComplex, out);
	//return
	*outdata = out;
	*outsize = ms_VaveValueSize / 2;
	return true;
}
