/*
    discord.cpp
    Adrenaline Engine

    Definitions to everything related to discord integration.
*/

#include "discord.h"
#include <iostream>

void Adren::RPC::initialize() {
	DiscordEventHandlers handler;
	memset(&handler, 0, sizeof(handler));
	Discord_Initialize("953091116272582667", &handler, 1, NULL);
    std::cerr << "Discord RPC initialized.." << "\n \n" << std::endl;
}

void Adren::RPC::update() {
    DiscordRichPresence rpc;
    memset(&rpc, 0, sizeof(rpc));
    rpc.state = "Testing..";
    rpc.details = "Updating engine";
    rpc.startTimestamp = std::time(0);
    rpc.largeImageKey = "adren2";
    rpc.largeImageText = "This is my game engine.";
    Discord_UpdatePresence(&rpc);
}