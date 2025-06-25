#pragma once

#include "Webserv.hpp"
struct HttpRequest;

class Get
{
private:
	const HttpRequest& m_req;

public:
	Get(const HttpRequest& req);

	std::string response() const;
	~Get();
};
