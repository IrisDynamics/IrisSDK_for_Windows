#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <iostream>
#include "SimpleHttpExceptions.h"
#include "StringJsonFormatter.h"
#include "QueryStringFormatter.h"
#include <functional>
#include <mutex>
#include "IrisHttpResponseType.h"


#pragma comment(lib, "winhttp.lib")

class SimpleHttpClient;

void __stdcall winhttp_callback(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);

/**
 *	@brief	Class used for storing all relevant information and handles pertinent to one request.
 *			Also handles all winhttp api calls for creating connection and request handles
 *			and for sending requests. Not intended to be used by end users.
 */ 
class ActiveConnection {
public:
	enum class SendFlag {
		with_credentials,
		without_credentials
	};

	ActiveConnection(HINTERNET hSession, std::wstring _url, std::wstring _verb, std::wstring _path, std::string _data, std::function<void(HttpResponse)> _on_response_received) :
		url(_url),
		verb(_verb),
		path(_path),
		data(_data),
		on_response_received(_on_response_received)
	{
		connection = create_winhttp_connection(hSession, url);
		request = create_winhttp_request(connection, verb, path);
	}

	~ActiveConnection() {
  		WinHttpCloseHandle(request);
  		WinHttpCloseHandle(connection);
	}

	// Copy semantics are defaulted here, but COPYING SHOULD ONLY BE USED FOR SHARED_PTRS.
	// This is because upon destruction of ANY copied ActiveConnection, the connection and 
	// request HINTERNET handles will become invalid for ALL copies of that ActiveConnection.
	// This could be made more safe by defining custom move semantics and deleting copy semantics
	// but I don't feel comfortable enough to implement that, and shared_ptrs give me exactly
	// the kind of behaviour I want, with destructor being called upon the last copy of the 
	// object going out of scope or being erased. Because of this, I am choosing the easier,`
	// but less safe approach.
	ActiveConnection(const ActiveConnection&) = default;
	ActiveConnection& operator=(const ActiveConnection&) = default;
	ActiveConnection(ActiveConnection&&) = default;
	ActiveConnection& operator=(ActiveConnection&& other) = default;

	HINTERNET get_connection() {
		return connection;
	}

	HINTERNET get_request() {
		return request;
	}

	void invoke_callback(HttpResponse response) {
		on_response_received(response);
	}

	void send(SimpleHttpClient* client, SendFlag flag = SendFlag::without_credentials) {
		if (flag == SendFlag::with_credentials) apply_credentials(request);

		BOOL bResults = WinHttpSendRequest(
			request,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			(LPVOID)data.c_str(),
			(DWORD)strlen(data.c_str()),
			(DWORD)strlen(data.c_str()),
			(DWORD_PTR)client // Pass pointer to client object so we can find our way home from the callback
		);

		if (!bResults && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST) {
			// I couldn't imagine how this may occur but this is theoretically a 
			// potential infinite loop
			send(client, flag);
		}
	}

	void reset_request() {
		WinHttpCloseHandle(request);
		request = create_winhttp_request(connection, verb, path);
	}

	void set_credentials(std::wstring _user, std::wstring _pass) {
		user = _user;
		pass = _pass;
	}

private:
	HINTERNET connection;
	HINTERNET request;
	std::function<void(HttpResponse)> on_response_received;

	std::wstring url;
	std::wstring verb;
	std::wstring path;
	std::string data;

	std::wstring user;
	std::wstring pass;

	HINTERNET create_winhttp_connection(HINTERNET hSession, std::wstring url) {
		HINTERNET hConnect = WinHttpConnect(
			hSession,
			url.c_str(),
			//ALWAYS USE HTTPS. This class uses basic authentication, and so our authentication is not encrypted by default.
			// Using http would leave us vulnerable to someone obtaining authentication info by listening to our communication
			INTERNET_DEFAULT_HTTPS_PORT,
			0);

		if (!hConnect) {
			freak_out();
		}

		return hConnect;
	}

	HINTERNET create_winhttp_request(HINTERNET hConnect, std::wstring verb, std::wstring path) {
		HINTERNET hRequest = WinHttpOpenRequest(
			hConnect,
			verb.c_str(),
			path.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE
		);

		if (!hRequest) {
			freak_out();
		}

		attach_callback(hRequest);

		return hRequest;
	}

