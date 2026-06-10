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

#include "TestHarness.h"
#include "FakeByteStream.h"

#include "../GCodeClient.h"

using freelss::FakeByteStream;
using freelss::GCodeClient;

namespace
{

void test_send_returns_ok_when_ready_arrives()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->queueResponse(">\n");
	GCodeClient::install(stream);

	GCodeClient::SendResult r = GCodeClient::get()->send("M17", 100);
	TEST_ASSERT("send_ok", r == GCodeClient::SEND_OK);

	const std::vector<std::string> lines = stream->writtenLines();
	TEST_ASSERT_EQ("send_ok.write_count", (size_t) 1, lines.size());
	TEST_ASSERT_EQ("send_ok.write_line",  std::string("M17"), lines[0]);

	GCodeClient::release();
}

void test_send_terminates_line_with_newline()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->queueResponse(">\n");
	GCodeClient::install(stream);

	GCodeClient::get()->send("M21", 100);

	TEST_ASSERT_EQ("send_newline.payload", std::string("M21\n"), stream->written());

	GCodeClient::release();
}

void test_send_ignores_chatter_before_ack()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->queueResponse("echo: M21\r\nok\r\n>\n");
	GCodeClient::install(stream);

	GCodeClient::SendResult r = GCodeClient::get()->send("M21", 100);
	TEST_ASSERT("send_chatter", r == GCodeClient::SEND_OK);

	GCodeClient::release();
}

void test_send_returns_timeout_when_no_ack()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->queueResponse("echo: M21\r\n");
	GCodeClient::install(stream);

	GCodeClient::SendResult r = GCodeClient::get()->send("M21", 30);
	TEST_ASSERT("send_timeout", r == GCodeClient::SEND_TIMEOUT);

	GCodeClient::release();
}

void test_send_returns_io_error_on_write_failure()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->setFailNextWrite(true);
	GCodeClient::install(stream);

	GCodeClient::SendResult r = GCodeClient::get()->send("M21", 100);
	TEST_ASSERT("send_io_error", r == GCodeClient::SEND_IO_ERROR);
	TEST_ASSERT("send_io_error.disconnected", !GCodeClient::get()->isConnected());

	GCodeClient::release();
}

void test_send_returns_disconnected_when_stream_closed()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->setOpen(false);
	GCodeClient::install(stream);

	GCodeClient::SendResult r = GCodeClient::get()->send("M21", 100);
	TEST_ASSERT("send_disconnected", r == GCodeClient::SEND_DISCONNECTED);

	GCodeClient::release();
}

void test_ping_uses_M100()
{
	FakeByteStream * stream = new FakeByteStream();
	stream->queueResponse(">\n");
	GCodeClient::install(stream);

	TEST_ASSERT("ping.ok", GCodeClient::get()->ping(100));
	const std::vector<std::string> lines = stream->writtenLines();
	TEST_ASSERT_EQ("ping.count",  (size_t) 1, lines.size());
	TEST_ASSERT_EQ("ping.command", std::string("M100"), lines[0]);

	GCodeClient::release();
}

}

int main()
{
	TEST_RUN("send returns OK when ready arrives",          test_send_returns_ok_when_ready_arrives);
	TEST_RUN("send terminates line with newline",           test_send_terminates_line_with_newline);
	TEST_RUN("send ignores chatter before ack",             test_send_ignores_chatter_before_ack);
	TEST_RUN("send returns TIMEOUT when no ack",            test_send_returns_timeout_when_no_ack);
	TEST_RUN("send returns IO_ERROR on write failure",      test_send_returns_io_error_on_write_failure);
	TEST_RUN("send returns DISCONNECTED when stream closed", test_send_returns_disconnected_when_stream_closed);
	TEST_RUN("ping sends M100",                              test_ping_uses_M100);

	return freelss_test::reportAndExit();
}
