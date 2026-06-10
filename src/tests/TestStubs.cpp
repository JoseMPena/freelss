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

#include "../Logger.h"
#include "../Thread.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>

namespace freelss
{

const char * Logger::ENDL = "\n";

Logger::Logger(FILE * fp) : m_fp(fp) {}

Logger& Logger::operator << (const std::string& str) { if (m_fp) fputs(str.c_str(), m_fp); return *this; }
Logger& Logger::operator << (const char * str)       { if (m_fp && str) fputs(str, m_fp);   return *this; }
Logger& Logger::operator << (unsigned long n)        { if (m_fp) fprintf(m_fp, "%lu", n);   return *this; }
Logger& Logger::operator << (long n)                 { if (m_fp) fprintf(m_fp, "%ld", n);   return *this; }
Logger& Logger::operator << (unsigned int n)         { if (m_fp) fprintf(m_fp, "%u", n);    return *this; }
Logger& Logger::operator << (int n)                  { if (m_fp) fprintf(m_fp, "%d", n);    return *this; }
Logger& Logger::operator << (unsigned short n)       { if (m_fp) fprintf(m_fp, "%u", (unsigned int) n); return *this; }
Logger& Logger::operator << (short n)                { if (m_fp) fprintf(m_fp, "%d", (int) n);          return *this; }
Logger& Logger::operator << (unsigned char n)        { if (m_fp) fprintf(m_fp, "%u", (unsigned int) n); return *this; }
Logger& Logger::operator << (char n)                 { if (m_fp) fputc(n, m_fp);            return *this; }
Logger& Logger::operator << (float n)                { if (m_fp) fprintf(m_fp, "%f", (double) n); return *this; }
Logger& Logger::operator << (double n)               { if (m_fp) fprintf(m_fp, "%f", n);    return *this; }

Logger InfoLog(stdout);
Logger ErrorLog(stderr);

Thread::Thread()  : m_stopRequested(false), m_handle(0) {}
Thread::~Thread() {}
void Thread::execute() {}
void Thread::join()    {}
void Thread::stop()    { m_stopRequested = true; }

void Thread::sleep(unsigned long seconds)
{
	::usleep(seconds * 1000000UL);
}

void Thread::usleep(unsigned long microseconds)
{
	::usleep(microseconds);
}

}
