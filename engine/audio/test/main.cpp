#include <print>
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/AudioEndpoint.hpp"
#include "win32/base.hpp"

int main() {
	auto const logger = spdlog::stdout_color_mt("core");
	spdlog::set_default_logger(logger);

	win32::set_logger_writer([](std::string_view const message) -> void {
		core::Logger::error(message);
	});

	core::SmartReference<core::IAudioEndpoint> endpoint;
	if (!core::IAudioEndpoint::create(endpoint.put())) {
		core::Logger::error("core::IAudioEndpoint::create");
		return 1;
	}

	if (!endpoint->refreshAudioEndpoint()) {
		core::Logger::error("core::IAudioEndpoint::refreshAudioEndpoint");
		return 1;
	}

	for (uint32_t i = 0; i < endpoint->getAudioEndpointCount(); i++) {
		auto const name = endpoint->getAudioEndpointName(i);
		core::Logger::info("{}: {}", i, name);
		if (!endpoint->setAudioEndpoint(name)) {
			core::Logger::error("core::IAudioEndpoint::setAudioEndpoint('{}')", name);
			return 1;
		}
	}

	if (!endpoint->setAudioEndpoint("")) {
		core::Logger::error("core::IAudioEndpoint::setAudioEndpoint('')");
		return 1;
	}

	core::SmartReference<core::IAudioDecoder> decoder;
	if (!core::IAudioDecoder::create("D:/Project/东方远空界/game/THlib/music/New.ogg", decoder.put())) {
		core::Logger::error("core::IAudioDecoder::create");
		return 1;
	}

	core::SmartReference<core::IAudioPlayer> player;
	if (!endpoint->createLoopAudioPlayer(decoder.get(), core::AudioMixingChannel::music, player.put())) {
		core::Logger::error("core::IAudioEndpoint::createAudioPlayer");
		return 1;
	}

	player->setLoop(true, 105.60 - 90.40, 90.40);
	player->setTime(80.0);
	player->reset();
	player->start();

	std::string command;
	while (true) {
		std::cin >> command;
		if (command == "exit") {
			break;
		}
	}

	return 0;
}
