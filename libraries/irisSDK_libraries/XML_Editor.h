/**
    @file XML_Editor.h
    @author Michelle Aleman <maleman@irisdynamics.com>
    @brief  XML editing object that reads and writes specified property values
    @version 1.1

    @copyright Copyright 2022 Iris Dynamics Ltd
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    For questions or feedback on this file, please email <support@irisdynamics.com>.
*/


#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <sstream>
#include <vector>
#include <iomanip>
#include  "irisSDK_libraries/device_config.h"

 //temporary printing
#include "ic4_library/iriscontrols4.h"


/* @brief Class that reads and writes property values to the specified XML file */
class XML_Editor {
protected:

    // Path to XML file
    const char* file_name;

    // File stream
    std::ifstream config_file;
    std::ofstream new_config_file;

    // Open and close tag chars in xml
    std::string open_tag = "<";
    std::string close_tag = ">";

    // Parsing indexes
    int open_tag_index;
    int close_tag_index;

    // Open and close comment strings in XML
    std::string open_comment_str = "<!--";
    std::string close_comment_str = "-->";

    // XML configuration file root tag
    std::string root_label = "content";

    // String contains all file contents
    std::string contents = "";

    // Value keeps track of how much of the contents string has been parsed 
    int content_ptr;

    // list and string values for property names to search for in XML file
    std::list<std::string> property_names;
    std::string property_name;



    /* @brief Function sets the property_names list and property_name string properties, and reads the xml file to the contents string */
    void init_parse(std::list<std::string> _property_names) {

        // copy list and first name
        property_names = _property_names;
        property_name = "";
        if (property_names.size() > 0) property_name = _property_names.front();

        // Copy entire file to the string 'contents' for parsing
        get_file_contents();
    }



    /* @brief Function opens the specified XML file and returns its contents as a std string. */
    std::string get_file_contents() {
        std::stringstream buffer;
        config_file.open(file_name, std::ios::in);

        if (config_file.is_open()) {
            buffer << config_file.rdbuf();
            config_file.close();
        }

        contents = buffer.str();

        return contents;
    }



    float get_property_value() {

        // Read value until closing tag for this label and return
        open_tag_index = contents.find(open_tag, content_ptr);                                              // Get index of next opening tag <
        std::string value = contents.substr(content_ptr, (open_tag_index - content_ptr));                   // Copy contents until next open tag < to get property value

        // Return value as an integer (extra white space around value as string does not affect value returned)
        float return_value = std::stof(value.c_str());

        /*
        //temp
        std::cout << "\nfound value ";
        std::cout << property_name.c_str();
        std::cout << ": |";
        std::cout << (String(return_value)).c_str();
        std::cout << "|\n";
        std::cin.get();
        */


        return return_value;
    }




    /* @brief Function updates the value of the sepcified property in the XML file */
    void set_property_value(float value) {

        // Convert value to string
        std::string value_str = String(value);

        // Read value until closing tag for this label and return
        open_tag_index = contents.find(open_tag, content_ptr);                                              // Get index of next opening tag <

        //temp print string to replace
        //std::cout << "found value ";
        //std::cout << (contents.substr(content_ptr, (open_tag_index - content_ptr))).c_str();
        //std::cout << ".\n\n";

        // Replace
        contents.replace(content_ptr, (open_tag_index - content_ptr), value_str);

        // Overwrite existing config file
        new_config_file.open(file_name, std::ios::trunc);

        new_config_file.write(contents.c_str(), contents.length());

        // Close file
        new_config_file.close();
    }




    /* @brief Function parses the given 'contents' string and searches for the given property name enclosed in XML tags: <>.
    If found, the class bool 'property_found' is set to true. Otherwise, 0 is returned and 'property_found' is false. */
    float find_property() {

        // Initialize search status
        property_found = false;


        while ((content_ptr < contents.size()) && (property_names.size() > 0)) {

            // Read next label
            open_tag_index = contents.find(open_tag, content_ptr);                                                  // Get index of next opening tag <
            content_ptr = open_tag_index + open_tag.length();                                                       // Update content ptr

            //temp
            if (open_tag_index < 0) break;

            // Check if opening tag is for a comment
            if (contents.substr(open_tag_index, open_comment_str.length()) == (std::string)open_comment_str) {
                // Skip comment
                close_tag_index = contents.find(close_comment_str, content_ptr) + close_comment_str.length();
                content_ptr = close_tag_index;

                // Restart looking for property label after skipping comment
                continue;
            }

            // Not a comment, get property label
            close_tag_index = contents.find(close_tag, content_ptr);                                                // Get index of next closing tag >
            content_ptr = close_tag_index + close_tag.length();
            std::string label = contents.substr((open_tag_index + 1), (close_tag_index - open_tag_index - 1));      // Read until the closing tag to get label name

            if ((label == property_name) && (property_names.size() == 1)) {
                // Update search status
                property_found = true;
                // Break out of while loop
                break;
            }
            else if (label == property_name) {
                // Continue looking within this property
                property_names.pop_front();
                property_name = property_names.front();
                continue;
            }
            else {
                // Skip this property
                open_tag_index = contents.find((open_tag + "/" + label + close_tag), content_ptr);                  // get index of opening tag
                content_ptr = open_tag_index + (open_tag + "/" + label + close_tag).length();

            }

        } // while content size > 0
        return 0;

    }



