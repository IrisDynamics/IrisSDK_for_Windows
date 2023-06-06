#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector> 
#include <string>
#include <sstream>

/**
 * @brief  A functional-style XML editor that allows reading and writing of XML elements for the purposes of storing and reading configurable properties.
 *         DOES NOT SUPPORT THE FULL EXTENT OF XML FEATURES! This file supports the reading and writing of two types of XML elements:
 *				1. Parent elements: Elements that only contain other elements, but not values. Used to create heirarchical structures of elements
 *				2. Value elements: Elements that only contain text. Used for the purpose of storing values
 *		   This editor makes some simplifying assumptions about XML element structure:
 *				A) Parent elements will not share names with any of their descendants (a path should contain no duplicate names)
 *				B) Elements will not share names with 'sibling' elements (elements with the same parent (or both lacking a parent)) in the same heirarchy
 *              (Names of elements can be shared with elements that do not have a sibling or ancestor relationship with each other)
 *		   Assumption A can be relaxed by switching to a parser that is aware of the current heirarchy during parsing. Assumption B cannot be relaxed for this
 *		   application (No way to differentiate between which of two sibling properties to use. Will always use the first). Additionally this editor assumes that
 *		   the XML file being read is well-formed according to XML specification (Particularly that open tags are always matched with corresponding close tags and tags of different
		   elements are not overlapping). Violation of these assumptions currently leads to undefined behaviour.
 */
class Writable_XML_Editor {
protected:

	struct Element_Pointers {
		size_t start = std::string::npos; // First character of the opening tag
		size_t contents_start = std::string::npos; // First character after the opening tag
		size_t contents_end = std::string::npos; // First character of the closing tag
		size_t end = std::string::npos; // First character after the closing tag

		Element_Pointers() {}

		Element_Pointers(size_t _start, size_t _contents_start, size_t _contents_end, size_t _end) {
			start = _start;
			contents_start = _contents_start;
			contents_end = _contents_end;
			end = _end;
		}
	};

	const char* root_label;
	const char* file_name;
	//Currently the only state that may be shared across method calls. But is only modified explicitly through set_current_path()
	std::vector<std::string> current_path;

	/**
	 *  @brief  Given a string containing an XML file, and an int containing an index in that string. Return the next XML tag found
	 *			and the index that the tag is found at
	 */
	std::pair<std::string, size_t> get_next_tag(std::string file_str, size_t index) {
		size_t tag_start = file_str.find("<", index); // Get the index of the less-than char at the beginning of the next tag
		size_t tag_end = file_str.find(">", tag_start + 1); // Get the index of the greater-than char at the end of this tag
		std::string tag = file_str.substr(tag_start, tag_end - tag_start + 1);
		return { tag, tag_start };
	}

	bool is_closing_tag(std::string tag) {
		return tag.substr(0, 2) == "</";
	}

	bool is_comment_tag(std::string tag) {
		return tag.substr(0, 4) == "<!--";
	}

	bool is_declaration_tag(std::string tag) {
		return tag.substr(0, 2) == "<?";
	}

	/**
	 *	@brief  Given a string, returns an opening XML tag with that string as its name.
	 */
	std::string make_opening_tag(std::string name) { return "<" + name + ">"; }

	/**
	 *	@brief  Given a string, returns an closing XML tag with that string as its name.
	 */
	std::string make_closing_tag(std::string name) { return "</" + name + ">"; }

	/**
	 *	@brief  Given a string containing an opening or closing tag, returns that tag's name
	 */
	std::string element_tag_to_name(std::string tag) {
		if (is_closing_tag(tag)) return tag.substr(2, tag.length() - 3);
		return tag.substr(1, tag.length() - 2);
	}

