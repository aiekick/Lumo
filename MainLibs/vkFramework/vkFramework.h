#pragma once
#include <memory>
#include <ctools/cTools.h>

class Texture2D;
typedef std::shared_ptr<Texture2D> Texture2DPtr;
typedef ct::cWeak<Texture2D> Texture2DWeak;
