#include "Lib.h"
void loadMapData(const std::string& mapName);
bool startsWith(std::string str, std::string start);
std::string stringCut(std::string* str, int num);
void cutEmptySpace(std::string* str);
void cutSemicolon(std::string* str);
void cutComma(std::string* str);
std::string parseString(std::string* json);
float parseFloat(std::string* json);