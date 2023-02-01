// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#pragma once

struct ConsoleMessageType final
{
	enum Enum
	{
		Info = 0,
		Developer = 1,
		Verbose = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};
};

struct ConsoleMessage
{
	std::string text;
	float timeSubmitted = 0.0f;
	ConsoleMessageType::Enum type = ConsoleMessageType::Info;
};
