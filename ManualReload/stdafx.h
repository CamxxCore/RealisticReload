#pragma once
#define __MAJOR_REV__ 1.0
#define __MINOR_REV__ .04

#define APP_VERSION (__MAJOR_REV__ + __MINOR_REV__)

#define MAX_STRING 256

#include <windows.h>

#include <cstdio>
#include <vector>
#include <fstream>
#include <memory>
#include <string>

#include "resource.h"

#include "Utility/General.h"
#include "Utility/Logger.h"
#include "Utility/Hooking.h"
#include "Utility/pattern.h"
#include "Utility/patch.h"
#include "Utility/config.h"

#include "types.h"
#include "strings.h"

#pragma region Scripthook Includes

#include "inc/types.h"
#include "inc/enums.h"
#include "inc/natives.h"
#include "inc/main.h"

#pragma endregion

void main();

void run();

void unload();
