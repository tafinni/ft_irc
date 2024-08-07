#include "inc/Server.hpp"

void valueCheck(std::string port, std::string password)
{
	for (char c : port)
		if (!std::isdigit(c))
			throw std::runtime_error("Invalid port (allowed: 1024-65535)");
	if (std::atoi(port.c_str()) < 1024 || std::atoi(port.c_str()) > 65535)
			throw std::runtime_error("Invalid port (allowed: 1024-65535)");
	if (password.size() <= 0 || password.size() > 30)
		throw std::runtime_error("Allowed password lenght 1-30 characters");
	if (std::string(password).find(' ') != std::string::npos || std::string(password).find('	') != std::string::npos)
		throw std::runtime_error("No empty spaces allowed in password");

}

int main(int argc, char **argv)
{

	try
	{
		if (argc != 3)
			throw std::runtime_error("Usage: ./ircserv [port] [password]");
		valueCheck(argv[1], argv[2]);

		Server server(std::atoi(argv[1]), argv[2]);

		server.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
