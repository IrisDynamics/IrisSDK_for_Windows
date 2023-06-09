#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <tuple>
#include <regex>

#include <iostream>

std::string escape_quotes(std::string);

class MalformedJsonError : public std::runtime_error {
public:
	MalformedJsonError(std::string _what):
		runtime_error(_what) 
	{}
};


class Json {
public:
	class JsonObject;
	class JsonArray;

	/** @brief	Various JSON types and structs */
	enum class JsonValueTag {
		t_object,
		t_array,
		t_string,
		t_number,
		t_true,
		t_false,
		t_null,
		t_unknown
	};

	/** 
	 *	@brief	The object representing a JSON element. This is the object that the user should be interacting
	 *			with the most. A JSON element can be any of the following:
	 *				-Object
	 *				-Array
	 *				-String
	 *				-Number
	 *				-Bool
	 *				-Null
	 *			A JSON element can also contain an unknown value in the case of default initialization, but this 
	 *			value is illegal according to the JSON specification and operations done on such an element are not
	 *			defined.
	 */	
	class JsonElement {
		std::string str;
		std::shared_ptr<JsonObject> obj;
		std::shared_ptr<JsonArray> arr;

		JsonValueTag tag;
	public:

		/** 
		 *	@brief	Default constructor. Creates a JSON element with an unknown type. Should be transformed into
		 *			a valid JSON element before being operated on.
		 */
		JsonElement() {
			tag = JsonValueTag::t_unknown;
		}

		/**
		 *	@brief	Constructor with user defined tag. Necessary for storing a number element from string without
		 *			defined type and without losing information. Also necessary for constructing null elements.
		 */
		JsonElement(std::string _str, JsonValueTag _tag = JsonValueTag::t_string) :
			str(_str),
			tag(_tag)
		{}

		JsonElement(JsonObject _obj) {
			obj = std::make_shared<JsonObject>(_obj);
			tag = JsonValueTag::t_object;
		}

		JsonElement(JsonArray _arr) {
			arr = std::make_shared<JsonArray>(_arr);
			tag = JsonValueTag::t_array;
		}

		JsonElement(bool _bool) {
			str = _bool ? "true" : "false";
			tag = _bool ? JsonValueTag::t_true : JsonValueTag::t_false;
		}

		JsonElement(const char* c_str) {
			str = c_str;
			tag = JsonValueTag::t_string;
		}

		// These are effectively the same. See if there's a way to contract these. I still don't know how template functions work
		JsonElement(int _i) {
			str = std::to_string(_i);
			tag = JsonValueTag::t_number;
		}
		JsonElement(double _d) {
			str = std::to_string(_d);
			tag = JsonValueTag::t_number;
		}
		JsonElement(float _f) {
			str = std::to_string(_f);
			tag = JsonValueTag::t_number;
		}

		explicit operator int() {
			switch (tag) {
			case JsonValueTag::t_number:
				return stoi(str);

			default:
				throw std::runtime_error("Tried to convert non-number JSON element to int");
			}
		}

		explicit operator float() {
			switch (tag) {
			case JsonValueTag::t_number:
				return stof(str);

			default:
				throw std::runtime_error("Tried to convert non-number JSON element to float");
			}
		}

		explicit operator bool() {
			switch (tag) {
			case JsonValueTag::t_true:
				return true;

			case JsonValueTag::t_false:
				return false;

			default:
				throw std::runtime_error("Tried to convert non bool JSON element to bool");
			}
		}

		/**	@brief	Proxy for get() with the same signature. */
		JsonElement operator[](std::string str) {
			return get(str);
		}

		/**
		 *	@brief	If the object is a JSON object, returns the value from the key-value pair
		 *			with key matching <key>. If the element being searched isn't a JSON object,
		 *			throws an std::runtime_error.
		 *	@param[in]	A string containing the key for the desired key-value pair.
		 *	@returns	The value from the key-value pair matching <key>.
		 */
		JsonElement get(std::string key) {
			if (tag == JsonValueTag::t_object) {
				return obj->get(key);
			}
			else {
				throw std::runtime_error("Tried to access a key value pair from a non object Json element");
			}
		}

