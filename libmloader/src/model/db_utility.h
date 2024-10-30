#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <exception>

namespace autolog
{
	namespace dbutil
	{
		static void ValidateFieldAndValues(const std::vector<std::string>& Fields, const std::vector<std::string>& Values)
		{
			if (Fields.size() != Values.size())
			{
				throw std::runtime_error("Number of Fields is not the same as number of Values");
			}
		}

		static std::string GenericJoin(const std::vector<std::string>& strings, const char delimiter = ' ')
		{
			std::string result{""};

			std::for_each(strings.begin(), strings.end(), [&](const std::string& str)
			{
				result += str + delimiter;
			});

			// remove last delimiter character
			result.pop_back();
			return result;
		}

		static std::string CreateGenericInsertStatement(const std::string& TableName, const std::vector<std::string>& Fields, const std::vector<std::string>& Values)
		{
			ValidateFieldAndValues(Fields, Values);

			throw std::runtime_error("Unimplemented");

			return "";
		}
	}
}