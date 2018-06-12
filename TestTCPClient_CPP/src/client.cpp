//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2018 Gerald Coggins
//

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <iostream>
#include <string>
#include <asio.hpp>

namespace TestTCPClient_CPP
{
	using asio::ip::tcp;
	// the size of the packets to be sent
	// any data larger than this will be split into
	// multiple packages and sent sequencially
	const size_t packetSize = 128;
	char buf[packetSize] = {};

	void handleMessage(size_t length)
	{
		std::cout << "Server says: ";
		std::cout.write(buf, length) << std::endl;

		memset(buf, 0, length);
	}

	void printSocketException(asio::system_error& e)
	{
		std::cout << "ERROR asio::system_error:" << std::endl;
		std::cerr << e.what() << std::endl;
	}

	void connect(tcp::socket &socket, 
		tcp::resolver::results_type endpoints)
	{
		// attempt to connect to the server
		try
		{
			asio::error_code errorConnect;
			asio::connect(socket, endpoints, errorConnect);

		}
		catch (asio::system_error& e)
		{
			std::cout << "CONNECT TO SERVER FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
			printSocketException(e);
		}
	}

	void sendMessage(tcp::socket &socket, 
		const std::string &message, 
		const size_t &packetSize)
	{
		// attempt to send a message to the server
		try
		{
			asio::error_code errorOut;
			asio::write(socket, asio::buffer(message, packetSize), errorOut);

			// if write fails print message and close socket
			if (errorOut)
			{
				std::cout << "CONNECTION TO CLIENT LOST: " << socket.remote_endpoint().address().to_string() << std::endl;
				socket.close();
			}
		}
		catch (asio::system_error& e)
		{
			std::cout << "SEND MESSAGE TO SERVER FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
			printSocketException(e);
		}
	}

	size_t readMessage(tcp::socket &socket, const size_t &packetSize)
	{
		size_t len = 0;
		// attempt to read a message from the client
		try
		{
			asio::error_code errorIn;
			len = socket.read_some(asio::buffer(buf, packetSize), errorIn);

			// check for failed receive
			if (errorIn == asio::error::eof) {} // Connection closed cleanly by peer. 
			else if (errorIn)
			{
				std::cout << "CONNECTION TO CLIENT LOST: " << socket.remote_endpoint().address().to_string() << std::endl;
				socket.close();
			}
		}
		catch (asio::system_error& e)
		{
			std::cout << "RECEIVE MESSAGE FROM SERVER FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
			printSocketException(e);
		}

		return len;
	}

	void runClient(const std::string& baseAddress)
	{
		asio::io_context io_context;
		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints =
			resolver.resolve(baseAddress, "5000");

		tcp::socket socket(io_context);

		connect(socket, endpoints);

		size_t len = readMessage(socket, packetSize);
		handleMessage(len);

		std::cout << "Type to send a message to the server\n\tType qqq to quit\n\tType qqqs to close client and server\n";

		while (true)
		{
			// take user input to send to the server
			std::string input;
			std::cin >> input;
			
			sendMessage(socket, input, packetSize);

			if (input == "qqq")
				break;
			if (input == "qqqs")
				break;

			size_t len = readMessage(socket, packetSize);
			handleMessage(len);
		} // END while(true)
	} // END runClient()
} // END namespace TestTCPClient_CPP

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
