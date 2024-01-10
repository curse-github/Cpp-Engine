#ifndef _JSON_H
#define _JSON_H
#include "EngineLib.h"
#include <vector>
#include <array>

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
extern Vector4 flashlightColor;
extern float flashlightRadius;

extern float enemySpeed;
extern std::string enemyTexPath;
extern Vector3 enemyModulate;

extern std::string instanceUnlitTexPath;
extern std::string instanceWorkingTexPath;
extern std::string instanceBrokenTexPath;
extern float instanceBrokenChance;
extern std::vector<std::array<int, 5>> instanceData;

extern std::vector<Vector3> horizontalWallData;
extern std::vector<Vector3> verticalWallData;

extern bool parsedMap;
void loadMapData(const std::string& mapName);
#endif// _JSON_H