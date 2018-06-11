//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2018 Gerald Coggins
//

#include <iostream>
#include <string>
#include <asio.hpp>

namespace TestTCPClient_CPP
{
	using asio::ip::tcp;
	const std::string endMessageToken = "\n";

	void printSocketException(asio::system_error& e)
	{
		std::cout << "ERROR asio::system_error:" << std::endl;
		std::cerr << e.what() << std::endl;
	}

	void runClient(const std::string& baseAddress)
	{
		asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints =
			resolver.resolve(baseAddress, "5000");

		tcp::socket socket(io_context);


		// attempt to connect to the server
		asio::error_code errorConnect;
		try
		{
			asio::connect(socket, endpoints, errorConnect);

			asio::streambuf buf;
			size_t len = asio::read_until(socket, buf, endMessageToken, errorConnect);

			// print received message
			std::cout << "Server says: ";
			std::cout.write((char*)buf.data().data(), len) << std::endl;
		}
		catch (asio::system_error& e)
		{
			std::cout << "CONNECT TO SERVER FAILED: " << baseAddress << std::endl;
			printSocketException(e);
		}

		std::cout << "Type to send a message to the server\n\tType qqq to quit\n\tType qqqs to close client and server\n";

		while (true)
		{
			// take user input to send to the server
			std::string input;
			std::cin >> input;
			
			// attempt to send a message to the server
			asio::error_code errorOut;
			try
			{
				asio::write(socket, asio::buffer(input + endMessageToken), errorOut);
			}
			catch (asio::system_error& e)
			{
				std::cout << "SEND MESSAGE TO SERVER FAILED: " << baseAddress << std::endl;
				printSocketException(e);
			}

			if (input == "qqq")
				break;
			if (input == "qqqs")
				break;

			// create buffer to hold sent data
			asio::streambuf buf;

			// attempt to read a message from the client
			asio::error_code errorIn;
			try
			{
				size_t len = asio::read_until(socket, buf, endMessageToken, errorIn);

				// handle failed receive
				if (errorIn == asio::error::eof)
					break; // Connection closed cleanly by peer.
				else if (errorIn)
					throw asio::system_error(errorIn); // Some other error.

				// print received message
				std::cout << "Server says: ";
				std::cout.write((char*)buf.data().data(), len);
				std::cout << std::endl;

			}
			catch (asio::system_error& e)
			{
				std::cout << "RECEIVE MESSAGE FROM SERVER FAILED: " << baseAddress << std::endl;
				printSocketException(e);
			}
		} // END while(true)
	}
}

int main(int argc, char* argv[])
{
	// default address to local host
	std::string serverAddress = "127.0.0.1";

	// set address through command arguments
	if (argc == 2)
	{
		serverAddress = argv[1];
	}

	std::cerr << "Usage: client <host>: " << serverAddress << std::endl;

	TestTCPClient_CPP::runClient(serverAddress);

	return 0;
}
