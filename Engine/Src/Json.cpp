#include "Json.h"
void loadMapData(const std::string& mapName) {
	std::string data;
	FsReadDiskFile(&data, "Data/" + mapName + ".json");
	cutEmptySpace(&data);
	if(!startsWith(data, "{")) return;
	stringCut(&data, 1);

	Log(parseString(&data));//key
	cutSemicolon(&data);
	Log(parseFloat(&data));//value
	cutComma(&data);
	Log(parseString(&data));//key
	cutSemicolon(&data);
	Log(parseFloat(&data));//value
	cutComma(&data);
	Log(parseString(&data));//key
	cutSemicolon(&data);
	Log(parseString(&data));//value
}
bool startsWith(std::string str, std::string start) {
	//Log(std::to_string(0) + ", " + std::to_string(std::min((int)start.size(), (int)str.size())) + ": \"" + str.substr(0, std::min((int)start.size(), (int)str.size())) + "\" == \"" + start + "\", " + ((str.substr(0, std::min((int)start.size(), (int)str.size())) == start)?"True":"False"));
	return str.substr(0, std::min((int)start.size(), (int)str.size())) == start;
}
std::string stringCut(std::string* str, int num) {
	int len=std::min(num, (int)str->size());
	std::string tmp=str->substr(0, len);
	(*str)=str->substr(len, str->size());
	return tmp;
}
void cutEmptySpace(std::string* str) {
	while((startsWith(*str, " ") || startsWith(*str, "\n") ||
		startsWith(*str, "\r") || startsWith(*str, "\t") ||
		startsWith(*str, "\b")) && str->size() > 0
		) {
		stringCut(str, 1);
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
	if(!startsWith(*json, "\"")) return "";
	stringCut(json, 1);
	std::string str="";
	while(!startsWith(*json, "\"") && !startsWith(*json, "\'")) {
		str+=stringCut(json, 1);
	}
	stringCut(json, 1);
	return "\""+str+"\"";
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