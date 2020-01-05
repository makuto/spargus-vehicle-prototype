#include "Logging.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

namespace Logging
{
Record::Record(Severity newSeverity, const char* func, size_t line, const char* file)
    : severity(newSeverity), Function(func), Line(line), File(file), Offset(0)
{
}

Record& Record::operator<<(char data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%d", (int)data);
	return *this;
}

Record& Record::operator<<(void* data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%p", data);
	return *this;
}

Record& Record::operator<<(const char* data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%s", data);
	return *this;
}

Record& Record::operator<<(const int data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%d", data);
	return *this;
}

Record& Record::operator<<(const unsigned int data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%u", data);
	return *this;
}

Record& Record::operator<<(const float data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%f", data);
	return *this;
}

Record& Record::operator<<(const double data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%f", data);
	return *this;
}

Record& Record::operator<<(const bool data)
{
	Offset +=
	    snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%s", data ? "True" : "False");
	return *this;
}

Record& Record::operator<<(const size_t data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "%lu", (unsigned long)data);
	return *this;
}

Record& Record::operator<<(const glm::mat4& data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset,
	                   "\n\t[%f, %f, %f, %f\n"
	                   "\t%f, %f, %f, %f\n"
	                   "\t%f, %f, %f, %f\n"
	                   "\t%f, %f, %f, %f]",
	                   data[0][0], data[0][1], data[0][2], data[0][3], data[1][0], data[1][1],
	                   data[1][2], data[1][3], data[2][0], data[2][1], data[2][2], data[2][3],
	                   data[3][0], data[3][1], data[3][2], data[3][3]);
	return *this;
}

Record& Record::operator<<(const glm::vec3& data)
{
	Offset += snprintf(OutBuffer + Offset, sizeof(OutBuffer) - Offset, "(%f, %f, %f)", data[0],
	                   data[1], data[2]);
	return *this;
}

void FormatFuncName(char* buffer, const char* func, size_t bufferSize)
{
	const char* funcBegin = func;
	const char* funcEnd = strchr(funcBegin, '(');

	if (!funcEnd)
	{
		strncpy(buffer, func, bufferSize - 1);
		return;
	}

	for (const char* i = funcEnd - 1; i >= funcBegin;
	     --i)  // search backwards for the first space char
	{
		if (*i == ' ')
		{
			funcBegin = i + 1;
			break;
		}
	}

	unsigned long numCharsToCopy = std::min((funcEnd - funcBegin), (long)bufferSize - 1);
	strncpy(buffer, funcBegin, numCharsToCopy);
	buffer[numCharsToCopy] = '\0';
}

Logger::Logger(Severity maxSeverity, CustomLogOutputFunc customOutputFunc)
    : MaxSeverity(maxSeverity), CustomOutputFunc(customOutputFunc)
{
	Singleton = this;
	// std::cout << "Logger initialized at " << Singleton << "\n";
}
bool Logger::checkSeverity(Severity severity) const
{
	return severity <= MaxSeverity;
}

void Logger::operator+=(const Record& record)
{
	if (CustomOutputFunc)
		CustomOutputFunc(record);
	else
	{
		static char funcNameBuffer[256];
		FormatFuncName(funcNameBuffer, record.Function, sizeof(funcNameBuffer));
		// Emacs supports this line output for quickly jumping to the code that output it
		std::cout << record.File << ":" << record.Line << ": " << funcNameBuffer << "():"
		          << "(" << severityToString(record.severity) << ") " << record.OutBuffer << "\n";
	}
}

Logger* Logger::Singleton = nullptr;

Logger* Logger::GetSingleton()
{
	// TODO: @Stability: This could be a problem. We can't print yet though...
	// if (!Singleton)
	// 	std::cout << "Warning: something tried to access Logger before any Logger had been "
	// 	             "initialized!\n";
	return Singleton;
}

void MinimalLogOutput(const Record& record)
{
	static char funcNameBuffer[256];
	FormatFuncName(funcNameBuffer, record.Function, sizeof(funcNameBuffer));
	std::cout << funcNameBuffer << "(): " << record.OutBuffer << "\n";
}

}  // namespace Logging
