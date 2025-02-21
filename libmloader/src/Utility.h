// Copyright (c) 2025 mlogic1
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace mloader
{
	template<typename... T>
	inline int ExecShellWithCallback(std::function<void(const std::string&)> callback, const T&... args)
	{
		std::vector<std::string> arguments = { args... };

		std::ostringstream stream;
		for (int i = 0; i < arguments.size() - 1; ++i)
		{
			stream << arguments[i] << " ";
		}

		stream << arguments.back();

		std::string command = stream.str();
		FILE* fp = popen(command.c_str(), "r");

		if (fp == NULL)
		{
			perror("popen");
			return EXIT_FAILURE;
		}

		char buffer[1024];
		while(fgets(buffer, sizeof(buffer), fp)) 
		{
			if (callback)
			{
				callback(buffer);
			}
		}

		return pclose(fp);
	}

	template<typename... T>
	inline int ExecShell(const T&... args)
	{
		return ExecShellWithCallback(nullptr, args...);
	}
}

#endif // UTILITY_H