		/**
		 *	@brief	If the object is a JSON object, returns true if the object contains a key-value
		 *			pair with key matching <key>, and false otherwise. If the element being searched 
		 *			isn't a JSON object, throws an std::runtime_error.
		 *	@param[in]	A string containing the key for the desired key-value pair.
		 *	@returns	True if key-value pair exists, and false otherwise.
		 */
		bool contains(std::string key) {

			if (tag == JsonValueTag::t_object) {
				return obj->contains(key);
			}
			else {
				throw std::runtime_error("Tried to access a key value pair from a non object Json element");
			}
		}

		/**	@brief	Proxy for get() with the same signature. */
		JsonElement operator[](int index) {
			return at(index);
		}

		/**
		 *	@brief	If the element is a JSON array, returns the element stored at index <index>
		 *			of the array. If the element being searched isn't a JSON array, throws an 
		 *			std::runtime_error.
		 *	@param[in]	A int containing the index for the desired element.
		 *	@returns	The element at index <index>.
		 */
		JsonElement at(int index) {
			if (tag == JsonValueTag::t_array) {
				return arr->at(index);
			}
			else {
				throw std::runtime_error("Tried to access an array index from a non-array JSON element");
			}
		}

		/** 
		 *	@brief	If this element is a JSON object, pushes a new key-value pair to the end of the 
		 *			object. If this element isn't a JSON object, throws an std::runtime_error.
		 *	@param[in]	std::string key - The key for the new key-value pair.
		 *	@param[in]	JsonElement val - The value for the new key-value pair. A JSON element.
		 */
		void push_back(std::string key, JsonElement val) {
			if (tag == JsonValueTag::t_object) {
				obj->push_back({ {key, JsonValueTag::t_string}, val });
			}
			else {
				throw std::runtime_error("Tried to push a key-value pair to a non-object JSON element");
			}
		}

		/**
		 *	@brief	If this element is a JSON array, pushes a new element to the end of the array. If
		 *			this element isn't a JSON array, throws an std::runtime_error.
		 *	@param[in]	A JSON element. The element to be pushed to the end of the array.
		 */
		void push_back(JsonElement val) {
			if (tag == JsonValueTag::t_array) {
				arr->push_back(val);
			}
			else {
				throw std::runtime_error("Tried to push a json element to a non-array JSON element");
			}
		}

		/**
		 *	@brief	Returns a JSON formatted string containing the current element and all sub-elements
		 *			if this element is an object or an array. Formatted according to the JSON specification
		 */
		std::string to_string() {
			switch (tag) {
			case JsonValueTag::t_object:
				return obj->to_string();
			case JsonValueTag::t_array:
				return arr->to_string();
			case JsonValueTag::t_string:
				return "\"" + escape_quotes(str) + "\"";
			case JsonValueTag::t_unknown:
				throw std::runtime_error("Tried to create string from uninitialized json element");
			default:
				return str;
			}
		}

		/**
		 *	@brief	Returns the raw string stored in this element. Should only be used if trying to access
		 *			the value of a string element without adding the quotation marks.
		 */
		std::string raw_string() {
			return str;
		}

		/**
		 *	@brief	Returns the tag of the current element. The tag indicates which type of JSON element 
		 *			the current element is.
		 */
		JsonValueTag get_tag() {
			return tag;
		}

		int size() {
			switch (tag) {
			case JsonValueTag::t_object:
				return obj->size();
			case JsonValueTag::t_array:
				return arr->size();
			default:
				throw std::runtime_error("Tried to access the size of a non-array and non-object JSON element.");
			}
		}
	};


