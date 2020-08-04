#include "E2DXAudio2VoicePool.hpp"

using namespace std;
using namespace Eyes2D;
using namespace Eyes2D::Sound;

XAudio2VoicePool::XAudio2VoicePool(IXAudio2* p, IXAudio2MasteringVoice* m) {
	m_XAudio = p;
	m_MasteringVoice = m;
	m_TotalVoiceSpace = 64;
	m_NextAllocSpace = 4;
	m_FreeVoiceID.clear();
	m_SourceVoice.clear();
	//初始化ID和空指针
	for (int i = 0; i < 64; i++) {
		m_FreeVoiceID.push_back(i);
		IXAudio2SourceVoice* voice = nullptr;
		m_SourceVoice.push_back(voice);
	}

	CreateMixVoice("SE");
	CreateMixVoice("BGM");
}

XAudio2VoicePool::~XAudio2VoicePool() {
	for (auto i : m_SourceVoice) {
		if (i != nullptr) {
			i->DestroyVoice();
		}
	}
	m_SourceVoice.clear();
	for (auto& i : m_MixVoice) {
		i.second->DestroyVoice();
	}
	m_MixVoice.clear();
	m_MasteringVoice = nullptr;
	m_XAudio = nullptr;
}

int XAudio2VoicePool::allocID() {
	if (m_FreeVoiceID.empty()) {
		for (int i = m_TotalVoiceSpace; i < (m_TotalVoiceSpace + m_NextAllocSpace); i++) {
			m_FreeVoiceID.push_back(i);
			IXAudio2SourceVoice* voice = nullptr;
			m_SourceVoice.push_back(voice);
		}
	}
	m_TotalVoiceSpace = m_TotalVoiceSpace + m_NextAllocSpace;
	m_NextAllocSpace = m_NextAllocSpace * 2;
	int id = m_FreeVoiceID.back();
	m_FreeVoiceID.pop_back();
	return id;
}

void XAudio2VoicePool::freeID(int id) {
	m_FreeVoiceID.push_back(id);
}

IXAudio2SubmixVoice* XAudio2VoicePool::CreateMixVoice(const string& name) {
	XAUDIO2_SEND_DESCRIPTOR sendunit;
	sendunit.Flags = 0;
	sendunit.pOutputVoice = m_MasteringVoice;
	XAUDIO2_VOICE_SENDS sendlist;
	sendlist.SendCount = 1;
	sendlist.pSends = &sendunit;
	
	IXAudio2SubmixVoice* mixer;
	HRESULT hr = m_XAudio->CreateSubmixVoice(&mixer, 2, 44100, 0, 0, &sendlist, 0);
	if (FAILED(hr))
		throw E2DException(0, hr, L"Eyes2D::XAudio2VoicePool::CreateMixVoice", L"Failed to create SubmixVoice.");

	m_MixVoice.insert({ name, mixer });
	return mixer;
}

IXAudio2SubmixVoice* XAudio2VoicePool::GetMixVoice(const string& name) {
	auto search = m_MixVoice.find(name);
	if (search != m_MixVoice.end())
		return search->second;
	else
		return nullptr;
}

int                  XAudio2VoicePool::CreateSourceVoice(const std::string& mixername, WAVEFORMATEX& format) {
	IXAudio2SubmixVoice* mixer = GetMixVoice(mixername);
	if (mixer == nullptr)
		throw E2DException(0, 0, L"Eyes2D::XAudio2VoicePool::CreateSourceVoice", L"Failed to find target SubmixVoice.");

	XAUDIO2_SEND_DESCRIPTOR sendunit;
	sendunit.Flags = 0;
	sendunit.pOutputVoice = mixer;
	XAUDIO2_VOICE_SENDS sendlist;
	sendlist.SendCount = 1;
	sendlist.pSends = &sendunit;
	
	IXAudio2SourceVoice* source;
	HRESULT hr = GetXAudio().GetXAudio()->CreateSourceVoice(&source, &format, 0, 2.0f, 0, &sendlist);
	if (FAILED(hr))
		throw E2DException(0, hr, L"Eyes2D::XAudio2VoicePool::CreateSourceVoice", L"Failed to create SourceVoice.");

	int id = allocID();
	m_SourceVoice[id] = source;
	return id;
}

IXAudio2SourceVoice* XAudio2VoicePool::GetSourceVoice(int id) {
	return m_SourceVoice.at(id);
}

void                 XAudio2VoicePool::DeleteSourceVoice(int id) {
	IXAudio2SourceVoice* source = m_SourceVoice.at(id);
	if (source != nullptr) {
		source->DestroyVoice();
		m_SourceVoice[id] = nullptr;
		freeID(id);
	}
}
