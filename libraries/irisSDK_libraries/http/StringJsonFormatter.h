#pragma once

#include <string>
#include <vector>

/**
 *  Does not support the full list of json features. Only supports 
 *  string fields. Is basically a glorified vector, containing pairs of strings.
 */

class StringJsonFormatter {
public:
	static std::wstring create_json_wstring(std::vector<std::pair<std::wstring,std::wstring>> params) {
		using namespace std;

		wstring out = L"[{";

		for (int i = 0; i < params.size(); i++) {
			if (i > 0) out.append(L",");

			out.append(L"\"" + params[i].first + L"\":\"" + params[i].second + L"\"");
		}

		out.append(L"}]");

		return out;
	}
	
	static std::string create_json_string(std::vector<std::pair<std::string,std::string>> params) {
		using namespace std;

		string out = "{";

		for (int i = 0; i < params.size(); i++) {
			if (i > 0) out.append(",");

			out.append("\"" + params[i].first + "\":\"" + params[i].second + "\"");
		}

		out.append("}");

		return out;
	}
};