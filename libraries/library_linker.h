/*
 * @file library_linker.h
 * @version 1.1
 * @brief Links cpp files to be used by other projects
 *
 * Copyright 2022 Iris Dynamics Ltd
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


#ifndef LIBARY_LINKER_H_
#define LIBARY_LINKER_H_

#define WINDOWS

#include "modbus_client/transaction.cpp"
#include "irisSDK_libraries/Signal_Generator.cpp"
#ifdef IRISCONTROLS
#include "ic4_library/console_message_parser.cpp"
#include "ic4_library/io_elements.cpp"
#include "ic4_library/ic_app.cpp"
#include "ic4_library/iriscontrols4.cpp"
#include "ic4_library/log.cpp"
#include "ic4_library/serialAPI.cpp"
#include "irisSDK_libraries/Motor_Control_Panel.cpp"
#include "irisSDK_libraries/Motor_Plot_Panel.cpp"
#include "irisSDK_libraries/Signal_Panel.cpp"
#endif


#endif /* LIBARY_LINKER_H_ */
