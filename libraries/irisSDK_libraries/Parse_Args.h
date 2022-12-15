/**
   @file Parse_Args.h
   @class Parse_Args
   @brief Provides functions for parsing command arguments
   @author Michelle Aleman <maleman@irisdynamics.com>
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

#include <stdint.h>


/**
   @brief   Return true if s1 and s2 contain the same characters for the number of characters specified in num_chars, and the char immediately following the 
            last compared char in s1 is either white space or a null terminator. Otherwise returns false.            
*/
bool strings_equal(char * s1, char * s2, uint8_t num_chars){
    
    bool strings_equal = true; // Return value
    uint8_t index = 0; // Char array index
    
    while(index < num_chars){ // Compare each char from char array

        if(s1[index] != s2[index]){
            strings_equal = false; // Return false if any chars are not the same
            break;
        }
        index++;
    }
    
    // Return false if s1 has another character at the end of matching string that is not a white space or new line.
    if((s1[index] != ' ' && s1[index] != '\0')) strings_equal = false;
    
    return strings_equal;
}



/**
   @brief   Function returns true if arg is contained within arg_list.
            For parsing serial commmands, parameters should be: arg_list = args, arg = some string to search for in args         
*/
bool contains_arg(char * arg_list, char * arg){
    // Get number of chars in argument 
    int arg_len = 0;
    while(arg[arg_len]!='\0'){ arg_len++; }
    
    int list_index = 0;
    bool arg_in_list = false;
    while(arg_list[list_index]!='\0'){
        // Skip over arg_list characters that don't match with first character in arg
        if(arg_list[list_index] != arg[0]){
            list_index++;
            continue;
        }
        else{
            // If the first letter matches, check if full arg string is equal
            if(strings_equal(arg_list + list_index, arg, arg_len)){
                arg_in_list = true;
                break;
            }
            else{
                // If strings not equal, move onto next char in arg list
                list_index++;
            }
        }

    }
    return arg_in_list;
}


