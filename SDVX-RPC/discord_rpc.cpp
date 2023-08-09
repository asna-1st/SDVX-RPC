#include "discord_rpc.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <string>

namespace {
	volatile bool interrupted{ false };
}

void rpc_init(DiscordState& state) {
	discord::Core* core{};
	auto response = core->Create(123456789, DiscordCreateFlags_Default, &core);
	state.core.reset(core);

	if (!state.core) {
		std::cout << "Failed to instance Discord!\n";
	}
}

void rpc_update(DiscordState& state, std::string detail_text, std::string state_text, std::string large_key, std::string large_text, std::string small_key, std::string small_text, int mode) {
	std::time_t timestamps = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	discord::Activity activity{};
	activity.SetDetails(detail_text.c_str());
	activity.SetState(state_text.c_str());
	activity.GetAssets().SetLargeImage(large_key.c_str());
	activity.GetAssets().SetLargeText(large_text.c_str());
	activity.GetAssets().SetSmallImage(small_key.c_str());
	activity.GetAssets().SetSmallText(small_text.c_str());
	switch (mode) {
	case 1:
		activity.GetTimestamps().SetStart(timestamps);
		break;
	default:
		break;
	}
	activity.SetType(discord::ActivityType::Playing);
	state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed") << " update activity!\n";
		});
}

void rpc_close(DiscordState& state) {
	state.core.reset();
}

void handleInterruptSignal(int) {
	interrupted = true;
}

void runDiscordCallbacks(DiscordState& state) {
	std::signal(SIGINT, handleInterruptSignal);
	do {
		state.core->RunCallbacks();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	} while (!interrupted);
}

