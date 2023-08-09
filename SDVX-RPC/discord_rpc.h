#pragma once
#include "discord/cpp/discord.h"
#include <iostream>
#include <csignal>

struct DiscordState {
	std::unique_ptr<discord::Core> core;
};

void rpc_init(DiscordState& state);
void rpc_update(DiscordState& state, std::string detail_text, std::string state_text, std::string large_key, std::string large_text, std::string small_key, std::string small_text, int mode);
void rpc_close(DiscordState& state);
void runDiscordCallbacks(DiscordState& state);
void handleInterruptSignal(int);