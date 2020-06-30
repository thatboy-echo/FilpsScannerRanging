#include <httplib.h>
#include <iostream>

int main()
{
	using namespace httplib;
	using namespace std;
	Server server;
	server.set_mount_point("/", ".");
	server.bind_to_port("localhost", 8089);
	server.Post("/json", [&](const Request& req, Response& res)
		{
			cout << req.body << endl;
			res.set_content(R"({"type":"POST","info":"hello"})", "application/json");
		});

	server.Get("/json", [&](const Request& req, Response& res)
		{
			cout << req.body << endl;
			res.set_content(R"({"type":"GET","info":"hello"})", "application/json");
		});
	server.listen_after_bind();
}