	/**
	 *	@brief  Recursive XML parser. Searches all children of the tag described by <recursive_current_path>. If a child is an opening tag, calls recursively on that tag. Should not be
	 *			called directly, but rather through find_element()
	 *	@retval If the path to a child tag matches target_path, or if any recursive call returns true, then returns true along with the index to the start of the matched tag.
	 *	@retval If no matching path is found, returns false along with the index after the closing tag of <recursive_current_path>
	 */
	std::pair<bool, size_t> find_element_recursive(std::vector<std::string> target_path, std::string file_str, std::vector<std::string> recursive_current_path, size_t start_index) {
		//Trivial case. If just searching for the root label, just assume string::find() succeeds and return it
		if (target_path.back() == root_label) return { true, file_str.find(root_label, 0) };

		std::pair<std::string, size_t> next_tag;
		std::string next_tag_name;
		size_t current_index = start_index;

		while (current_index != std::string::npos) { // Using this conditional just to protect against infinite loops due to poorly formed XML
			next_tag = get_next_tag(file_str, current_index);
			next_tag_name = element_tag_to_name(next_tag.first);

			if (is_closing_tag(next_tag.first) && next_tag_name == recursive_current_path.back()) {
				//Found the closing tag matching the recursive_current_path of this recursive call. This call failed, so update the index to after this tag and break out and return false
				current_index = next_tag.second + next_tag.first.length();
				break;
			}

			if (is_closing_tag(next_tag.first) || is_comment_tag(next_tag.first) || is_declaration_tag(next_tag.first)) {
				//This tag isn't an opening tag, update the index to after this tag and repeat the loop to look for other opening tags
				current_index = next_tag.second + next_tag.first.length();
				continue;
			}

			//At this point the next tag should be an opening tag

			//Push the next element name onto the recursive_current_path for testing matches and recursive calls
			recursive_current_path.push_back(next_tag_name);

			if (target_path == recursive_current_path) {
				//We've found the tag we're looking for. Return true and the index pointing to the start of this tag
				return { true, next_tag.second };
			}

			//Make recursive call to search the child tag
			std::pair<bool, size_t> next_recursive_call = find_element_recursive(target_path, file_str, recursive_current_path, next_tag.second + next_tag.first.length());

			//Check for success in recursive call
			if (next_recursive_call.first) return next_recursive_call;

			//No match from this recursive call, pop the tag from the path and iterate to look for other opening tags
			recursive_current_path.pop_back();
			current_index = next_recursive_call.second;
		}

		return { false, current_index };
	}

	/**
	 *	@brief  This method evokes the recursive XML parser. Given a path, and a string to search, checks to see if that path exists in that string. If it does,
	 *			returns a struct containing indices for navigating that struct. If it doesn't returns a struct with all values equal to npos
	 */
	Element_Pointers find_element(std::vector<std::string> element_path, std::string file_str, bool absolute_path = false) {
		size_t element_start = std::string::npos;
		size_t element_contents_start = std::string::npos;
		size_t element_contents_end = std::string::npos;
		size_t element_end = std::string::npos;

		if (!absolute_path) element_path.insert(element_path.begin(), current_path.begin(), current_path.end());
		size_t root_label_begin = file_str.find(root_label, 0) + make_opening_tag(root_label).length();

		std::pair<bool, size_t> search_result = find_element_recursive(element_path, file_str, { root_label }, root_label_begin);

		if (search_result.first) {
			//Search was successful, update important indices in the file string for navigating this element
			std::string open_tag = make_opening_tag(element_path.back());
			std::string close_tag = make_closing_tag(element_path.back());

			element_start = search_result.second;
			element_contents_start = element_start + open_tag.length();
			element_contents_end = file_str.find(close_tag, element_contents_start); //This relies on assumption A
			element_end = element_contents_end + close_tag.length();
		}

		return Element_Pointers(element_start, element_contents_start, element_contents_end, element_end);
	}

	std::string read_from_file() {
		std::ifstream input_file;
		std::stringstream buffer;
		input_file.open(file_name, std::ios::in);

		if (input_file.is_open()) {
			buffer << input_file.rdbuf();
		}

		return buffer.str();
	}

	void write_to_file(std::string contents) {
		std::ofstream output_file;
		output_file.open(file_name, std::ios::trunc);

		if (output_file.is_open()) {
			output_file << contents;
			output_file.close();
		}
	}

	//Possibly a slight misnomer. If called, will return false if the pointers' first member is npos, and true otherwise
	bool was_find_successful(Element_Pointers pointers) {
		return pointers.start != std::string::npos;
	}

	bool element_exists(std::vector<std::string> element_path, std::string file_str, bool absolute_path = false) {
		return was_find_successful(find_element(element_path, file_str, absolute_path));
	}

	std::string string_vector_to_string(std::vector<std::string> str_vec, std::string delim = "/") {
		std::string ret_str;

		for (std::string str : str_vec) {
			ret_str += str;
			if (str != str_vec.back()) ret_str += delim;
		}

		return ret_str;
	}

	std::string full_path_string(std::vector<std::string> path) {
		path.insert(path.begin(), current_path.begin(), current_path.end());
		return string_vector_to_string(path);
	}

	bool path_has_duplicate_names(std::vector<std::string> path) {
		//Code referenced from https://stackoverflow.com/questions/46477764/check-stdvector-has-duplicates
		path.insert(path.begin(), current_path.begin(), current_path.end());
		std::sort(path.begin(), path.end());
		return std::adjacent_find(path.begin(), path.end()) != path.end();
	}

	void create_file_if_doesnt_exist() {
		std::ifstream test_fs{file_name};
		if (test_fs.good()) return;

		std::ofstream output_file{ file_name };

		std::string base_content = make_opening_tag(root_label) + "\n" + make_closing_tag(root_label);
		output_file << base_content;
	}

public:

	Writable_XML_Editor(const char* _file_name, const char* _root_label = "content") :
		file_name(_file_name),
		root_label(_root_label),
		current_path({ _root_label })
	{
		create_file_if_doesnt_exist();
	}

