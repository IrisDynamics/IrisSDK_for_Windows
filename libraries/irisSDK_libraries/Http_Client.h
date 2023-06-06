/**
    @file Http_Client.h
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

#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#include <iostream>
//#include <fstream>      //may not need this
#include <string>
#include <vector>
#include <iostream>
#include <fstream> 
#include <locale>
#include <codecvt>
#include <map>          


#pragma comment(lib, "winhttp.lib")

using namespace std;


//todos: 
//* might be better to do a get request with a body instread of using a query string? Worth trying out anyways 
//* I moved all the request processing stuff into a separate method that each request calls - seems to work ok for gets, but not puts or posts - (post includes additional header fields that get doesn't include. Also handles different http return message (201 vs 200))
//* we need a way to send text files to tulip, but that might not live in this class. Probably the next thing to figure out though.
//* could probabyl make all the string variables in the request_config struct std::strings and then cast them in the actual calls - seems a bit cleaner? or does it... - (I like the idea)
//* put request is currently reading from a file and adding 
//* these operations sometimes (not ofter, but sometimes) time out iris controls - should probably be handled in a separate thread or soemthing - (Receive might be blocking, leading to timeout)

//in the sdk libraries
class Http_Client
{

public:

    LPSTR pszOutBuffer;
    DWORD dwSize = 100;
    DWORD dwDownloaded = 0;

    //handles
    HINTERNET  hSession = NULL,
               hConnect = NULL,
               hRequest = NULL;

    //config stuff
    BOOL  bResults = FALSE;
    DWORD dwProxyAuthScheme = 0;

    //auth header 
    LPCWSTR basic_header;
    DWORD basic_header_length;

    //content type header 
    LPCWSTR  content_header;
    DWORD content_header_length;

    //string converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> str_wstr_converter;

    //filter and param lists - used for gets 
    std::vector<std::tuple<string, string>> filters;
    std::vector<std::tuple<string, string>> query_params;

    struct request_config {
        LPCWSTR Server;
        LPCWSTR Path;
        BOOL UseSSL = true;
        LPCWSTR ServerUsername;
        LPCWSTR ServerPassword;
        LPCWSTR ProxyUsername;
        LPCWSTR ProxyPassword;
        string Body = "{}";
    };

    request_config request;


    Http_Client() {

    }

    DWORD ChooseAuthScheme(DWORD dwSupportedSchemes)
    {
        //  It is the server's responsibility only to accept 
        //  authentication schemes that provide a sufficient
        //  level of security to protect the servers resources.
        //
        //  The client is also obligated only to use an authentication
        //  scheme that adequately protects its username and password.
        //
        //  Thus, this sample code does not use Basic authentication  
        //  becaus Basic authentication exposes the client's username
        //  and password to anyone monitoring the connection.

        if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
            return WINHTTP_AUTH_SCHEME_NEGOTIATE;
        else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
            return WINHTTP_AUTH_SCHEME_NTLM;
        else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
            return WINHTTP_AUTH_SCHEME_PASSPORT;
        else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
            return WINHTTP_AUTH_SCHEME_DIGEST;
        else
            return WINHTTP_AUTH_SCHEME_BASIC;
    }


    /**
    * @fn add_request_body_pair()
    * @brief Adds a new pair to the request Body string. The value of this pair is a string.
    */
    void add_request_body_pair(string key, string value) {
        //remove back curly brace 
        request.Body.pop_back();

        //add new key/value pair with escape character 
        request.Body.append("\"" + key + "\": \"" + value + "\", }");

        //request.Body = "{\"id\": \"This is a test\", \"qfeux_test_int\" : 12, \"vnjoe_test_string\" : \"helphelphelp\"}";

        //for testing/debugging
       // printf(request.Body.c_str());
    }

    /**
    * @fn add_request_body_pair()
    * @brief Adds a new pair to the request Body string. The value of this pair is an int.
    */
    void add_request_body_pair(string key, int value) {
        //convert int parameter to string
        string int_string = to_string(value);
        
        //remove back curly brace 
        request.Body.pop_back();

        //add new key/value pair with escape character 
        request.Body.append("\"" + key + "\": " + int_string + " , }");

        //for testing/debugging
       // printf(request.Body.c_str());
    }

    /**
    * @fn add_filter_pair(string _filter_key, string _filter_val)
    * @brief adds a new filter to the container - thses will be appended to the query string during a get request 
    * @param _filter_key - the key of the key value pair, _filter_val - the value of the key value pair
    */
    void add_filter_pair(string _filter_key, string _filter_val) {
        tuple<string, string> temp_pair = { _filter_key, _filter_val };
        filters.push_back(temp_pair);
    }

    /**
     * @fn add_param_pair(string _param_key, string _param_val)
     * @brief adds a new param to the container - thses will be appended to the query string during a get request
     * @param _filter_key - the key of the key value pair, _filter_val - the value of the key value pair
     */
    void add_param_pair(string _param_key, string _param_val) {
        tuple<string, string> temp_pair = { _param_key, _param_val };
        query_params.push_back(temp_pair);
    }

    /**
     * @fn reset_query_params()
     * @brief clears the query_params vector so it can be reused for another request 
     */
    void reset_query_params() {
        query_params.clear();
    }

    /**
     * @fn reset_filter_params()
     * @brief clears the filters vector so it can be reused for another request
     */
    void reset_filter_params() {
        filters.clear();
    }

    /**
    * @fn create_query_string()
    * @brief Creates a quesry string to be used in get requests 
    */
    wstring  create_query_string() {

        printf("lets make a string");       

        wstring temp_path = request.Path;
        temp_path.append(L"?");

        //add any params to the get string 
        for (int i = 0; i < query_params.size(); i++) {
            temp_path.append(L"&");
            //convert the first part of the tuple to wstring and append to temp string 
            temp_path.append(str_wstr_converter.from_bytes(get<0>(query_params[i])));
            temp_path.append(L"=");
            temp_path.append(str_wstr_converter.from_bytes(get<1>(query_params[i])));
        }

        //check if there are filters, and if so add them too 
        if (filters.size() > 0) {
            temp_path.append(L"&filters=[{");
            for (int i = 0; i < filters.size(); i++) {
                temp_path.append(L"\"" + str_wstr_converter.from_bytes(get<0>(filters[i])) + L"\":\"" + str_wstr_converter.from_bytes(get<1>(filters[i])) + L"\",");
            }

            //remove end ,
            temp_path.pop_back();

            //add filter aggregator - this seems to be needed for tulip, not sure if it is for all servers though
            temp_path.append(L"}]&filterAggregator=any");
            return temp_path;
        }

    }

    /**
    * @fn clear_request_data_body()
    * @brief Clears the request body of the request_data struct 
    * @note All other members of the request_data struct are not effected by this method
    */
    void clear_request_data_body() {
        request.Body = "{}";
    }

    /**
    * @fn reset_request_data()
    * @brief Resets all members of the request_data struct to defaults
    * @note Members must be specificied before this struct can be resused
    */
    void reset_request_data() {
        request.Server = L"";
        request.Path = L"";
        request.UseSSL = true;
        request.ServerUsername = L"";
        request.ServerPassword = L"";
        request.ProxyUsername = L"";
        request.ProxyPassword = L"";
        request.Body = "{}";
    }

    /**
     * @fn http_client_setup()
     * @brief sets up http session and connection
     */
    void http_client_setup() {
        //lets begin 
       // Use WinHttpOpen to obtain a session handle.
        hSession = WinHttpOpen(L"IrisLibrary",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);

        if (!hSession) {
            throw_winhttp_error("Failed to setup http session during client setup");
        }

        // Tulip uses basic authentication. Basic authentication is handled by
        //  including user id and password credentials unencrypted in a header
        //  field. Because of this, we should never use raw HTTP with Tulip. 
        //  Doing so would mean we are sending our Tulip credentials unencrypted
        //  across the internet, and anyone listening would have full access to our
        //  account and any features that it allows.
        INTERNET_PORT nPort = //INTERNET_DEFAULT_HTTPS_PORT;
            (request.UseSSL) ?
            INTERNET_DEFAULT_HTTPS_PORT :
            INTERNET_DEFAULT_HTTP_PORT;

        // Specify an HTTP server
        hConnect = WinHttpConnect(hSession,
            request.Server,
            nPort,
            0);
        
        if (!hConnect) {
            throw_winhttp_error("Failed to create http connection during client setup.");
        }
    }

    /**
     * @fn send_post_request()
     * @brief sends a post request to the server specified in http_client_setup
     */
    bool send_post_request() {

        BOOL bResults;

        //make sure the body is properly formatted
        remove_hanging_comma();

        OutputDebugString((LPCWSTR)request.Path);

        hRequest = WinHttpOpenRequest(hConnect,
            L"POST",
            (LPCWSTR)request.Path,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            (request.UseSSL) ?
            WINHTTP_FLAG_SECURE : 0);


        //adding auth header 
        if (hRequest) {
            bResults = WinHttpAddRequestHeaders(hRequest, basic_header, basic_header_length, WINHTTP_ADDREQ_FLAG_ADD);
        }

        //add content type header 
        if (hRequest) {
            bResults = WinHttpAddRequestHeaders(hRequest, content_header, content_header_length, WINHTTP_ADDREQ_FLAG_ADD);
        }

        DWORD dwStatusCode = 0;
        DWORD dwSupportedSchemes;
        DWORD dwFirstScheme;
        DWORD dwSelectedScheme;
        DWORD dwTarget;
        DWORD dwLastStatus = 0;
        DWORD dwSize = sizeof(DWORD);

        BOOL  bDone = FALSE;

        bResults = FALSE;

        // Send a request until status code is not 401 or 407.
        while (!bDone)
        {

            //  If a proxy authentication challenge was responded to, reset those credentials before each SendRequest, because the proxy  
            //  may require re-authentication after responding to a 401 or to a redirect. If you don't, you can get into a 
            //  407-401-407-401- loop.
            if (dwProxyAuthScheme != 0)
                bResults = WinHttpSetCredentials(hRequest,
                    WINHTTP_AUTH_TARGET_PROXY,
                    dwProxyAuthScheme,
                    request.ProxyUsername,
                    request.ProxyPassword,
                    NULL);

            // Send a request.
            bResults = WinHttpSendRequest(hRequest,
                WINHTTP_NO_ADDITIONAL_HEADERS,
                0,
                LPVOID(request.Body.c_str()),
                strlen(request.Body.c_str()),
                strlen(request.Body.c_str()),
                0);

            if (!bResults) {
                printf("Issue with sending request");
            }

            // End the request.
            if (bResults) {
                bResults = WinHttpReceiveResponse(hRequest, NULL);
            }
            else {
                printf("\nError receiving response.\n");
            }


            // Resend the request in case of 
            // ERROR_WINHTTP_RESEND_REQUEST error.
            if (!bResults && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST)
                continue;

            // Check the status code.
            if (bResults) {
                bResults = WinHttpQueryHeaders(hRequest,
                    WINHTTP_QUERY_STATUS_CODE |
                    WINHTTP_QUERY_FLAG_NUMBER,
                    NULL,
                    &dwStatusCode,
                    &dwSize,
                    NULL);

            }

            if (bResults)
            {

                switch (dwStatusCode)
                {
                // 

                case 201:
                    // The record was successfully created.
                    // You can use WinHttpReadData to read the contents of the server's response.
                    printf("\nThe record was successfully created.\n");
                    bDone = TRUE;
                    break;

                case 401:
                    // The server requires authentication.
                    printf(" The server requires authentication. Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                    {
                        dwSelectedScheme = ChooseAuthScheme(dwSupportedSchemes);

                        if (dwSelectedScheme == 0)
                            bDone = TRUE;
                        else
                            bResults = WinHttpSetCredentials(hRequest,
                                dwTarget,
                                dwSelectedScheme,
                                request.ServerUsername,
                                request.ServerPassword,
                                NULL);
                    }

                    // If the same credentials are requested twice, abort the request.  
                    if (dwLastStatus == 401)
                        bDone = TRUE;

                    break;

                case 407:
                    // The proxy requires authentication.
                    printf("The proxy requires authentication.  Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                        dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

                    // If the same credentials are requested twice, abort the request.  
                    if (dwLastStatus == 407)
                        bDone = TRUE;
                    break;

                default:
                    // The status code does not indicate success.
                    printf("Error. Status code %d returned.\n", dwStatusCode);
                    bDone = TRUE;
                }
            }

            // Keep track of the last status code.
            dwLastStatus = dwStatusCode;

            // If there are any errors, break out of the loop.
            if (!bResults)
                bDone = TRUE;
        }

        // Report any errors.
        if (!bResults)
        {
            DWORD dwLastError = GetLastError();
            printf("Error %d has occurred.\n", dwLastError);
        }

        printf("End of request\n");

        //finish_request(bResults);

        return true;
    }

    /**
     * @fn send_get_request()
     * @brief sends a get request to the server specified in http_client_setup
     */
    bool send_get_request() {
        //get the query string
        wstring temp_path = create_query_string();

        std::cout << std::endl;

        std::wcout << temp_path << std::endl;

        hRequest = WinHttpOpenRequest(hConnect,
            L"GET",
            (LPCWSTR)temp_path.c_str(),
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            (request.UseSSL) ?
            WINHTTP_FLAG_SECURE : 0);


        if (!hRequest) {
            throw_winhttp_error("Failure following WinHttpOpenRequest() from send_get_request()");
        }

        finish_request();

        return true;

    }

    /**
     * @fn finish_request(BOOL bResults)
     * @brief finishes up the request and checks the response
     */
    void finish_request() {
        DWORD dwStatusCode = 0;
        DWORD dwSupportedSchemes;
        DWORD dwFirstScheme;
        DWORD dwSelectedScheme;
        DWORD dwTarget;
        DWORD dwLastStatus = 0;
        DWORD dwSize = sizeof(DWORD);

        BOOL  bDone = FALSE;

        BOOL bResults = FALSE;

        while (!bDone)
        {
            //  If a proxy authentication challenge was responded to, reset
            //  those credentials before each SendRequest, because the proxy  
            //  may require re-authentication after responding to a 401 or  
            //  to a redirect. If you don't, you can get into a 
            //  407-401-407-401- loop.
            if (dwProxyAuthScheme != 0)
                WinHttpSetCredentials(hRequest,
                    WINHTTP_AUTH_TARGET_PROXY,
                    dwProxyAuthScheme,
                    request.ProxyUsername,
                    request.ProxyPassword,
                    NULL);

            // Send a request.
            bResults = WinHttpSendRequest(hRequest,
                WINHTTP_NO_ADDITIONAL_HEADERS,
                0,
                WINHTTP_NO_REQUEST_DATA,
                0,
                0,
                0);

            // End the request.
            if (bResults) 
                bResults = WinHttpReceiveResponse(hRequest, NULL);

            // Resend the request in case of 
            // ERROR_WINHTTP_RESEND_REQUEST error.
            if (!bResults && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST)
                continue;

            // Check the status code.
            bResults = WinHttpQueryHeaders(hRequest,
                WINHTTP_QUERY_STATUS_CODE |
                WINHTTP_QUERY_FLAG_NUMBER,
                NULL,
                &dwStatusCode,
                &dwSize,
                NULL);

            if (bResults)
            {

                switch (dwStatusCode)
                {
                case 200:
                    // The resource was successfully retrieved.
                    // You can use WinHttpReadData to read the contents of the server's response.
                    printf("The resource was successfully retrieved.\n");

                    //keep reading data until there is nothing left
                    do {
                        // Check for available data.
                        dwSize = 0;
                        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                        {
                            printf("Error %u in WinHttpQueryDataAvailable.\n",
                                GetLastError());
                            break;
                        }

                        // No more available data.
                        if (!dwSize)
                            break;

                        // Allocate space for the buffer.
                        pszOutBuffer = new char[dwSize + 1];

                        if (!pszOutBuffer)
                        {
                            printf("Out of memory\n");
                            break;
                        }

                        //read the results 
                        ZeroMemory(pszOutBuffer, dwSize + 1);

                        if (!WinHttpReadData(hRequest, LPVOID(pszOutBuffer), dwSize, &dwDownloaded)) {
                            printf("Error %u in winhttpreaddata.\n",
                                GetLastError());
                        }
                        else {
                            printf("%s", pszOutBuffer);
                        }

                        bDone = TRUE;
                    } while (dwSize > 0);

                    break;

                case 201:
                    // The record was successfully created.
                    // You can use WinHttpReadData to read the contents of the server's response.
                    printf("\nThe record was successfully created.\n");
                    bDone = TRUE;
                    break;

                case 401:
                    // The server requires authentication.
                    printf(" The server requires authentication. Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                    {
                        dwSelectedScheme = ChooseAuthScheme(dwSupportedSchemes);

                        if (dwSelectedScheme == 0)
                            bDone = TRUE;
                        else
                            bResults = WinHttpSetCredentials(hRequest,
                                dwTarget,
                                dwSelectedScheme,
                                request.ServerUsername,
                                request.ServerPassword,
                                NULL);
                    }

                    // If the same credentials are requested twice, abort the request.  
                    if (dwLastStatus == 401)
                        bDone = TRUE;

                    break;

                case 407:
                    // The proxy requires authentication.
                    printf("The proxy requires authentication.  Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                        dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

                    // If the same credentials are requested twice, abort the request.  
                    if (dwLastStatus == 407)
                        bDone = TRUE;
                    break;

                default:
                    // The status code does not indicate success.
                    printf("Error. Http Status code %d returned.\n", dwStatusCode);
                    bDone = TRUE;
                }
            }

            // Keep track of the last status code.
            dwLastStatus = dwStatusCode;

            // If there are any errors, break out of the loop.
            if (!bResults)
                bDone = TRUE;
        }

        // Report any errors.
        if (!bResults)
        {
            DWORD dwLastError = GetLastError();
            printf("Error %d has occurred.\n", dwLastError);
        }
    }

    /**
     * @fn getFileContent(const std::string& path)
     * @brief reads in the contents of a file
     */
    string getFileContent(const std::string& path)
    {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        return content;
    }

    /**
    * @fn send_put_request()
    * @brief Sends a put request to a given end point, indicated by the ID parameter 
    * @param ID - the string that is the id of the record we are putting to, Filename - the file we want to put
    */
    bool send_put_request (string _ID) {
        DWORD dwStatusCode = 0;
        DWORD dwSupportedSchemes;
        DWORD dwFirstScheme;
        DWORD dwSelectedScheme;
        DWORD dwTarget;
        DWORD dwLastStatus = 0;
        DWORD dwSize = sizeof(DWORD);

        BOOL  bDone = FALSE;

        //this string manilupation could maybe be moved to it's own method - had some issues passing the string back and forth
        //copy the original path and add a slash at the end
        wstring temp_string = request.Path;
        temp_string.append(L"/");

        //conver the _ID string to a wstring and append to the temporary path 

        wstring wide_id = str_wstr_converter.from_bytes(_ID);
        temp_string.append(wide_id);

        //convert w string to LPCWSTR
        LPCWSTR temp_path = temp_string.c_str();
                
        hRequest = WinHttpOpenRequest(hConnect,
            L"PUT",
            temp_path,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            (request.UseSSL) ?
            WINHTTP_FLAG_SECURE : 0);

        //adding auth header 
        if (hRequest) {
            bResults = WinHttpAddRequestHeaders(hRequest, basic_header, basic_header_length, WINHTTP_ADDREQ_FLAG_ADD);
        }

        //reset content type header for posting files 
        content_header = L"Content-Type: application/json\0";
        content_header_length = -1;

        //add content type header 
        if (hRequest) {
            bResults = WinHttpAddRequestHeaders(hRequest, content_header, content_header_length, WINHTTP_ADDREQ_FLAG_ADD);
        }

        remove_hanging_comma();

        // Send a request until status code is not 401 or 407.
        while (!bDone)
        {

            //  If a proxy authentication challenge was responded to, reset those credentials before each SendRequest, because the proxy  
            //  may require re-authentication after responding to a 401 or to a redirect. If you don't, you can get into a 
            //  407-401-407-401- loop.
            if (dwProxyAuthScheme != 0)
                bResults = WinHttpSetCredentials(hRequest,
                    WINHTTP_AUTH_TARGET_PROXY,
                    dwProxyAuthScheme,
                    request.ProxyUsername,
                    request.ProxyPassword,
                    NULL);


            // Send a request.
            bResults = WinHttpSendRequest(hRequest,
                WINHTTP_NO_ADDITIONAL_HEADERS,
                0,
                LPVOID(request.Body.c_str()),
                strlen(request.Body.c_str()),
                strlen(request.Body.c_str()),
                0);

            if (!bResults) {
                printf("Issue with sending request");
            }

            // End the request.
            if (bResults) {
                bResults = WinHttpReceiveResponse(hRequest, NULL);
            }
            else {
                printf("\nError receiving response.\n");
            }


            // Resend the request in case of 
            // ERROR_WINHTTP_RESEND_REQUEST error.
            if (!bResults && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST)
                continue;

            // Check the status code.
            if (bResults) {
                bResults = WinHttpQueryHeaders(hRequest,
                    WINHTTP_QUERY_STATUS_CODE |
                    WINHTTP_QUERY_FLAG_NUMBER,
                    NULL,
                    &dwStatusCode,
                    &dwSize,
                    NULL);

            }

            if (bResults)
            {

                switch (dwStatusCode)
                {
                case 200:
                    // The resource was successfully retrieved.
                    // You can use WinHttpReadData to read the contents of the server's response.
                    printf("\nThe resource was successfully retrieved.\n");

                    // Allocate space for the buffer.
                    pszOutBuffer = new char[dwSize + 1];
                    if (!pszOutBuffer)
                    {
                        printf("\nOut of memory\n");
                        break;
                    }

                    //read the results 
                    ZeroMemory(pszOutBuffer, dwSize + 1);

                    if (!WinHttpReadData(hRequest, LPVOID(pszOutBuffer), dwSize, &dwDownloaded)) {
                        printf("\nError %u in winhttpreaddata.\n",
                            GetLastError());
                    }
                    else {
                        printf("\nRead data:\n");
                        printf("%s", pszOutBuffer);
                    }

                    bDone = TRUE;
                    break;

                case 201:
                    // The record was successfully created.
                    // You can use WinHttpReadData to read the contents of the server's response.
                    printf("\nThe record was successfully created.\n");
                    bDone = TRUE;
                    break;

                case 401:
                    // The server requires authentication.
                    printf(" The server requires authentication. Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                    {
                        dwSelectedScheme = ChooseAuthScheme(dwSupportedSchemes);

                        if (dwSelectedScheme == 0)
                            bDone = TRUE;
                        else
                            bResults = WinHttpSetCredentials(hRequest,
                                dwTarget,
                                dwSelectedScheme,
                                request.ServerUsername,
                                request.ServerPassword,
                                NULL);
                    }

                    // If the same credentials are requested twice, abort the request.  For simplicity, this sample does not check
                    // for a repeated sequence of status codes.
                    if (dwLastStatus == 401)
                        bDone = TRUE;

                    break;

                case 407:
                    // The proxy requires authentication.
                    printf("The proxy requires authentication.  Sending credentials...\n");

                    // Obtain the supported and preferred schemes.
                    bResults = WinHttpQueryAuthSchemes(hRequest,
                        &dwSupportedSchemes,
                        &dwFirstScheme,
                        &dwTarget);

                    // Set the credentials before resending the request.
                    if (bResults)
                        dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

                    // If the same credentials are requested twice, abort the request.  For simplicity, this sample does not check 
                    // for a repeated sequence of status codes.
                    if (dwLastStatus == 407)
                        bDone = TRUE;
                    break;

                default:
                    // The status code does not indicate success.
                    printf("Error. Status code %d returned.\n", dwStatusCode);
                    bDone = TRUE;
                }
            }

            // Keep track of the last status code.
            dwLastStatus = dwStatusCode;

            // If there are any errors, break out of the loop.
            if (!bResults)
                bDone = TRUE;
        }

        // Report any errors.
        if (!bResults)
        {
            DWORD dwLastError = GetLastError();
            printf("Error %d has occurred.\n", dwLastError);
        }

        printf("End of request\n");
        return true;
    }

    private: 

     /**
     * @fn remove_hanging_comma()
     * @brief removes the hanging comma from the request body
     */
    void remove_hanging_comma() {
        //find where in the string the last comma is
        size_t found_pos = request.Body.find_last_of(",");

        //remove this last comma 
        request.Body.erase(found_pos, 1);

    }

    void throw_winhttp_error(std::string error_msg) {
        throw std::runtime_error(error_msg + ". Error code: " + std::to_string(GetLastError()));
    }
};