	void attach_callback(HINTERNET hRequest) {
		if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(
			hRequest,
			winhttp_callback,
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
			NULL
		))
		{
			freak_out();
		}
	}

	void apply_credentials(HINTERNET hRequest) {
		if (user.size() > 0 && pass.size() > 0) {
			WinHttpSetCredentials(
				hRequest,
				WINHTTP_AUTH_TARGET_SERVER,
				WINHTTP_AUTH_SCHEME_BASIC,
				user.c_str(),
				pass.c_str(),
				NULL
			);
		}
	}
};

/**
 *	@brief	Just a thread safe container for accessing active connections. 
 *			Not intended to be used by end users.
 */	
class ActiveConnectionsContainer {
public:
	void push_back(std::shared_ptr<ActiveConnection> conn) {
		mutex.lock();

		active_connections.push_back(conn);

		mutex.unlock();
	}

	void set_credentials(std::wstring user, std::wstring pass) {
		mutex.lock();

		for (std::shared_ptr<ActiveConnection> conn : active_connections) {
			conn->set_credentials(user, pass);
		}

		mutex.unlock();
	}

	std::shared_ptr<ActiveConnection> get(HINTERNET hRequest) {
		mutex.lock();

		auto conn_iter = std::find_if(active_connections.begin(), active_connections.end(), [hRequest](std::shared_ptr<ActiveConnection> conn) { return conn->get_request() == hRequest; });
		
		if (conn_iter == active_connections.end()) throw std::out_of_range("Tried to access a connection that doesn't exist.");

		mutex.unlock();

		return *conn_iter;
	}

	void erase(HINTERNET hRequest) {
		mutex.lock();

		auto conn_iter = std::find_if(active_connections.begin(), active_connections.end(), [hRequest](std::shared_ptr<ActiveConnection> conn) { return conn->get_request() == hRequest; });
		active_connections.erase(conn_iter);

		mutex.unlock();
	}

private:
	std::vector<std::shared_ptr<ActiveConnection>> active_connections;

	std::mutex mutex;
};

/** 
 *	@brief	A class intended to isolate as much of the WinHttp API as possible from the
 *			rest of the app so fewer people will need to have their innocence tainted.
 *			Should be the only HTTP class that users interact with from this library. 
 */
class SimpleHttpClient {

public:

	SimpleHttpClient() {
		hSession = create_winhttp_session();
	}

	~SimpleHttpClient() {
		WinHttpCloseHandle(hSession);
	}

	// Deleting all copy and move operations because custom destructor (see 'Rule of 5')
	SimpleHttpClient(const SimpleHttpClient&) = delete;
	SimpleHttpClient& operator=(const SimpleHttpClient&) = delete;
	SimpleHttpClient(SimpleHttpClient&&) = delete;
	SimpleHttpClient& operator=(SimpleHttpClient&&) = delete;

	/**
	 *	@brief	Initiates a new request with given parameters. Upon successful completion of the request,
	 *			will execute the <fun> callback with the response body as the parameter. 
	 *	@param[in]	std::wstring url - The destination URL for the request. Does not include the body.
	 *				e.g. L"www.google.com"
	 *	@param[in]	std::wstring verb - One of the HTTP verbs in all uppercase letters. e.g. L"GET"
	 *	@param[in]	std::wstring path - The path of the requested resource, including the query string, if
	 *				there is any.
	 *	@param[in]	std::string data - The payload for the request.
	 *	@param[in]	std::function<void(HttpResponse)> fun - A callback function that will be executed upon 
	 *				successful completion of the request. Will be called regardless of HTTP success or 
	 *				error. Parameter contains the response body, information on whether the response was 
	 *				successful,  and failure information if unsuccessful. Can be a lambda expression or a 
	 *				function pointer. The callback will be executed from a separate thread, and as such, 
	 *				the callback MUST BE THREAD SAFE, or race conditions may occur. 
	 *
	 *	@note		To simplify C++'s weird function pointer voodoo, we recommend using lambdas for the callback.
	 *				The syntax for the lambda expression would be:
	 *					[&](HttpResponse response) { 
	 *						<Your code goes here. The HttpResponse struct is defined in 'IrisHttpResponseType.h'> 
	 *					}
	 *				This will give your lambda access to all variables in its current scope. Beware that the local scope 
	 *				may have exited by the time the callback is invoked. In such a situation, all variables within that
	 *				scope will have become invalid.
	 */
	void make_request(std::wstring url, std::wstring verb, std::wstring path = L"", std::string data = "", std::function<void(HttpResponse)> fun = [](HttpResponse) {}) {
		std::shared_ptr<ActiveConnection> conn = std::make_shared<ActiveConnection>(hSession, url, verb, path, data, fun);
		conn->set_credentials(user, pass);

		connections.push_back(conn); // This is a copy. Only safe because it is a shared_ptr

		conn->send(this);
	}

