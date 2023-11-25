#ifndef _JSONH
#define _JSONH
#include "Lib.h"
#include <vector>

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
extern Vector3 playerModulate;

extern std::string instanceUnlitTexPath;
extern std::string instanceWorkingTexPath;
extern std::string instanceBrokenTexPath;
extern std::vector<std::vector<int>> instanceData;

extern bool parsedMap;
void loadMapData(const std::string& mapName);
#endif// JSONH