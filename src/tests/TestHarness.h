/*
 ****************************************************************************
 *  Copyright (c) 2026 FreeLSS contributors                                 *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 ****************************************************************************
*/

#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

namespace freelss_test
{

inline int& failureCount()
{
	static int n = 0;
	return n;
}

inline int& testCount()
{
	static int n = 0;
	return n;
}

inline void recordPass(const char * name)
{
	testCount()++;
	std::printf("[ OK ] %s\n", name);
}

inline void recordFail(const char * name, const char * file, int line, const std::string& reason)
{
	testCount()++;
	failureCount()++;
	std::fprintf(stderr, "[FAIL] %s\n  %s:%d: %s\n", name, file, line, reason.c_str());
}

inline int reportAndExit()
{
	std::printf("\n%d test(s); %d failure(s).\n", testCount(), failureCount());
	return failureCount() == 0 ? 0 : 1;
}

}

#define TEST_ASSERT(name, cond)                                                 \
	do {                                                                        \
		if (!(cond)) {                                                          \
			std::ostringstream _oss;                                            \
			_oss << "expected: " << #cond;                                      \
			freelss_test::recordFail((name), __FILE__, __LINE__, _oss.str());   \
			return;                                                             \
		}                                                                       \
	} while (0)

#define TEST_ASSERT_EQ(name, expected, actual)                                  \
	do {                                                                        \
		auto _exp = (expected);                                                 \
		auto _act = (actual);                                                   \
		if (!(_exp == _act)) {                                                  \
			std::ostringstream _oss;                                            \
			_oss << "expected " << #actual << " == " << #expected               \
			     << "; got " << _act << " vs " << _exp;                         \
			freelss_test::recordFail((name), __FILE__, __LINE__, _oss.str());   \
			return;                                                             \
		}                                                                       \
	} while (0)

#define TEST_RUN(name, fn)                                                      \
	do {                                                                        \
		int before = freelss_test::failureCount();                              \
		fn();                                                                   \
		if (freelss_test::failureCount() == before) {                           \
			freelss_test::recordPass(name);                                     \
		}                                                                       \
	} while (0)
