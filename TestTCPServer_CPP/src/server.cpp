//
// server.cpp
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

namespace TestTCPServer_CPP
{
	using asio::ip::tcp;
	// the size of the packets to be sent
	// any data larger than this will be split into
	// multiple packages and sent sequencially
	const size_t packetSize = 128;
	char buf[packetSize] = {};

	void printSocketException(asio::system_error& e)
	{
		std::cout << "ERROR asio::system_error:" << std::endl;
		std::cerr << e.what() << std::endl;
	}

	// prints the input string to the console
	// returns true as long as the the server should continue running
	bool handleClientMessage(const std::string& message, tcp::socket& socket)
	{
		if (message == "qqqs")
		{
			return false;
		}
		if (message == "qqq")
		{
			std::cout << "Client has disconnected: " << socket.remote_endpoint().address().to_string() << std::endl;
			socket.shutdown(asio::socket_base::shutdown_type::shutdown_both);
			socket.close();
		}
		else
		{
			std::cout << "Client says: " << message << std::endl;
			// create message to send
			std::string messageOut = "message recieved";

			asio::error_code errorOut;

			// attempt to send a message to the client
			try
			{
				asio::write(socket, asio::buffer(messageOut), errorOut);

				// if write fails print message and close socket
				if (errorOut)
				{
					std::cout << "CONNECTION TO CLIENT LOST: " << socket.remote_endpoint().address().to_string() << std::endl;
					socket.close();
				}
			}
			catch (asio::system_error& e)
			{
				std::cout << "SEND MESSAGE TO CLIENT FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
				printSocketException(e);
			}
		}

		return true;
	}

	void acceptNewClient(tcp::acceptor& acceptor, tcp::socket& socket)
	{
		acceptor.accept(socket);
		std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << std::endl;

		// create message to be sent on connect
		std::string messageOut = "Welcome";

		// attempt to send a message to the client
		asio::error_code errorOut;
		try
		{
			asio::write(socket, asio::buffer(messageOut), errorOut);

			// if write fails print message and close socket
			if (errorOut)
			{
				std::cout << "CONNECTION TO CLIENT LOST: " << socket.remote_endpoint().address().to_string() << std::endl;
				socket.close();
			}
		}
		catch (asio::system_error& e)
		{
			printSocketException(e);
		}
	}

	void runServer()
	{
		asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v6(), 5000));

		tcp::socket socket(io_service);
		asio::error_code ec;
		socket.set_option(asio::socket_base::reuse_address(true), ec);

		acceptNewClient(acceptor, socket);

		while (true)
		{
			if(socket.is_open() == false)
			{
				acceptNewClient(acceptor, socket);
			}

			// read sent message
			asio::error_code errorIn;

			// attempt to read a message from the client
			try
			{
				// read from client
				size_t len = socket.read_some(asio::buffer(buf, packetSize), errorIn);

				// if read fails print message and close socket
				if(errorIn)
				{
					std::cout << "CONNECTION TO CLIENT LOST: " << socket.remote_endpoint().address().to_string() << std::endl;
					socket.close();
					continue;
				}
				
				// handle the sent message interanlly
				// HACK: returns true as long as the server should keep running
				if (!handleClientMessage(std::string(buf, len), socket))
					break;

				// clear the data from the buffer between uses
				memset(buf, 0, packetSize);
			}
			catch (asio::system_error& e)
			{
				std::cout << "CONNECTION TO CLIENT FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
				printSocketException(e);
			}
		}
	} // END runServer()
}

int main()
{
	TestTCPServer_CPP::runServer();
	return 0;
}