    float read_property() {

        // Check if file read successfully
        if (contents.length()) {

            // Move content ptr to start of real file content
            content_ptr = contents.find(open_tag + root_label + close_tag) + (open_tag + root_label + close_tag).length();

            // Parse string to get property value as an integer
            find_property();

            if (property_found) {
                // Return value of this property

                return get_property_value();
            }
            else {
                std::cout << "Unable to find property: ";
                std::cout << property_name;
                std::cout << "\n";
            }

        }
        else {
            std::cout << "Unable to open config file\n";
        }

    }


public:

    // Success status of read or write
    bool property_found = false;



    /* @brief Constructor initializes path to XML file name */
    XML_Editor(const char* _file_name) :
        // Initialization List
        file_name(_file_name)
    {}


    /* @brief Returns the value of the given property name as a float, else returns 0. Check the public bool value 'property_found' to determine if search was successful */
    float get_float(std::string _property_name) {
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse({ _property_name });
        // Read property value as a float
        float value = read_property();
        return value;
    }


    /* @brief Returns the value of the given property name as an integer, else returns 0. Check the public bool value 'property_found' to determine if search was successful */
    int get_int(std::string _property_name) { 
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse({ _property_name });
        // Read property value are an integer
        int value = (int)read_property();
        return value;
    }


    /* @brief Returns the value of the given property name as an string, else returns "0". Check the public bool value 'property_found' to determine if search was successful */
    std::string get_str(std::string _property_name) {
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse({ _property_name });
        // Read property value as a string
        std::string value = String(read_property());
        return value;
    }


    /* @brief Get function for float values that are nested within more than 1 property tag name. Input: List of strings of the path of property names to find.
    Returns the value of the given property name as an float, else returns 0. Check the public bool value 'property_found' to determine if search was successful */
    float get_float(std::list<std::string> _property_names) {
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse(_property_names);
        // Read property value as a float
        float value = read_property();
        return value;
    }


    /* @brief Get function for integer values that are nested within more than 1 property tag name. Input: List of strings of the path of property names to find.
    Returns the value of the given property name as an int, else returns 0. Check the public bool value 'property_found' to determine if search was successful */
    float get_int(std::list<std::string> _property_names) {
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse(_property_names);
        // Read property value as int
        int value = (int)read_property();

        return value;
    }


    /* @brief Get function for string values that are nested within more than 1 property tag name. Input: List of strings of the path of property names to find.
    Returns the value of the given property name as an string, else returns 0. Check the public bool value 'property_found' to determine if search was successful */
    std::string get_str(std::list<std::string> _property_names) {
        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse(_property_names);
        // Read property value as string
        std::string value = String(read_property());

        return value;
    }


    /* @brief Function updates the value of the specified property in the XML file
       @param[in] _property_names The path of XML tags to the property to change
                  value The new value to give the specified property
    */
    void set_property(std::list<std::string> _property_names, float value) {

        // copy list and first name and copy entire file to the string 'contents' for parsing
        init_parse(_property_names);

        // Check if file read successfully
        if (contents.length()) {

            // Move content ptr to start of real file content
            content_ptr = contents.find(open_tag + root_label + close_tag) + (open_tag + root_label + close_tag).length();

            // Parse string to get property value as an integer
            find_property();

            if (property_found) {
                set_property_value(value);
            }
            else {
                std::cout << "Unable to find property ";
                std::cout << property_name;
                std::cout << "\n";
            }
        }
        else {
            std::cout << "Unable to open config file\n";
        }

    }

    /* @brief Function updates the value of the specified property in the XML file */
    void set_property(std::string _property_name, float value) {
        std::list<std::string> property_as_list = { _property_name };
        set_property(property_as_list, value);
    }

};