	/**
	 *	@brief	Class for representing JSON objects. Contains a container of JSON elements stored as key-value pairs.
	 */
	class JsonObject {
		std::vector<std::pair<JsonElement, JsonElement>> kvp_list;
	public:

		JsonObject() {}

		/**
		 *	@brief	Pushes a key-value pair to the end of the JSON object. The key for this pair must
		 *			be a valid JSON string element.
		 *	@param[in]	std::pair of JSON elements. With the first element being a valid JSON string element
		 */
		void push_back(std::pair<JsonElement, JsonElement> kvp) {
			if (kvp.first.get_tag() != JsonValueTag::t_string) throw std::invalid_argument("Tried to push to JSON object using a non-string key");
			kvp_list.push_back(kvp);
		}

		/** @brief	Returns a string containing a JSON formatted representation of the array and its contained elements. */
		std::string to_string() {
			std::string out = "{";

			for (int i = 0; i < kvp_list.size(); i++) {
				if (i > 0) {
					out += ",";
				}

				out += kvp_list[i].first.to_string();
				out += ":";
				out += kvp_list[i].second.to_string();
			}

			out += "}";

			return out;
		}

		/** @brief	Given a string as a key, returns the JSON element with that string as a key in the current object.
		 *			If the key doesn't exist in the object, throws an out_of_range exception.
		 *	@param[in]	A string matching the key of the desired key-value pair.
		 *	@returns	The value of the matching key-value pair. A JSON element.
		 */
		JsonElement get(std::string key) {
			for (auto kvp : kvp_list) {
				if (key == kvp.first.raw_string()) {
					return kvp.second;
				}
			}
			throw std::out_of_range("Tried to find key '" + key + "' in object " + to_string() + ", but key couldn't be found..");
		}

		/** 
		 *	@brief	Given a string as a key, returns true if the current object contains a key-value pair with matching key
		 *			and false otherwise.
		 *	@param[in]	A string matching the key of the desired key-value pair.
		 *	@returns	True if a matching key-value pair exists, and false otherwise.
		 */
		bool contains(std::string key) {
			for (auto kvp : kvp_list) {
				if (key == kvp.first.raw_string()) {
					return true;
				}
			}
			return false;
		}
		
		int size() {
			return kvp_list.size();
		}
	};


	/** 
	 *	@brief	Class for representing JSON arrays. Contains a container of JSON elements.
	 */
	class JsonArray {
		std::vector<JsonElement> arr;
	public:

		JsonArray() {}

		JsonArray(std::vector<JsonElement> _arr) :
			arr(_arr)
		{}

		/** @brief	Adds a JSON element to the end of the container. */
		void push_back(JsonElement val) {
			arr.push_back(val);
		}

		/** @brief	Returns a string containing a JSON formatted representation of the array and its contained elements. */
		std::string to_string() {
			std::string out = "[";

			for (int i = 0; i < arr.size(); i++) {
				if (i > 0) {
					out += ",";
				}

				out += arr[i].to_string();
			}

			out += "]";

			return out;
		}

		/** 
		 *	@brief	Returns the JSON element stored at index <index> of the container.
		 *	@param[in]	The index of the desired element
		 */
		JsonElement at(int index) {
			if (index > arr.size()) throw std::out_of_range("Tried to access an index out of range of a JSON array");
			return arr[index];
		}

		int size() {
			return arr.size();
		}
	};


public:
	/** 
	 *	@brief	Given a string containing well formatted JSON, parses and returns an interactable object
	 *			representation of that string.
	 *	@param[in]	A string containing well formatted JSON that should be parsed.
	 *	@returns	The root element of the resulting interactable JSON object.
	 */
	JsonElement parse(std::string str) {
		return parse_next_item(str, 0).first;
	}


private:
	/// Parsing ///

	//Signatures/prefixes
	const std::string string_signature = "\"";
	const std::string true_signature = "true";
	const std::string false_signature = "false";
	const std::string null_signature = "null";
	const std::string object_signature = "{";
	const std::string array_signature = "[";