	/**
	 *	@brief  Given an (absolute) path (including root element). Checks to see if that path exists in the file. If so, updates the current_path member so future
	 *			public method calls will use current_path + path
	 */
	void set_current_path(std::vector<std::string> path) {
		std::string file_str = read_from_file();

		if (element_exists(path, file_str, true)) {
			current_path = path;
		}
		else {
			throw std::runtime_error("Error: Can't set current path to " + string_vector_to_string(path) + ". Path does not exist.");
		}
	}

	/**
	 *	@brief  Given a path, if that path leads to an existing element, return the value stored between the opening and closing tag for that element
	 */
	std::string read_string(std::vector<std::string> property_path) {
		std::string file_str = read_from_file();

		Element_Pointers element_pointers = find_element(property_path, file_str);

		if (!was_find_successful(element_pointers)) {
			//std::cout << "Error: Could not read property " + string_vector_to_string(property_path) + ". Property does not exist" << std::endl;
			throw std::runtime_error("Error: Could not read property " + full_path_string(property_path) + ". Property does not exist");
		}

		size_t contents_length = element_pointers.contents_end - element_pointers.contents_start;

		return file_str.substr(element_pointers.contents_start, contents_length);
	}

	float read_float(std::vector<std::string> property_path) {
		return stof(read_string(property_path));
	}

	int read_int(std::vector<std::string> property_path) {
		return stoi(read_string(property_path));
	}

	/**
	 *	@brief  Given a path, if that path leads to an existing element, replace the value stored between the opening and closing tag for that element
	 *			with <value>. WARNING: If the path points to an existing section element containing other tags, this method will overwrite everything
	 *			contained in that section with <value>
	 */
	void update_element(std::vector<std::string> element_path, std::string value) {
		std::string file_str = read_from_file();

		Element_Pointers element_pointers = find_element(element_path, file_str);

		if (!was_find_successful(element_pointers)) {
			std::cout << "Error: Could not update element " + full_path_string(element_path) + ". Element does not exist" << std::endl;
			return;
		}

		size_t contents_length = element_pointers.contents_end - element_pointers.contents_start;
		file_str.replace(element_pointers.contents_start, contents_length, value);

		write_to_file(file_str);
	}

	void update_element(std::vector<std::string> element_path, float value) {
		update_element(element_path, std::to_string(value));
	}

	/**
	 *	@brief  Given a path, if <element_path> minus the final element exists, create a new section element matching <element_path>
	 */
	void create_element(std::vector<std::string> element_path) {
		std::string file_str = read_from_file();

		if (element_exists(element_path, file_str)) return; //Already exists, nothing to do
		if (path_has_duplicate_names(element_path)) {
			std::cout << "Cannot create element with path" + full_path_string(element_path) + ". Path has duplicate names" << std::endl;
			return;
		}

		std::vector<std::string> path_minus_back = std::vector<std::string>(element_path.begin(), element_path.end() - 1);
		Element_Pointers element_pointers = find_element(path_minus_back, file_str);

		if (!was_find_successful(element_pointers)) {
			std::cout << "Error: Could not create element " + full_path_string(element_path) + ". Path leading to new element does not exist" << std::endl;
			return;
		}

		std::string new_element_str = make_opening_tag(element_path.back()) + "\n" + make_closing_tag(element_path.back()) + "\n";

		file_str.insert(element_pointers.contents_end, new_element_str);

		write_to_file(file_str);
	}

	/**
	 *	@brief  Runs create_element() followed by update_element() with parameter <value>. WARNING: See update_element() description
	 */
	void create_element(std::vector<std::string> element_path, std::string value) {
		create_element(element_path);
		update_element(element_path, value);
	}

	void create_element(std::vector<std::string> element_path, float value) {
		create_element(element_path, std::to_string(value));
	}

	/**
	 *	@brief  Given a path, checks to see if there is an element matching that path and delete that element and its contents if true
	 */
	void delete_element(std::vector<std::string> element_path) {
		std::string file_str = read_from_file();

		Element_Pointers element_pointers = find_element(element_path, file_str);

		if (!was_find_successful(element_pointers)) return; // Section doesn't exist. Nothing to do

		size_t element_length = element_pointers.end - element_pointers.start;
		file_str.erase(element_pointers.start, element_length + 1); // +1 to remove trailing newline

		write_to_file(file_str);
	}

	/**
	 *	@brief  Given a path, checks to see if there is an element matching that path and delete that element's contents if true
	 */
	void delete_element_contents(std::vector<std::string> element_path) {
		std::string file_str = read_from_file();

		Element_Pointers element_pointers = find_element(element_path, file_str);

		if (!was_find_successful(element_pointers)) return; // Section doesn't exist. Nothing to do

		size_t contents_length = element_pointers.contents_end - element_pointers.contents_start;
		file_str.erase(element_pointers.contents_start, contents_length);
		file_str.insert(element_pointers.contents_start, "\n"); // Re-insert newline between open and close tags

		write_to_file(file_str);
	}

	std::string get_root_label() {
		return root_label;
	}
};