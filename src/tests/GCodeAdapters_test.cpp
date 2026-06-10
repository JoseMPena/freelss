/*
 ****************************************************************************
 *  Copyright (c) 2026 FreeLSS contributors                                 *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  FreeLSS is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 ****************************************************************************
*/

#include "TestHarness.h"
#include "FakeByteStream.h"

#include "../GCodeClient.h"
#include "../GCodeTurnTable.h"
#include "../GCodeLaser.h"
#include "../GCodeLighting.h"

#include <vector>
#include <string>

using freelss::FakeByteStream;
using freelss::GCodeClient;
using freelss::GCodeTurnTable;
using freelss::GCodeLaser;
using freelss::GCodeLighting;
using freelss::Laser;

namespace
{

const freelss::real TWO_PI = (freelss::real) (2.0 * 3.14159265359);

/**
 * Lifecycle helper: installs a GCodeClient backed by a FakeByteStream
 * that always acks, and releases it on scope exit AFTER the adapter on
 * the stack has been destroyed.  The naive "install/release at function
 * boundaries" pattern lets adapter destructors run on a freed client.
 */
struct ClientScope
{
	FakeByteStream * stream;

	ClientScope() : stream(new FakeByteStream())
	{
		for (int i = 0; i < 64; i++) { stream->queueResponse(">\n"); }
		GCodeClient::install(stream);
	}

	~ClientScope()
	{
		GCodeClient::release();
	}

	std::vector<std::string> lines() const { return stream->writtenLines(); }
};

// ---------- GCodeTurnTable ----------

void test_turntable_setMotorEnabled_true_sends_M17()
{
	ClientScope scope;
	{
		GCodeTurnTable tt(GCodeClient::get(), 3200, 1200, false);
		tt.setMotorEnabled(true);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("tt.enable.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("tt.enable.cmd",   std::string("M17"), lines[0]);
	}
}

void test_turntable_setMotorEnabled_false_sends_M18()
{
	ClientScope scope;
	{
		GCodeTurnTable tt(GCodeClient::get(), 3200, 1200, false);
		tt.setMotorEnabled(false);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("tt.disable.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("tt.disable.cmd",   std::string("M18"), lines[0]);
	}
}

void test_turntable_rotate_full_revolution_sends_all_steps()
{
	ClientScope scope;
	{
		GCodeTurnTable tt(GCodeClient::get(), 3200, 1200, false);
		int steps = tt.rotate(TWO_PI);

		TEST_ASSERT_EQ("tt.rotate.steps", 3200, steps);

		std::vector<std::string> lines = scope.lines();
		// tt.rotate sends G2; tt's dtor sends M18 -> 2 lines total.
		TEST_ASSERT_EQ("tt.rotate.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("tt.rotate.cmd",   std::string("G2 T3200 F1200"), lines[0]);
	}
	// After scope: dtor produced M18 then release happened.
	std::vector<std::string> lines = scope.lines();
	TEST_ASSERT_EQ("tt.rotate.after_dtor.count", (size_t) 2, lines.size());
	TEST_ASSERT_EQ("tt.rotate.after_dtor.dtor",  std::string("M18"), lines[1]);
}

void test_turntable_rotate_zero_or_negative_is_noop()
{
	ClientScope scope;
	{
		GCodeTurnTable tt(GCodeClient::get(), 3200, 1200, false);
		int a = tt.rotate(0);
		int b = tt.rotate(-1);

		TEST_ASSERT_EQ("tt.zero", 0, a);
		TEST_ASSERT_EQ("tt.neg",  0, b);
		TEST_ASSERT_EQ("tt.noop.writes", (size_t) 0, scope.lines().size());
	}
}

void test_turntable_rotate_inverts_direction_when_configured()
{
	ClientScope scope;
	{
		GCodeTurnTable tt(GCodeClient::get(), 3200, 1200, true);
		tt.rotate(TWO_PI / 4); // 800 steps -> "-800"

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("tt.invert.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("tt.invert.cmd",   std::string("G2 T-800 F1200"), lines[0]);
	}
}

// ---------- GCodeLaser ----------

void test_laser_right_on_sends_M21()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::RIGHT_LASER);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.right_on.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("laser.right_on.cmd",   std::string("M21"), lines[0]);
		TEST_ASSERT("laser.right.state",  laser.isOn(Laser::RIGHT_LASER));
		TEST_ASSERT("laser.left.state",  !laser.isOn(Laser::LEFT_LASER));
	}
}

void test_laser_left_on_sends_M19()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::LEFT_LASER);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.left_on.count", (size_t) 1, lines.size());
		TEST_ASSERT_EQ("laser.left_on.cmd",   std::string("M19"), lines[0]);
	}
}

void test_laser_right_off_sends_M22()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::RIGHT_LASER);
		laser.turnOff(Laser::RIGHT_LASER);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.right_off.count", (size_t) 2, lines.size());
		TEST_ASSERT_EQ("laser.right_off.cmd",   std::string("M22"), lines[1]);
		TEST_ASSERT("laser.right_off.state",   !laser.isOn(Laser::RIGHT_LASER));
	}
}

void test_laser_left_off_sends_M20()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::LEFT_LASER);
		laser.turnOff(Laser::LEFT_LASER);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.left_off.count", (size_t) 2, lines.size());
		TEST_ASSERT_EQ("laser.left_off.cmd",   std::string("M20"), lines[1]);
	}
}