	const std::string valid_number_prefixes = "0123456789-";
	
	//For parsing control
	const std::string valid_number_characters = "0123456789.+-eE";

	/**
	 *	Each of the parse_<value> methods assume that the index provided is at exactly the start
	 *  location of the method's respective signature. Calling these methods without satisfying 
	 *  this condition may lead to broken or incomplete output. See parse_next_item()
	 */

	std::pair<JsonElement, size_t> parse_string(std::string str, size_t index) {
		size_t next_index = index;

		std::pair<JsonElement, size_t> out;

		do {
			next_index = str.find(string_signature, next_index + 1);
			if (next_index == std::string::npos) {
				throw MalformedJsonError("Encountered open quotation mark without corresponding closing quotation mark");
			}
			if (str.at(next_index - 1) == '\\') {
				//We encountered an escaped quotation mark
				continue;
			}
			//If we got here we found an unescaped closing quotation mark
			size_t start_of_string = index + 1;
			out.first = {str.substr(start_of_string, next_index - start_of_string), JsonValueTag::t_string}; //Don't store the quotation marks, we will reinsert them in conversion back to string
			next_index += 1; //Set string index beyond closing quotation mark
			out.second = next_index;
			break;
		} while (1); //Just need some loop structure for the continue, but could use a helper function
		
		return out;
	}

	std::pair<JsonElement, size_t> parse_number(std::string str, size_t index) {
		size_t end_of_number_index = str.find_first_not_of(valid_number_characters, index);
		std::string number_str = str.substr(index, end_of_number_index - index);
		if (!is_valid_json_number(number_str)) {
			throw MalformedJsonError("Encountered a number with invalid syntax");
		}
		return { {number_str, JsonValueTag::t_number}, end_of_number_index };
	}

	bool is_valid_json_number(std::string str) {
		// This regex should match only numbers as defined in RFC8259
		return std::regex_match(str, std::regex("^-?([1-9][0-9]*|0)(\\.[0-9]+)?([eE][+-]?[0-9]+)?$"));
	}

	std::pair<JsonElement, size_t> parse_object(std::string str, size_t index) {
		JsonObject obj;

		size_t curr_index = index + 1; // +1 for starting beyond the opening curly brace

		bool first_loop = true;

		do {
			curr_index = find_next_non_whitespace(str, curr_index);
			if (str.at(curr_index) == '}') {
				// Encountered end of object
				break;
			}

			if (!first_loop) {
				if (str.at(curr_index) != ',') {
					throw MalformedJsonError("Encountered JSON object items not separated by comma.");
				}
				curr_index++;
			}

			auto kvp = parse_object_kvp(str, curr_index);
			obj.push_back({ std::get<0>(kvp), std::get<1>(kvp) });
			curr_index = std::get<2>(kvp); 
			first_loop = false;
		} while (1);

		curr_index++; // +1 for moving beyond the closing curly brace

		return { {obj} , curr_index };
	}

	std::tuple<JsonElement, JsonElement, size_t> parse_object_kvp(std::string str, size_t index) {
		std::pair<JsonElement, size_t> next_item_key = parse_next_item(str, index);

		JsonElement key = next_item_key.first;
		size_t curr_index = next_item_key.second;

		if (key.get_tag() != JsonValueTag::t_string) {
			throw MalformedJsonError("Tried to read the key from the key-value-pair of a json object, but it wasn't a string.");
		}

		// parse_next_item() should give an index with no whitespace, but just to be safe. 
		// Remove this if this seems too redundant and confusing
		curr_index = find_next_non_whitespace(str, curr_index);
		if (str.at(curr_index) != ':') {
			throw MalformedJsonError("Key and value in object kvp are not separated by a colon.");
		}

		curr_index++; // Increment to get past the colon 
		std::pair<JsonElement, size_t> next_item_val = parse_next_item(str, curr_index);

		JsonElement val = next_item_val.first;
		curr_index = next_item_val.second;

		return { key, val, curr_index };
	}

