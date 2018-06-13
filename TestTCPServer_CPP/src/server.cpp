/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
This software is released under the MIT License
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Copyright (c) 2018 Gerald Coggins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

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

	// prints asio system errors
	//		[e] - the system error to be printed
	void printSocketException(asio::system_error& e)
	{
		std::cout << "ERROR asio::system_error:" << std::endl;
		std::cerr << e.what() << std::endl;
	}

	// attempts to send a message to the client through a socket
	//		[socket] - the socket the data will be sent through
	//		[message] - the data to be sent
	//		[packetSize] - data larger than this will be broken into multiple writes
	void sendMessage(tcp::socket &socket, const std::string &message, const size_t &packetSize)
	{
		// attempt to send a message to the client
		try
		{
			asio::error_code errorOut;
			asio::write(socket, asio::buffer(message), errorOut);

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

	// attempts to read the client's response to a sent message
	//		[socket] - the socket the data will be received through
	//		[packetSize] - data larger than this will be broken into multiple reads
	size_t readMessage(tcp::socket &socket, const size_t &packetSize)
	{
		size_t len = 0;
		// attempt to read a message from the client
		try
		{
			// read from client
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
			std::cout << "CONNECTION TO CLIENT FAILED: " << socket.remote_endpoint().address().to_string() << std::endl;
			printSocketException(e);
		}

		return len;
	}

	// prints the input string to the console
	// HACK: returns true as long as the the server should continue running
	//		[message] - the message that was received
	//		[socket] - the socket connected to the client that the message came through
	bool handleClientMessage(const std::string& message, tcp::socket& socket)
	{
		// the client closed and the server will close too
		if (message == "qqqs")
		{
			// make the socket available for reuse
			socket.shutdown(asio::socket_base::shutdown_type::shutdown_both);
			socket.close();

			// HACK: quit the server
			return false;
		}
		// the client was closed and it's socket should be made available
		if (message == "qqq")
		{
			std::cout << "Client has disconnected: " << socket.remote_endpoint().address().to_string() << std::endl;

			// make the socket available for reuse
			socket.shutdown(asio::socket_base::shutdown_type::shutdown_both);
			socket.close();
		}
		// catch-all block for any other input
		else
		{
			std::cout << "Client says: " << message << std::endl;

			// create message to send
			std::string messageOut = "message recieved";
			sendMessage(socket, messageOut, packetSize);
		}

		// clear the data from the buffer between uses
		memset(buf, 0, packetSize);

		return true;
	}

	// assigns a connecting client to an available socket
	//		[acceptor] - 
	//		[socket] - the socket to connect to a client through
	void acceptNewClient(tcp::acceptor& acceptor, tcp::socket& socket)
	{
		acceptor.accept(socket);
		std::cout << "Connected: " << socket.remote_endpoint().address().to_string() << std::endl;

		// create message to be sent on connect
		std::string messageOut = "Welcome";

		sendMessage(socket, messageOut, packetSize);
	}

	// creates io service, attaches clients to sockets and loops read/handle messages
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
			size_t len = readMessage(socket, packetSize);

			// check if nothing to handle
			if(len == 0)
				continue;

			// handle the sent message interanlly
			// HACK: returns true as long as the server should keep running
			if (!handleClientMessage(std::string(buf, len), socket))
				break;
		}
	} // END runServer()
} // END namespace TestTCPServer_CPP

// program entry point
int main()
{
	TestTCPServer_CPP::runServer();
	return 0;
}
