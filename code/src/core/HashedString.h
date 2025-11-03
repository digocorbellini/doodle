#pragma once

#include "common\types.h"

static constexpr std::size_t HASHED_STRING_MAX_LENGTH = 64;

struct HashedString
{
	std::uint64_t hash;

	// TODO: perhaps this should only be available for debug?
	const char str[HASHED_STRING_MAX_LENGTH];
};

