#include "Json.h"
bool startsWith(std::string str, std::string start);
std::string stringCut(std::string* str, int num);
void cutComment(std::string* str);
void cutEmptySpace(std::string* str);
void cutSemicolon(std::string* str);
void cutComma(std::string* str);
std::string parseString(std::string* json);
float parseFloat(std::string* json);
Vector2 parseVector2(std::string* json);
Vector3 parseVector3(std::string* json);
Vector4 parseVector4(std::string* json);

Vector2 mapSize;
float mapScale;
float spacing;
std::string mapTexPath;
std::string minimapTexPath;
Vector2 fullMapSize;

Vector2 playerOffset;
float playerSize;
float playerSpeed;
float playerSprintSpeed;
std::string playerTexPath;
Vector4 playerModulate;

bool parsedMap;

void loadMapData(const std::string& mapName) {
	parsedMap=false;
	std::string data;
	FsReadDiskFile(&data, "Data/" + mapName + ".json");
	cutEmptySpace(&data);
	if(!startsWith(data, "{")) return;
	stringCut(&data, 1);
	cutEmptySpace(&data);
	while(!startsWith(data, "}")) {
		std::string key=parseString(&data);
		cutSemicolon(&data);
		if(key == "mapSize") {
			mapSize=parseVector2(&data);
		} else if(key == "mapScale") {
			mapScale=parseFloat(&data);
		} else if(key == "spacing") {
			spacing=parseFloat(&data);
		} else if(key == "playerOffset") {
			playerOffset=parseVector2(&data);
		} else if(key == "playerSize") {
			playerSize=parseFloat(&data);
		} else if(key == "playerSpeed") {
			playerSpeed=parseFloat(&data);
		} else if(key == "playerSprintSpeed") {
			playerSprintSpeed=parseFloat(&data);
		} else if(key == "playerTex") {
			playerTexPath=parseString(&data);
		} else if(key == "playerModulate") {
			playerModulate=parseVector4(&data);
		} else if(key == "mapTex") {
			mapTexPath=parseString(&data);
		} else if(key == "minimapTex") {
			minimapTexPath=parseString(&data);
		} else {
			Log("Unknown key \""+key+"\"");
			return;
		}
		cutComma(&data);
		cutEmptySpace(&data);
	}
	if(!startsWith(data, "}")) return;
	stringCut(&data, 1);
	fullMapSize=mapSize * (1 + spacing) * mapScale;
	parsedMap=true;
	return;
}
bool startsWith(std::string str, std::string start) {
	return str.substr(0, std::min((int)start.size(), (int)str.size())) == start;
}
std::string stringCut(std::string* str, int num) {
	int len=std::min(num, (int)str->size());
	std::string tmp=str->substr(0, len);
	(*str)=str->substr(len, str->size());
	return tmp;
}
void cutComment(std::string* str) {
	if(startsWith(*str, "//")) {
		stringCut(str, 2);
		while(!startsWith(*str, "\n")) {
			stringCut(str, 1);
		}
	} else if (startsWith(*str, "/*")) {
		stringCut(str, 2);
		while(!startsWith(*str, "*/")) {
			stringCut(str, 1);
		}
		stringCut(str, 2);
	}
}
void cutEmptySpace(std::string* str) {
	while((startsWith(*str, " ") || startsWith(*str, "\n") ||
		startsWith(*str, "\r") || startsWith(*str, "\t") ||
		startsWith(*str, "\b")) && str->size() > 0
		) {
		stringCut(str, 1);
		if(startsWith(*str, "//") || startsWith(*str, "/*")) {
			cutComment(str);
		}
	}
}
void cutSemicolon(std::string* str) {
	cutEmptySpace(str);
	if(!startsWith(*str, ":")) return;
	stringCut(str, 1);
}
void cutComma(std::string* str) {
	cutEmptySpace(str);
	if(!startsWith(*str, ",")) return;
	stringCut(str, 1);
}
std::string parseString(std::string* json) {
	cutEmptySpace(json);
	if(!startsWith(*json, "\"") && !startsWith(*json, "\'")) return "";
	stringCut(json, 1);
	std::string str="";
	while(!startsWith(*json, "\"") && !startsWith(*json, "\'")) {
		str+=stringCut(json, 1);
	}
	stringCut(json, 1);
	return str;
}
float parseFloat(std::string* json) {
	cutEmptySpace(json);
	std::string str="";
	while(startsWith(*json, "0") || startsWith(*json, "1") ||
		startsWith(*json, "2") || startsWith(*json, "3") ||
		startsWith(*json, "4") || startsWith(*json, "5") ||
		startsWith(*json, "6") || startsWith(*json, "7") ||
		startsWith(*json, "8") || startsWith(*json, "9") ||
		startsWith(*json, ".")) {
		str+=stringCut(json, 1);
	}
	return std::stof(str);
}
Vector2 parseVector2(std::string* json) {
	cutEmptySpace(json);
	if(!startsWith(*json, "[")) return Vector2();
	stringCut(json, 1);
	float x=parseFloat(json);
	cutComma(json);
	float y=parseFloat(json);
	cutEmptySpace(json);
	if(!startsWith(*json, "]")) return Vector2();
	stringCut(json, 1);
	return Vector2(x, y);
}
Vector3 parseVector3(std::string* json) {
	cutEmptySpace(json);
	if(!startsWith(*json, "[")) return Vector3();
	stringCut(json, 1);
	float x=parseFloat(json);
	cutComma(json);
	float y=parseFloat(json);
	cutComma(json);
	float z=parseFloat(json);
	cutEmptySpace(json);
	if(!startsWith(*json, "]")) return Vector3();
	stringCut(json, 1);
	return Vector3(x, y, z);
}
Vector4 parseVector4(std::string* json) {
	cutEmptySpace(json);
	if(!startsWith(*json, "[")) return Vector4();
	stringCut(json, 1);
	float x=parseFloat(json);
	cutComma(json);
	float y=parseFloat(json);
	cutComma(json);
	float z=parseFloat(json);
	cutComma(json);
	float w=parseFloat(json);
	cutEmptySpace(json);
	if(!startsWith(*json, "]")) return Vector4();
	stringCut(json, 1);
	return Vector4(x, y, z, w);
}