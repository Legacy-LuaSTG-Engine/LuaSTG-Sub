#include "backend/StreamLoopAudioPlayerXAudio2.hpp"
#include "core/Logger.hpp"
#include "win32/base.hpp"

using std::string_view_literals::operator""sv;

namespace core {
	bool StreamLoopAudioPlayerXAudio2::ActionQueue::createObjects() {
		semaphore_space = CreateSemaphoreExW(NULL, 64, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
		semaphore_data = CreateSemaphoreExW(NULL, 0, 64, NULL, 0, SEMAPHORE_ALL_ACCESS);
		if (semaphore_space == NULL || semaphore_data == NULL) return false;
		event_exit = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		event_buffer[0] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		event_buffer[1] = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
		if (event_exit == NULL || event_buffer[0] == NULL || event_buffer[1] == NULL) return false;
		return true;
	}

	void StreamLoopAudioPlayerXAudio2::ActionQueue::notifyExit() {
		SetEvent(event_exit);
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::notifyBufferAvailable(size_t const i) {
		SetEvent(event_buffer[i]);
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::sendAction(Action const& v) {
		DWORD const wait_result = WaitForSingleObject(semaphore_space, 1000); // 减少剩余空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行
		if (wait_result == WAIT_OBJECT_0) {
			data[writer_index] = v;
			writer_index = (writer_index + 1) % size;
			ReleaseSemaphore(semaphore_data, 1, NULL); // 增加已用空间的信号量
		}
		else if (wait_result == WAIT_TIMEOUT) {
			Logger::warn("[core] audio decode thread is blocking");
		}
	}
	void StreamLoopAudioPlayerXAudio2::ActionQueue::reciveAction(Action& v) {
		HANDLE objects[4] = {
			event_exit,
			event_buffer[0],
			event_buffer[1],
			semaphore_data,
		};
		switch (WaitForMultipleObjects(4, objects, FALSE, INFINITE)) {
		case WAIT_OBJECT_0 + 0:
			v.type = ActionType::Exit; // 特殊处理，且不重置 event
			break;
		case WAIT_OBJECT_0 + 1:
			ResetEvent(event_buffer[0]);
			v.type = ActionType::BufferAvailable; // 特殊处理
			v.action_buffer_available.index = 0;
			break;
		case WAIT_OBJECT_0 + 2:
			ResetEvent(event_buffer[1]);
			v.type = ActionType::BufferAvailable; // 特殊处理
			v.action_buffer_available.index = 1;
			break;
		case WAIT_OBJECT_0 + 3:
			//WaitForSingleObject(semaphore_data, INFINITE); // 减少已用空间的信号量，减少到 0 就会阻塞，大于 0 才会继续执行，上面已经调用过
			v = data[reader_index];
			reader_index = (reader_index + 1) % size;
			ReleaseSemaphore(semaphore_space, 1, NULL); // 增加剩余空间的信号量
			break;
		default:
			v.type = ActionType::Exit; // 出错了，快给爷退出
			break;
		}
	}

	StreamLoopAudioPlayerXAudio2::ActionQueue::ActionQueue() = default;
	StreamLoopAudioPlayerXAudio2::ActionQueue::~ActionQueue() {
		writer_index = 0;
		reader_index = 0;
		if (semaphore_space) CloseHandle(semaphore_space); semaphore_space = NULL;
		if (semaphore_data) CloseHandle(semaphore_data); semaphore_data = NULL;
		if (event_exit) CloseHandle(event_exit); event_exit = NULL;
		if (event_buffer[0]) CloseHandle(event_buffer[0]); event_buffer[0] = NULL;
		if (event_buffer[1]) CloseHandle(event_buffer[1]); event_buffer[1] = NULL;
	}
}

namespace core {
	// IXAudio2VoiceCallback

	void WINAPI StreamLoopAudioPlayerXAudio2::OnBufferEnd(void* const buffer_context) noexcept {
		action_queue.notifyBufferAvailable(reinterpret_cast<size_t>(buffer_context));
	}
	void WINAPI StreamLoopAudioPlayerXAudio2::OnVoiceError(void* const, HRESULT const error) noexcept {
		std::ignore = win32::check_hresult(error, "IXAudio2VoiceCallback::OnVoiceError"sv);
	}

	// IAudioEndpointEventListener

	void StreamLoopAudioPlayerXAudio2::onAudioEndpointCreate() {
		create();
	}
	void StreamLoopAudioPlayerXAudio2::onAudioEndpointDestroy() {
		destroy();
	}

	// StreamLoopAudioPlayerXAudio2

	StreamLoopAudioPlayerXAudio2::~StreamLoopAudioPlayerXAudio2() {
		if (m_parent) {
			m_parent->removeEventListener(this);
		}
		action_queue.notifyExit();
		if (m_working_thread.joinable()) {
			m_working_thread.join();
		}
		destroy();
	}

	bool StreamLoopAudioPlayerXAudio2::create() {

	}
	bool StreamLoopAudioPlayerXAudio2::create(AudioEndpointXAudio2* const parent, AudioMixingChannel const mixing_channel, IAudioDecoder* const decoder) {
		m_parent = parent;
		m_mixing_channel = mixing_channel;
		m_decoder = decoder;

		// 动作队列

		if (!action_queue.createObjects()) {
			Logger::error("[core] StreamLoopAudioPlayerXAudio2::ActionQueue::createObjects failed: CreateSemaphoreExW or CreateEventExW failed");
			return false;
		}

		// 填写格式

		m_format.wFormatTag = WAVE_FORMAT_PCM;
		m_format.nChannels = m_decoder->getChannelCount();
		m_format.nSamplesPerSec = m_decoder->getSampleRate();
		m_format.nAvgBytesPerSec = m_decoder->getByteRate();
		m_format.nBlockAlign = m_decoder->getFrameSize();
		m_format.wBitsPerSample = static_cast<uint16_t>(m_decoder->getSampleSize() * 8u);

		// 准备解码缓冲区

		auto const buffer_bytes = static_cast<size_t>(m_decoder->getFrameSize()) * 2048u;
		raw_buffer.resize(buffer_bytes * 2u);
		p_audio_buffer[0] = raw_buffer.data();
		p_audio_buffer[1] = raw_buffer.data() + buffer_bytes;

		// 启动解码线程

		m_working_thread = std::thread(&StreamLoopAudioPlayerXAudio2::worker, this);

		// 创建资源

		std::ignore = create();
		m_parent->addEventListener(this);
		return true;
	}
	void StreamLoopAudioPlayerXAudio2::destroy() {
		std::unique_lock lock(m_player_lock);
		if (m_voice != nullptr) {
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
	void StreamLoopAudioPlayerXAudio2::worker() {
		struct StreamBuffer {
			uint8_t* data{};
			uint32_t max_size{};
			uint32_t max_frame{};
			XAUDIO2_BUFFER info{};
			double add_time{};
			double set_time{};
		};

		std::array<StreamBuffer, 2> stream_buffer{};

		stream_buffer[0].data = raw_buffer.data();
		stream_buffer[0].max_size = static_cast<uint32_t>(raw_buffer.size()) / 2u; // 缓冲区一半
		stream_buffer[0].max_frame = stream_buffer[0].max_size / static_cast<uint32_t>(m_decoder->getFrameSize()); // 可以容纳这么多帧
		stream_buffer[0].info.pAudioData = stream_buffer[0].data;
		stream_buffer[0].info.pContext = reinterpret_cast<void*>(0);

		stream_buffer[1].data = raw_buffer.data() + stream_buffer[0].max_size; // 偏移到上一个缓冲区的尾部
		stream_buffer[1].max_size = stream_buffer[0].max_size; // 相同
		stream_buffer[1].max_frame = stream_buffer[0].max_frame; // 相同
		stream_buffer[1].info.pAudioData = stream_buffer[1].data;
		stream_buffer[1].info.pContext = reinterpret_cast<void*>(1);

		//#define PLAYER_DECODER_DEBUG

		double start_time = 0.0; // 用于 on_reset, on_set_time

		// 只有音频对象存在时才执行
		auto lock_player_and_do = [this](std::function<void()> const& fun) {
			std::shared_lock lock(m_player_lock);
			if (m_voice != nullptr) {
				fun();
			}
		};

		auto on_reset = [&](bool const play) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::Reset] ({})", play ? "Start" : "");
		#endif

			// 先停下来，改好解码器起始位置再清空队列
			lock_player_and_do([this] {
				m_voice->Stop();
			});

			// 配置为上次设置时间的信息
			total_time = 0.0;
			current_time = start_time;
			m_decoder->seekByTime(start_time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = start_time;

			// 清空当前缓冲区队列
			lock_player_and_do([&] {
				m_voice->FlushSourceBuffers();
				if (play) {
					m_voice->Start();
				}
			});
		};

		auto on_set_time = [&](double const time) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::SetTime] ({}s)", time);
		#endif

			// 配置信息
			start_time = time;
			total_time = 0.0; // 这里可以不关心线程访问冲突
			current_time = time; // 这里可以不关心线程访问冲突
			m_decoder->seekByTime(time);
			stream_buffer[0].add_time = stream_buffer[1].add_time = 0.0;
			stream_buffer[0].set_time = stream_buffer[1].set_time = time;

			// 清空当前缓冲区队列
			lock_player_and_do([this] {
				m_voice->FlushSourceBuffers();
			});
		};

		auto on_buffer_available = [&](size_t const index) {
		#ifdef PLAYER_DECODER_DEBUG
			Logger::debug("[Player] [ActionType::BufferAvailable] ({})", index);
		#endif

			StreamBuffer& buffer = stream_buffer[index];

			// 这个缓冲区播放完后，可以更新当前时间和总播放时间
			total_time += buffer.add_time; // 这里可以不关心线程访问冲突
			current_time = buffer.set_time; // 这里可以不关心线程访问冲突

			// 解码下一节到缓冲区
			uint32_t read_frames = 0;
			m_decoder->read(buffer.max_frame, buffer.data, &read_frames);
			audio_buffer_index = index; // 这里可以不关心线程访问冲突

			// 计算当前时间
			double time_pos = 0.0;
			m_decoder->tellAsTime(&time_pos);

			// 提交
			buffer.info.AudioBytes = read_frames * m_decoder->getFrameSize();
			lock_player_and_do([&] {
				m_voice->SubmitSourceBuffer(&buffer.info);
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::BufferAvailable] Submit {} Samples", read_frames);
			#endif
			});

			// 这个缓冲区播放完后要更新的数据
			buffer.add_time = static_cast<double>(read_frames) / static_cast<double>(m_decoder->getSampleRate());
			buffer.set_time = time_pos;
		};

		bool is_running = true;
		Action action = {};

		while (is_running) {
			action_queue.reciveAction(action);
			switch (action.type) {
			default:
			case ActionType::Exit:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Exit]");
			#endif
				is_running = false; // 该滚蛋了
				lock_player_and_do([this] {
					// 先停下，然后清空缓冲区队列，防止继续使用上面的局部 buffer 导致内存读取错误
					m_voice->Stop();
					m_voice->FlushSourceBuffers();
				});
				break;
			case ActionType::Stop:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Stop]");
			#endif
				lock_player_and_do([this] {
					m_voice->Stop();
				});
				break;
			case ActionType::Start:
			#ifdef PLAYER_DECODER_DEBUG
				Logger::debug("[Player] [ActionType::Start]");
			#endif
				lock_player_and_do([this] {
					m_voice->Start();
				});
				break;
			case ActionType::Reset:
				on_reset(action.action_reset.play);
				break;
			case ActionType::SetTime:
				on_set_time(action.action_set_time.time);
				break;
			case ActionType::BufferAvailable:
				on_buffer_available(action.action_buffer_available.index);
				break;
			}
		}
	}
}