void test_laser_all_on_sends_both_left_then_right()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::ALL_LASERS);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.all_on.count", (size_t) 2, lines.size());
		TEST_ASSERT_EQ("laser.all_on.left",  std::string("M19"), lines[0]);
		TEST_ASSERT_EQ("laser.all_on.right", std::string("M21"), lines[1]);
		TEST_ASSERT("laser.all_on.state",    laser.isOn(Laser::ALL_LASERS));
	}
}

void test_laser_all_off_sends_both_off()
{
	ClientScope scope;
	{
		GCodeLaser laser(GCodeClient::get());
		laser.turnOn(Laser::ALL_LASERS);
		laser.turnOff(Laser::ALL_LASERS);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("laser.all_off.count", (size_t) 4, lines.size());
		TEST_ASSERT_EQ("laser.all_off.left_off",  std::string("M20"), lines[2]);
		TEST_ASSERT_EQ("laser.all_off.right_off", std::string("M22"), lines[3]);
		TEST_ASSERT("laser.all_off.state",       !laser.isOn(Laser::ALL_LASERS));
	}
}

// ---------- GCodeLighting ----------

void test_lighting_zero_sends_M6_only_on_transition()
{
	ClientScope scope;
	{
		GCodeLighting light(GCodeClient::get());
		light.setIntensity(0); // already off -> no command

		TEST_ASSERT_EQ("light.zero.initial",   (size_t) 0, scope.lines().size());
		TEST_ASSERT_EQ("light.zero.intensity", 0, light.getIntensity());
	}
}

void test_lighting_positive_sends_M5()
{
	ClientScope scope;
	{
		GCodeLighting light(GCodeClient::get());
		light.setIntensity(50);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("light.on.count",     (size_t) 1, lines.size());
		TEST_ASSERT_EQ("light.on.cmd",       std::string("M5"), lines[0]);
		TEST_ASSERT_EQ("light.on.intensity", 50, light.getIntensity());
	}
}

void test_lighting_repeated_positive_does_not_resend()
{
	ClientScope scope;
	{
		GCodeLighting light(GCodeClient::get());
		light.setIntensity(50);
		light.setIntensity(75);
		light.setIntensity(100);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("light.idempotent.count",     (size_t) 1, lines.size());
		TEST_ASSERT_EQ("light.idempotent.cmd",       std::string("M5"), lines[0]);
		TEST_ASSERT_EQ("light.idempotent.intensity", 100, light.getIntensity());
	}
}

void test_lighting_on_then_off_sends_M5_then_M6()
{
	ClientScope scope;
	{
		GCodeLighting light(GCodeClient::get());
		light.setIntensity(80);
		light.setIntensity(0);

		std::vector<std::string> lines = scope.lines();
		TEST_ASSERT_EQ("light.toggle.count", (size_t) 2, lines.size());
		TEST_ASSERT_EQ("light.toggle.on",    std::string("M5"), lines[0]);
		TEST_ASSERT_EQ("light.toggle.off",   std::string("M6"), lines[1]);
	}
}

void test_lighting_clamps_negative_and_above_100()
{
	ClientScope scope;
	{
		GCodeLighting light(GCodeClient::get());

		light.setIntensity(-10);
		TEST_ASSERT_EQ("light.clamp.neg", 0, light.getIntensity());

		light.setIntensity(250);
		TEST_ASSERT_EQ("light.clamp.high", 100, light.getIntensity());
	}
}

}

int main()
{
	TEST_RUN("turntable setMotorEnabled(true) -> M17",  test_turntable_setMotorEnabled_true_sends_M17);
	TEST_RUN("turntable setMotorEnabled(false) -> M18", test_turntable_setMotorEnabled_false_sends_M18);
	TEST_RUN("turntable rotate(2pi) -> full step count",test_turntable_rotate_full_revolution_sends_all_steps);
	TEST_RUN("turntable rotate(<=0) is a no-op",        test_turntable_rotate_zero_or_negative_is_noop);
	TEST_RUN("turntable rotate respects invertDirection", test_turntable_rotate_inverts_direction_when_configured);

	TEST_RUN("laser right on -> M21", test_laser_right_on_sends_M21);
	TEST_RUN("laser left on  -> M19", test_laser_left_on_sends_M19);
	TEST_RUN("laser right off -> M22", test_laser_right_off_sends_M22);
	TEST_RUN("laser left off  -> M20", test_laser_left_off_sends_M20);
	TEST_RUN("laser ALL on  -> M19 then M21", test_laser_all_on_sends_both_left_then_right);
	TEST_RUN("laser ALL off -> M20 then M22", test_laser_all_off_sends_both_off);

	TEST_RUN("lighting setIntensity(0) when off is a no-op", test_lighting_zero_sends_M6_only_on_transition);
	TEST_RUN("lighting setIntensity(50) -> M5",              test_lighting_positive_sends_M5);
	TEST_RUN("lighting positive is idempotent",              test_lighting_repeated_positive_does_not_resend);
	TEST_RUN("lighting on then off -> M5 then M6",           test_lighting_on_then_off_sends_M5_then_M6);
	TEST_RUN("lighting clamps out-of-range intensities",     test_lighting_clamps_negative_and_above_100);

	return freelss_test::reportAndExit();
}
