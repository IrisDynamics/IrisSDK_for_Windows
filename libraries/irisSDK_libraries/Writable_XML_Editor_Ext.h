#pragma once

#include "irisSDK_libraries/XML_Editor.h"

class Writable_XML_Editor_Ext : public XML_Editor {

	void find_property_ext(std::list<std::string> element_path) {
		init_parse(element_path);

		// Check if file read successfully
		if (contents.length()) {

			// Move content ptr to start of real file content
			content_ptr = contents.find(open_tag + root_label + close_tag) + (open_tag + root_label + close_tag).length();

			if (element_path.size() == 0) {
				content_ptr = contents.find("</" + root_label + ">");
				property_found = true;
				return;
			}

			// Parse string to get property value as an integer
			find_property();
		}
		else {
			std::cout << "Unable to open config file\n";
		}
	}

	void write_to_file() {
		new_config_file.open(file_name, std::ios::trunc);

		if (new_config_file.is_open()) {
			new_config_file << contents;
			new_config_file.close();
		}
	}

	std::string string_list_to_string(std::list<std::string> str_vec, std::string delim = "/") {
		std::string ret_str;

		for (std::string str : str_vec) {
			ret_str += str;
			if (str != str_vec.back()) ret_str += delim;
		}

		return ret_str;
	}

public:

	Writable_XML_Editor_Ext(const char* _file_name, const char* _root_label = "content") :
		XML_Editor(_file_name)
	{}

	/**
	 *	@brief  Given a path, if that path leads to an existing element, return the value stored between the opening and closing tag for that element
	 */
	std::string read_string(std::list<std::string> property_path) {
		find_property_ext(property_path);

		if (!property_found) {
			throw new std::runtime_error("Error: Could not read property " + string_list_to_string(property_path) + ". Property does not exist");
		}

		//size_t post_opening_tag = contents.find(">", content_ptr) + 1; // The index immediately following the opening tag
		size_t closing_tag = contents.find("</" + property_path.back() + ">", content_ptr);
		size_t contents_length = closing_tag - content_ptr;

		return contents.substr(content_ptr, contents_length);
	}

	float read_float(std::list<std::string> property_path) {
		return stof(read_string(property_path));
	}

	int read_int(std::list<std::string> property_path) {
		return stoi(read_string(property_path));
	}

	/**
	 *	@brief  Given a path, if that path leads to an existing element, replace the value stored between the opening and closing tag for that element
	 *			with <value>. WARNING: If the path points to an existing section element containing other tags, this method will overwrite everything
	 *			contained in that section with <value>
	 */
	void update_element(std::list<std::string> element_path, std::string value) {
		find_property_ext(element_path);

		if (!property_found) {
			std::cout << "Error: Cannot update element " + string_list_to_string(element_path) + ". Element does not exist." << std::endl;
			return;
		}

		size_t closing_tag = contents.find("</" + element_path.back() + ">", content_ptr);

		contents.replace(content_ptr, (closing_tag - content_ptr), value);

		write_to_file();
	}

	void update_element(std::list<std::string> element_path, float value) {
		update_element(element_path, String(value));
	}

	/**
	 *	@brief  Given a path, if <element_path> minus the final element exists, create a new section element matching <element_path>
	 */
	void create_element(std::list<std::string> element_path) {

		find_property_ext(element_path);

		if (property_found) return;

		std::list<std::string> path_minus_back(element_path.begin(), std::prev(element_path.end()));
		find_property_ext(path_minus_back);

		if (!property_found) {
			std::cout << "Error: Cannot create new element " + string_list_to_string(element_path) + ". Path " + string_list_to_string(path_minus_back) + " does not exist." << std::endl;
			return;
		}

		size_t closing_tag;
		if (path_minus_back.size() == 0) {
			closing_tag = contents.find("</" + root_label + ">");
		}
		else {
			closing_tag = contents.find("</" + path_minus_back.back() + ">", content_ptr);
		}

		std::string new_element_str = "<" + element_path.back() + ">" + "\n" + "</" + element_path.back() + ">" + "\n";
		contents.insert(closing_tag, new_element_str);

		write_to_file();
	}

	/**
	 *	@brief  Runs create_element() followed by update_element() with parameter <value>. WARNING: See update_element() description
	 */
	void create_element(std::list<std::string> element_path, std::string value) {
		create_element(element_path);
		update_element(element_path, value);
	}

	void create_element(std::list<std::string> element_path, float value) {
		create_element(element_path, String(value));
	}

	/**
	 *	@brief  Given a path, checks to see if there is an element matching that path and delete that element and its contents if true
	 */
	void delete_element(std::list<std::string> element_path) {

		find_property_ext(element_path);

		if (!property_found) return; //Section doesn't exist. Nothing to do

		content_ptr -= ("<" + element_path.back() + ">").length();
		std::string closing_tag = "</" + element_path.back() + ">";
		size_t post_closing_tag = contents.find(closing_tag, content_ptr) + closing_tag.length();

		contents.erase(content_ptr, post_closing_tag - content_ptr + 1); //+1 to delete newline

		write_to_file();
	}

	/**
	 *	@brief  Given a path, checks to see if there is an element matching that path and delete that element's contents if true
	 */
	void delete_element_contents(std::list<std::string> element_path) {

		find_property_ext(element_path);

		if (!property_found) return; //Section doesn't exist. Nothing to do

		size_t closing_tag = contents.find("</" + element_path.back() + ">", content_ptr);

		contents.erase(content_ptr, closing_tag - content_ptr); //+1 to delete newline
		contents.insert(content_ptr, "\n"); //Re-insert newline

		write_to_file();
	}
};