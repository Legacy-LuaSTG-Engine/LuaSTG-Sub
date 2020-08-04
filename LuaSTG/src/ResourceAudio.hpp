#pragma once
#include "ResourceBase.hpp"
#include "E2DMath.hpp"

namespace LuaSTGPlus {
	// 声音资源
	/*
	class ResAudio :
		public Resource
	{
	private:
		fcyRefPointer<f2dSoundBuffer> m_pBuffer;
		int m_status;//0停止1暂停2播放
		float m_lastfrq;
		long m_freq;
	public:
		void Resume()
		{
			m_pBuffer->Play();
			m_status = 2;
		}
		void Pause()
		{
			m_pBuffer->Pause();
			m_status = 1;
		}
		void Stop()
		{
			m_pBuffer->Stop();
			m_status = 0;
		}
	};
	//*/

	// 音效
	class ResSound :
		public Resource
	{
	private:
		fcyRefPointer<f2dSoundBuffer> m_pBuffer;
		int m_status;//0停止1暂停2播放
		float m_lastfrq;
		long m_freq;
	public:
		void Play(float vol, float pan)
		{
			m_pBuffer->Stop();

			//float nv = VolumeFix(vol);
			float nv = (float)Eyes2D::Math::LinearToLog(vol);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);
			if (m_pBuffer->GetPan() != pan)
				m_pBuffer->SetPan(pan);

			m_pBuffer->Play();
			m_status = 2;
		}

		void Resume()
		{
			m_pBuffer->Play();
			m_status = 2;
		}

		void Pause()
		{
			m_pBuffer->Pause();
			m_status = 1;
		}

		void Stop()
		{
			m_pBuffer->Stop();
			m_status = 0;
		}

		bool IsPlaying()
		{
			return m_pBuffer->IsPlaying();
		}

		bool IsStopped()
		{
			return !IsPlaying() && m_pBuffer->GetTime() == 0.;
			//return !IsPlaying() || m_status == 0;
		}

		bool SetSpeed(float speed) {
			float frq = (float)m_freq;
			int newfrq = (int)(frq * speed);
			if (newfrq > 100000 || newfrq < 100) {
				return false;
			}
			if (m_pBuffer->SetFrequency(newfrq) == FCYERR_OK) {
				m_lastfrq = speed;
				return true;
			}
			else {
				return false;
			}
		}

		float GetSpeed() {
			if (m_lastfrq <= 0.0f) {
				return 1.0f;
			}
			else {
				return m_lastfrq;
			}
		}
	public:
		ResSound(const char* name, fcyRefPointer<f2dSoundBuffer> buffer) : Resource(ResourceType::SoundEffect, name) {
			m_pBuffer = buffer;
			m_lastfrq = -1.0f;
			m_freq = m_pBuffer->GetFrequency();
		}
	};

	// 背景音乐
	class ResMusic :
		public Resource
	{
	public:
		// 对SoundDecoder作一个包装来保持BGM循环
		// 使用该Wrapper之后SoundBuffer的播放参数（位置）将没有意义
		// ! 从fancystg（已坑）中抽取的上古时期代码
		class BGMWrapper :
			public fcyRefObjImpl<f2dSoundDecoder>
		{
		protected:
			fcyRefPointer<f2dSoundDecoder> m_pDecoder;

			// 转到采样为单位
			fuInt m_TotalSample;
			fuInt m_pLoopStartSample;
			fuInt m_pLoopEndSample; // 监视哨，=EndSample+1
		public:
			// 直接返回原始参数
			fuInt GetBufferSize() { return m_pDecoder->GetBufferSize(); }
			fuInt GetAvgBytesPerSec() { return m_pDecoder->GetAvgBytesPerSec(); }
			fuShort GetBlockAlign() { return m_pDecoder->GetBlockAlign(); }
			fuShort GetChannelCount() { return m_pDecoder->GetChannelCount(); }
			fuInt GetSamplesPerSec() { return m_pDecoder->GetSamplesPerSec(); }
			fuShort GetFormatTag() { return m_pDecoder->GetFormatTag(); }
			fuShort GetBitsPerSample() { return m_pDecoder->GetBitsPerSample(); }

			// 不作任何处理
			fLen GetPosition() { return m_pDecoder->GetPosition(); }
			fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset) { return m_pDecoder->SetPosition(Origin, Offset); }

			// 对Read作处理
			fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead);
		public:
			BGMWrapper(fcyRefPointer<f2dSoundDecoder> pOrg, fDouble LoopStart, fDouble LoopEnd);
		};
	private:
		fcyRefPointer<f2dSoundBuffer> m_pBuffer;
		int m_status;//0停止1暂停2播放
		float m_lastfrq;
		long m_freq;
	public:
		void Play(float vol, double position)
		{
			m_pBuffer->Stop();
			m_pBuffer->SetTime(position);

			//float nv = VolumeFix(vol);
			float nv = (float)Eyes2D::Math::LinearToLog(vol);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);

			m_pBuffer->Play();
			m_status = 2;
		}

		void Stop()
		{
			m_pBuffer->Stop();
			m_status = 0;
		}

		void Pause()
		{
			m_pBuffer->Pause();
			m_status = 1;
		}

		void Resume()
		{
			m_pBuffer->Play();
			m_status = 2;
		}

		bool IsPlaying()
		{
			return m_pBuffer->IsPlaying();
		}

		bool IsPaused() {
			return m_status == 1;
		}

		bool IsStopped()
		{
			return !IsPlaying() && m_pBuffer->GetTime() == 0.;//用播放时间来判断貌似会遇到[播放然后立即判断状态会返回stop状态]的错误
			//return (!IsPlaying() || m_status == 0) && m_pBuffer->GetTime() == 0.;
		}

		void SetVolume(float v)
		{
			//float nv = VolumeFix(v);
			float nv = (float)Eyes2D::Math::LinearToLog(v);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);
		}

		float GetVolume() {
			double dv = (double)m_pBuffer->GetVolume();
			double tv = Eyes2D::Math::LogToLinear(dv);
			return (float)tv;
			//return m_pBuffer->GetVolume();
		}

		bool SetSpeed(float speed) {
			float frq = (float)m_freq;
			int newfrq = (int)(frq * speed);
			if (newfrq > 100000 || newfrq < 100) {
				return false;
			}
			if (m_pBuffer->SetFrequency(newfrq) == FCYERR_OK) {
				m_lastfrq = speed;
				return true;
			}
			else {
				return false;
			}
		}

		float GetSpeed() {
			if (m_lastfrq <= 0.0f) {
				return 1.0f;
			}
			else {
				return m_lastfrq;
			}
		}
	public:
		ResMusic(const char* name, fcyRefPointer<f2dSoundBuffer> buffer) : Resource(ResourceType::Music, name) {
			m_pBuffer = buffer;
			m_status = 0;
			m_lastfrq = -1.0f;
			m_freq = m_pBuffer->GetFrequency();
		}
	};
}
