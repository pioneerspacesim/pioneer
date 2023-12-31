// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Headtracker.h"
#include "core/Log.h"

#include <nanosockets/nanosockets.h>
#include <cstdint>

HeadtrackingManager::HeadtrackingManager() :
	m_trackerState {},
	m_trackerSocket {},
	m_connected(false)
{ }

HeadtrackingManager::~HeadtrackingManager()
{
	if (m_connected)
		Disconnect();
}

bool HeadtrackingManager::Connect(const char *host, uint16_t port)
{
	if (nanosockets_initialize()) {
		Log::Error("Headtracking: Error initializing socket library");
		return false;
	}

	NanoAddress address = {};
	address.port = port;

	if (!host || std::strlen(host) == 0) {
		if (nanosockets_address_set_ip(&address, "127.0.0.1")) {
			Log::Error("Headtracking: error setting default address");
			return false;
		}
	} else {
		if (nanosockets_address_set_hostname(&address, host)) {
			Log::Error("Headtracking: Error setting hostname to {}", host);
			return false;
		}
	}

	char ipaddr[64] = { 0 };
	nanosockets_address_get_ip(&address, ipaddr, sizeof(ipaddr));
	Log::Info("Listening for headtracking packets on {}:{}", ipaddr, port);

	m_trackerSocket = nanosockets_create(1024, 1024);
	if (m_trackerSocket == NANOSOCKETS_STATUS_ERROR) {
		Log::Error("Headtracking: Error creating UDP socket");
		return false;
	}

	if (nanosockets_bind(m_trackerSocket, &address)) {
		Log::Error("Headtracking: Error binding to UDP socket {}:{}", ipaddr, port);

		nanosockets_destroy(&m_trackerSocket);
		return false;
	}

	m_connected = true;
	return true;
}

void HeadtrackingManager::Disconnect()
{
	nanosockets_destroy(&m_trackerSocket);
	nanosockets_deinitialize();

	m_connected = false;
}

const HeadtrackingManager::State *HeadtrackingManager::GetHeadState() const
{
	return &m_trackerState;
}

void HeadtrackingManager::Update()
{
	if (!m_connected)
		return;

	int ready = nanosockets_poll(m_trackerSocket, 0);
	if (!ready || ready == -1)
		return;

	uint8_t packet[sizeof(State)] = {};
	memset(packet, 0, sizeof(State));

	while (nanosockets_poll(m_trackerSocket, 0) == 1) {
		NanoAddress _addr;
		int bytesRecv = nanosockets_receive(m_trackerSocket, &_addr, packet, sizeof(State));

		// Packet is the wrong size for headtracking, ignore it
		if (bytesRecv == sizeof(State)) {
			memcpy(&m_trackerState, packet, sizeof(State));
		}
	}
}
