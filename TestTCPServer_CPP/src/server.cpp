//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _CRT_SECURE_NO_WARNINGS
// define this to use depreciated function ctime()
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>
#include <boost/array.hpp>

namespace TestTCPServer_CPP
{
	using asio::ip::tcp;

	std::string make_daytime_string()
	{
		time_t now = time(0);
		return ctime(&now);
	}

	void printSocketException(std::exception& e)
	{
		std::cout << "ERROR std::exception:" << std::endl;
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
			}
			catch (std::exception& e)
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
		}
		catch (std::exception& e)
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
			boost::array<char, 128> buf;
			asio::error_code errorIn;

			// attempt to read a message from the client
			try
			{
				size_t len = socket.read_some(asio::buffer(buf), errorIn);

				// if message was successfully received
				// handle the message internally
				if (!errorIn)
				{
					std::string received(buf.begin(), buf.begin() + len);

					if (!handleClientMessage(received, socket))
						break;
				}
			}
			catch (std::exception& e)
			{
				std::cout << "RECEIVE MESSAGE FROM CLIENT FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
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
