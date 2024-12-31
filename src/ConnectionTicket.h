// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "sigc++/connection.h"

// Lifetime management utility for sigc::connection
// Use this instead of sigs::connection if you don't want to manually
// disconnect in the destructor

struct ConnectionTicket {

	~ConnectionTicket() {
		m_connection.disconnect();
	}

	void operator=(sigc::connection &&c) {
		m_connection = c;
	}

	void disconnect() {
		m_connection.disconnect();
	}

	sigc::connection m_connection;
};
