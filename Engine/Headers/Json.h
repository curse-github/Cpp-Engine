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
extern Vector2 playerHitbox;
extern float playerSpeed;
extern float playerSprintSpeed;
extern std::string playerTexPath;
extern Vector3 playerModulate;
extern std::string flashlightTexPath;
extern Vector3 flashlightColor;
extern float flashlightRange;

extern std::string instanceUnlitTexPath;
extern std::string instanceWorkingTexPath;
extern std::string instanceBrokenTexPath;
extern float instanceBrokenChance;
extern std::vector<std::vector<int>> instanceData;

extern std::vector<Vector3> horizontalWallData;
extern std::vector<Vector3> verticalWallData;

extern bool parsedMap;
void loadMapData(const std::string& mapName);
#endif// JSONH