	/**
	 *	@brief	Applies credentials, such that any future responses that asks for credentials will be met
	 *			with the same request, but with the Authentication header included.
	 *	@param[in]	std::wstring user - The username.
	 *	@param[in]	std::wstring pass - The password.
	 */
	void set_credentials(std::wstring user, std::wstring pass) {
		this->user = user;
		this->pass = pass;

		connections.set_credentials(user, pass);
	}

	// Public only so WinHttp callback can access it. Cannot use a member function as a callback but need 
	// to be able to access members, so doing it this way. Not intended to be called by users.
	void _on_notification_callback(
		HINTERNET hRequest,
		DWORD code
	) {
		switch (code) {
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			if (!WinHttpReceiveResponse(hRequest, 0)) freak_out();
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			on_response_received(hRequest);
			break;

		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			freak_out();
			break;
		}
	}

private:
	HINTERNET hSession;

	ActiveConnectionsContainer connections;

	std::wstring user;
	std::wstring pass;

	HINTERNET create_winhttp_session() {
		HINTERNET hSession = WinHttpOpen(
			L"IrisLibrary",
			WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			WINHTTP_FLAG_ASYNC);

		if (!hSession) {
			freak_out();
		}

		return hSession;
	}

	void on_response_received(HINTERNET hRequest) {
		DWORD status = 0;
		DWORD bufSize = sizeof(DWORD);

		BOOL bResults = WinHttpQueryHeaders(
			hRequest,
			WINHTTP_QUERY_STATUS_CODE |
			WINHTTP_QUERY_FLAG_NUMBER,
			NULL,
			&status,
			&bufSize,
			NULL
		);

		if (!bResults) freak_out();

		std::shared_ptr<ActiveConnection> conn;

		switch (status) {
		case 200:
		case 201:
			// We did it!
			clean_up_and_invoke_callback({true, read_response(hRequest), ""}, hRequest);
			break;
		case 401:
			// Need to authenticate
			conn = connections.get(hRequest);
			conn->reset_request();
			conn->send(this, ActiveConnection::SendFlag::with_credentials);
			break;
		default:
			clean_up_and_invoke_callback({ false, read_response(hRequest), "Encountered unhandled HTTP response code: " + std::to_string(status) }, hRequest);
			break;
		}
	}

#define MAX_BUFFER_SIZE 4096
	std::string read_response(HINTERNET hRequest) {
		char buffer[MAX_BUFFER_SIZE + 1];
		DWORD dw_size = 0;

		std::string out;

		do {
			if (!WinHttpQueryDataAvailable(hRequest, &dw_size)) {
				freak_out();
			}

			if (dw_size) {
				DWORD bytes_read = 0;

				while (bytes_read < dw_size) {
					DWORD bytes_to_read = min(dw_size - bytes_read, MAX_BUFFER_SIZE);

					if (!WinHttpReadData(hRequest, buffer, bytes_to_read, NULL)) {
						freak_out();
					}

					buffer[bytes_to_read] = '\0';

					out += std::string(buffer);

					bytes_read += bytes_to_read;
				}
			}
		} while (dw_size > 0);

		return out;
	}

	void clean_up_and_invoke_callback(HttpResponse data, HINTERNET hRequest) {
		std::shared_ptr<ActiveConnection> conn = connections.get(hRequest);

 		conn->invoke_callback(data);
		connections.erase(hRequest);
	}
};


void __stdcall winhttp_callback(
	HINTERNET hRequest,
	DWORD_PTR dwContext,
	DWORD dwInternetStatus,
	LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	SimpleHttpClient* handler = reinterpret_cast<SimpleHttpClient*>(dwContext);
	handler->_on_notification_callback(hRequest, dwInternetStatus);
}