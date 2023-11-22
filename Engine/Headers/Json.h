#ifndef _JSONH
#define _JSONH
#include "Lib.h"
extern Vector2 mapSize;
extern float mapScale;
extern float spacing;
extern std::string mapTexPath;
extern std::string minimapTexPath;
extern Vector2 fullMapSize;

extern Vector2 playerOffset;
extern float playerSize;
extern float playerSpeed;
extern float playerSprintSpeed;
extern std::string playerTexPath;
extern Vector4 playerModulate;

extern bool parsedMap;
void loadMapData(const std::string& mapName);
#endif// JSONH