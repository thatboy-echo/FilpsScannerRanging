#include <httplib.h>
#include <iostream>
#include "../../../Logger/Logger/Logger.hpp"

thatboy::logger::FileLogger logger{ "server.log" };

#define LOG_INFO(...) logger.log(thatboy::logger::LogLevel::Info, ##__VA_ARGS__)
#define LOG_WARNING(...) logger.log(thatboy::logger::LogLevel::Warning, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__)
#define LOG_ERROR(...) logger.log(thatboy::logger::LogLevel::Error, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__)
#define LOG_FATAL(...) logger.log(thatboy::logger::LogLevel::Fatal, "@\"", __FILE__, "\":", __FUNCTION__, "<", __LINE__, ">: ", ##__VA_ARGS__)


int main()
{
	using namespace httplib;
	using namespace std;
	Server server;
	server.set_mount_point("/", ".");
	server.bind_to_port("localhost", 8089);
	server.Post("/map/all", [&](const Request& req, Response& res)
		{
			cout << "[POST /map/all]" << req.remote_addr << ":" << req.remote_port << endl;
			LOG_INFO(req.method, "@", req.path, " from ", req.remote_addr, ":", req.remote_port, "{", req.body, "}");
			res.set_content(R"({"type":"POST","info":"ok"})", "application/json");
		});

	server.Get("/map/all", [&](const Request& req, Response& res)
		{
			cout << "[GET /map/all]" << req.remote_addr << ":" << req.remote_port << endl;
			LOG_INFO(req.method, "@", req.path, " from ", req.remote_addr, ":", req.remote_port, "{", req.body, "}");
			res.set_content(R"({"type":"GET","info":"ok"})", "application/json");
		});
	server.listen_after_bind();
}