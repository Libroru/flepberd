#pragma once

#include "raylib.h"
#include "rres.h"
#include "rres-raylib.h"

Sound RegisterSound(rresCentralDir centralDir, const char* resourceFile, const char* soundName);
Image RegisterImage(rresCentralDir centralDir, const char* resourceFile, const char* imageName);