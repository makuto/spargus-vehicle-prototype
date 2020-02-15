#pragma once

#include "Tracy.hpp"
#include "common/TracyColor.hpp"
// For manual zones, when scoping isn't an option
#include "TracyC.h"

#ifdef TRACY_ENABLE
#define PERF_ENABLE
#endif

#define PerfTimeScope ZoneScoped

#define PerfTimeNamedScope(uniqueVarName, scopeString, hexColor) \
	ZoneNamedNC(uniqueVarName, scopeString, hexColor, true)
// Add detail strings to current scope
#define PerfAddText(scopeVar, text, size) scopeVar.Text(text, size)
// Rename the scope with the given format. Note that this preserves Tracy filtering
// Usage example: PerfSetNameFormat(myLoadScope, "Load %s", filename);
#define PerfSetNameFormat(scopeVar, format, ...)                                 \
	{                                                                            \
		char perfNameBuffer[128];                                                \
		snprintf(perfNameBuffer, sizeof(perfNameBuffer), format, ##__VA_ARGS__); \
		scopeVar.Name(perfNameBuffer, sizeof(perfNameBuffer));                   \
	}

// When scoping isn't an option
#define PerfManualZoneBegin(contextName, name, color) TracyCZoneNC(contextName, name, color, true)
#define PerfManualZoneEnd(contextName) TracyCZoneEnd(contextName)

#define PerfEndFrame FrameMark

// Logging
#define PerfLog(text, size) TracyMessage(text, size)

// Locks
// TODO: These don't quite work yet
// #define Mutex(lockVariableName) TracyLockable(std::mutex, lockVariableName)
// #define LockGuard(lockVariableName) std::lock_guard<LockableBase(std::mutex)>
// lockVariableName##_guard(lockVariableName)
