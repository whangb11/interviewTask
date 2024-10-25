#include<algorithm>
#include<fstream>
#include<iostream>
#include<map>
#include<memory>
#include<string>
#include<vector>

constexpr auto TARGET_FILE_PATH = "./target";
constexpr auto MATCHING_STR = "qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM1234567890";


class Tag {
public:
	std::string name;
	std::vector<std::pair<std::string, std::string>> keys;
	std::vector<Tag> children;
	std::string value;


	Tag() = delete;

	Tag(const std::string& name) :name(name) {
		keys = {};
		children = {};
		value = "";
	}

	void dump(int indent=0) {
		for (auto i = 0; i < indent; ++i) {
			std::cout << "        ";
		}
		std::cout << name;
		for (const auto& key : keys) {
			std::cout << "-" << key.first << ":" << key.second ;
		}
		if (value.size() > 0) {
			std::cout << "-value:\"" << value << "\"";
		}
		
		std::cout << std::endl;
		auto it = children.rbegin();
		while (it!=children.rend())
		{
			it->dump(indent + 1);
			it++;
		}
	}
};

//no syntax check
class Parser {
	std::vector<Tag*> stack;
public:
	Tag dofile(const std::string& path) {
		Tag root("root");
		
		//read
		std::string instring;
		std::ifstream f;
		f.open(path, std::ios::in);
		if (!f) {
			std::cerr << "failed to open file at: " + path << std::endl;
			return root;
		}

		std::string tmp;
		while (std::getline(f,tmp))
		{
			instring += tmp + "\n";
		}

		//process
		processString(instring, root);
		return root;
	}


private:
	void processString(const std::string& instring, Tag& father) {
		//findNextTagHeader
		auto tBeginPos = instring.find_first_of("(");
		auto tEndPos = instring.find_first_of(")");

		//father constents
		if (tBeginPos != 0) {
			auto fVRawStr = instring.substr(0, tBeginPos);
			auto fVBeginPos = fVRawStr.find_first_of(MATCHING_STR);
			auto fVEndPos = fVRawStr.find_last_of(MATCHING_STR);
			if (fVBeginPos != std::string::npos) {
				auto fValue = fVRawStr.substr(fVBeginPos, fVEndPos - fVBeginPos + 1);
				father.value = fValue;
			}
			
		}

		//no tags found
		if (tBeginPos == std::string::npos) {
			return;
		}

		auto tagContent = instring.substr(tBeginPos + 1, tEndPos - tBeginPos - 1);

		//name
		std::string tName = "";
		for (const auto& ch : tagContent) {
			if (std::isalpha(ch) || isdigit(ch)) {
				tName += ch;
			}
			else {
				break;
			}
		}
		
		Tag newTag(tName);
		
		if (instring[tEndPos - 1] == '/') {//self close
			processTagKeys(tagContent,newTag);
			auto nextC = instring.substr(tEndPos+1);
			if (nextC.size() > 0) {
				processString(nextC, father);
			}
		}
		else {//non self close
			processTagKeys(tagContent, newTag);
			auto tTail = "(/" + tName + ")";
			auto tTailPos = instring.find(tTail);
			auto tagInnerString = instring.substr(tEndPos + 1, tTailPos - tEndPos - 1);
			processString(tagInnerString, newTag);
			auto nextC = instring.substr(tTailPos + tTail.size());
			if (nextC.size() > 0) {
				processString(nextC, father);
				//std::cout << nextC<<std::endl;
			}
		}
		father.children.push_back(newTag);
		
	}

	void processTagKeys(std::string tagstring,Tag& tag) {
		while (tagstring.find("key") != std::string::npos)
		{
			auto keyBeginPos = tagstring.find("key");
			auto keyEndPos = tagstring.find("\"",tagstring.find("\"") + 1);
			auto keyStr = tagstring.substr(keyBeginPos, keyEndPos-keyBeginPos+1);

			auto eqPos = keyStr.find("=");
			auto keyname = keyStr.substr(0,eqPos);
			auto keyvalue = keyStr.substr(eqPos + 1);
			tag.keys.push_back(std::make_pair(keyname, keyvalue));
			tagstring = tagstring.substr(keyEndPos + 1);
		}
	}
};

int main() {
	Parser parser;
	auto result = parser.dofile(TARGET_FILE_PATH);
	result.children.begin()->dump();
	return 0;
}