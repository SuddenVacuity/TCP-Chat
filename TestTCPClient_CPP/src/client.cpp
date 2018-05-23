#include <iostream>
#include <boost/array.hpp>
#include <asio.hpp>

namespace TestTCPClient_CPP
{
	using asio::ip::tcp;

	void printSocketException(std::exception& e, asio::error_code& error)
	{
		std::cout << "ERROR std::exception:" << std::endl;
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
			boost::array<char, 128> buf;

			asio::connect(socket, endpoints, errorConnect);

			size_t len = socket.read_some(asio::buffer(buf), errorConnect);

			// print received message
			std::cout << "Server says: ";
			std::cout.write(buf.data(), len);
		}
		catch (std::exception& e)
		{
			std::cout << "CONNECT TO SERVER FAILED: " << baseAddress << std::endl;
			printSocketException(e, errorConnect);
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
				asio::write(socket, asio::buffer(input), errorOut);
			}
			catch (std::exception& e)
			{
				std::cout << "SEND MESSAGE TO SERVER FAILED: " << baseAddress << std::endl;
				printSocketException(e, errorOut);
			}

			if (input == "qqq")
				break;
			if (input == "qqqs")
				break;

			// create buffer to hold sent data
			boost::array<char, 128> buf;

			// attempt to read a message from the client
			asio::error_code errorIn;
			try
			{
				size_t len = socket.read_some(asio::buffer(buf), errorIn);

				// handle failed receive
				if (errorIn == asio::error::eof)
					break; // Connection closed cleanly by peer.
				else if (errorIn)
					throw asio::system_error(errorIn); // Some other error.

				// print received message
				std::cout << "Server says: ";
				std::cout.write(buf.data(), len);
				std::cout << std::endl;

			}
			catch (std::exception& e)
			{
				std::cout << "RECEIVE MESSAGE FROM SERVER FAILED: " << baseAddress << std::endl;
				printSocketException(e, errorIn);
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
