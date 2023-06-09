# IrisControls4 Library Changelog

Contains more detailed code structure changes than the general release notes. 

Should be updated only by the project manager and only after a successful pull-request.

## v3.0
- Implemented recover function in circular buffer that removes chars until it finds the beginning of a new message.
- Implemented bytes_to_trailer function in circular buffer that finds how many bytes remain in the message before the trailer.
- Changed numerous erroneous uses of int to u32.
- Created new functions pop_int, pop_double, and pop_bool that are called when it is guaranteed the buffer contains a full int/double/bool. The parse_int and parse_double functions are still used for parsing console messages.
- Rewrote check_for_full_message to parse byte stuffed messages. It ensures the buffer contains a flag and a full trailer before incrementing pending_incoming_messages. 
- Removed unused get_char_ptr functions from Windows driver.
- Removed print_d implementation from Windows driver as this was supposed to have been done before. The implementation is in the base class.
- Implemented a 32 byte char array for use in converting vars to strings for console messages.
- Use aforementioned array in the Windows driver because the previous use of std::string was going out of scope and causing the values to not work.
- Amended the while loop in check to only call check_for_full_message if the RX buffer isn't empty. This prevents an edge case error where if start_index + 1 happens to contain a valid console command it could attempt to parse whatever junk comes after.
- parse_API now checks for FLAG and if not present it passes the message down the parsing chain. If there is a flag but the command ID is CONSOLE_MESSAGE, it passes the full message on including the FLAG.
- get_message can now handle IC4 messages and general serial messages (with or without the FLAG) but if IC4 messages must use a valid command ID (CONSOLE_MESSAGE).
- Restored support for ASCII formatted console message arguments by re-writing parse_int and parse_double. 
- Restored support for serial messages terminated by \r.
- Restored the argument char array at 100 bytes.
- Virtualised parse_double because the kv31 does not support sscanf of doubles. The kv31 implementation prints an error message to the console.
- Replaced a few for loops with while loops to remove compiler warnings.
- FlexSlider::set_range now multiplies min and max by my_factor before sending to the application. This is to mirror the behaviour in FlexSlider::add.

## v2.9
- Added serial API for adding/removing child GUI_Pages from GUI_pages.
- Overloaded GUI_Page::add to have a parent GUI_Page.
- Added class definitions and serial API for FlexDropdowns.



