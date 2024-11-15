#pragma once

#include <expected>

#ifdef _DEBUG
#define DLL_PUBLIC
#else
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif

//