	std::pair<JsonElement, size_t> parse_array(std::string str, size_t index) {
		JsonArray arr;

		size_t curr_index = index + 1; // +1 for starting beyond the opening bracket

		bool first_loop = true;

		do {
			curr_index = find_next_non_whitespace(str, curr_index);
			if (str.at(curr_index) == ']') {
				// Encountered end of array
				break;
			}

			if (!first_loop) {
				if (str.at(curr_index) != ',') {
					throw MalformedJsonError("Encountered JSON object items not separated by comma.");
				}
				curr_index++;
			}

			std::pair<JsonElement, size_t> next_item = parse_next_item(str, curr_index);
			arr.push_back(next_item.first);
			curr_index = next_item.second;
			first_loop = false;
		} while (1);

		curr_index++; // +1 for moving beyond the closing curly brace

		return { {arr} , curr_index };
	}

	std::pair<JsonElement, size_t> parse_next_item(std::string str, size_t index) {
		index = find_next_non_whitespace(str, index);

		std::pair<JsonElement, size_t> out = { {} , std::string::npos };

		if (index >= str.length()) return out;

		if (matches_at(str, index, string_signature)) {
			out = parse_string(str, index);
		}
		//Keep in mind if any of these characters are a prefix of a signature, they will interrupt that signature
		//Not the case right now, but it is possible.
		else if (matches_any_at(str, index, valid_number_prefixes)) {
			out = parse_number(str, index);
		}
		else if (matches_at(str, index, true_signature)) {
			out = { true , index + true_signature.length() };
		}
		else if (matches_at(str, index, false_signature)) {
			out = { false, index + false_signature.length() };
		}
		else if (matches_at(str, index, null_signature)) {
			out = { { null_signature, JsonValueTag::t_null }, index + null_signature.length() };
		}
		else if (matches_at(str, index, object_signature)) {
			out = parse_object(str, index);
		}
		else if (matches_at(str, index, array_signature)) {
			out = parse_array(str, index);
		}
		else {
			throw MalformedJsonError("Tried to parse JSON value but encountered illegal char");
		}

		out.second = find_next_non_whitespace(str, out.second);
		return out;
	}

	size_t find_next_non_whitespace(std::string str, size_t index) {
		if (index > str.length()) throw MalformedJsonError("Tried to start a non-whitespace char search beyond the end of the string");

		auto non_ws = std::find_if_not(str.begin() + index, str.end(), [](unsigned char c) { return std::isspace(c); });

		size_t out_index = std::distance(str.begin(), non_ws);

		if (out_index == str.length()) return std::string::npos;

		return out_index;
	}

	/** Compares <str1> starting at <index> to see if the next <str>.length() characters match <str2> */
	bool matches_at(std::string str1, size_t index, std::string str2) {
		if (index + str2.length() > str1.length()) return false;
		return str1.compare(index, str2.length(), str2) == 0;
	}

	bool matches_any_at(std::string str, size_t index, std::string char_list) {
		for (char c : char_list) {
			if (str.at(index) == c) {
				return true;
			}
		}
		return false;
	}
};

// This should technically allow exactly one layer of nesting a string of a json object inside a json object. I make no promises, however
/**
 *	@brief	Given a string containing quotation marks, returns the same string with one backslash
 *			prepended before each quotation mark.
 */
std::string escape_quotes(std::string str_to_escape) {
	size_t quote_position = 0;
	std::string out = str_to_escape;
	while ((quote_position = out.find('\"', quote_position)) != std::string::npos) {
		out = out.insert(quote_position, 1, '\\');
		quote_position = out.find('\"', quote_position) + 1; // Skip past the quotation mark and the newly added backslash
	}
	return out;
}