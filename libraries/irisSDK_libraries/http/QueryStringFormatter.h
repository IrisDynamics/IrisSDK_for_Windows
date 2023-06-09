#pragma once

#include <string>
#include <vector>

class QueryStringFormatter {
public:
	static std::wstring create_query_string(std::vector<std::pair<std::wstring, std::wstring>> params) {
		using namespace std;

		wstring out = L"?";

		for (int i = 0; i < params.size(); i++) {
			if (i > 0) out.append(L"&");

			out.append(params[i].first + L"=" + params[i].second);
		}

		return out;
	}

	static std::wstring create_query_filter_string(std::vector<std::pair<std::wstring, std::wstring>> params) {
		using namespace std;

		wstring out = L"[{";

		for (int i = 0; i < params.size(); i++) {
			if (i > 0) out.append(L",");

			out.append(L"\"" + params[i].first + L"\":\"" + params[i].second + L"\"");
		}

		out.append(L"}]");

		return out;
	}

};