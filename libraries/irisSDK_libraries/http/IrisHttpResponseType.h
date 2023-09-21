#pragma once

/**
 *	@brief	A struct containing response info from Http requests made through
 *			IrisSDK libraries
 */
struct HttpResponse {
	// Did the response complete successfully? (HTTP response code = 2xx?)
	bool completed_successfully = false;
	// The body of the request response, if it exists
	std::string body = "";
	// Includes a brief description of the error that was encountered
	std::string failure_string = "";

	HttpResponse(bool _completed_successfully, std::string _body, std::string _failure_string) :
		completed_successfully(_completed_successfully),
		body(_body),
		failure_string(_failure_string)
	{}
};