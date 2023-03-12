// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#pragma once

// Windows is such cancer
#ifdef WIN32
#define		_WINSOCK_DEPRECATED_NO_WARNINGS
#define		NOMINMAX
#endif

#include <enet/enet.h>

#include <functional>
#include <thread>
#include <string>
#include <vector>

#include "Model/ConsoleMessage.hpp"

void Wait( float seconds );

float Now();
