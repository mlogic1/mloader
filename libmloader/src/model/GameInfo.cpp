#include "GameInfo.h"

namespace mloader
{
	bool operator<(const GameInfo& lhs, const GameInfo& rhs)
	{
		return lhs.GameName < rhs.GameName;
	}
}