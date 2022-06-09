/*
	discord.h
	Adrenaline Engine

	Declarations to everything related to discord integration.
*/

#pragma once
#include "discord_register.h"
#include "discord_rpc.h"

namespace Adren {
class RPC {
public:
	void initialize();
	void update();
};
}