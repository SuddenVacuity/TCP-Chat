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

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace TestTCPClient
{
    public static class HttpConfig
    {
        public static string baseAddress = "127.0.0.1";
        public static int port = 5000;
    }
    
    class Client
    {
        static string helpText = 
            "\nEnter text to send it to the server" + 
            "\nCommands" + 
            "\n   help - displays this text" + 
            "\n   qqq - Close the client" + 
            "\n   qqqs - Closes the server and the client";

        public static void run()
        {
            // FIRST initialize a socket to use to connect to the server
            Socket server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            Console.WriteLine(helpText);

            // SECOND attempt to connect to the server
            // the connection only needs to be established once
            // failed connection attempts should be handled here
            if (!connect(ref server))
            {
                Console.WriteLine("Unable to connect to a server");
                Console.ReadLine();
                return;
            }

            // send/receive/modify as needed
            // constantly take input to be sent to the server
            bool runClient = true;
            while (runClient)
            {
                // take in user input to be sent and handled internally
                string input = Console.ReadLine();

                // THIRD handle input and decide if data should be sent to the server
                // returns and empty string if there's nothing to send
                string message = createServerMessage(input, ref runClient);

                if (message == "")
                    continue;

                // FOURTH attempt to send data to the server
                // the server must have an active socket open connection to the client
                // data sent must be byte[] type
                sendMessage(server, message);
                
                // FIFTH wait to receive data as a response from the server
                // wait for a response from the server
                // when using receive the server must actually send a response or it will hang waiting
                string stringData = readMessage(server);

                // SIXTH handle the server response internally
                handleResponseInternally(stringData);
            }

            // LAST close the socket connection
            server.Shutdown(SocketShutdown.Both);
            server.Close();
        }

        private static string createServerMessage(string input, ref bool runClient)
        {
            // before data is sent then skip the remainder of the loop
            if (input == "help")
            {
                // don't send client help command
                Console.WriteLine(helpText);
                return "";
            }
            // quit condition
            if (input == "qqq")
                runClient = false;
            if (input == "qqqs")
                runClient = false;

            return input;
        }
        
        private static void handleResponseInternally(string message)
        {
            Console.WriteLine(message);
        }
        

       private static string readMessage(Socket server)
        {
            string message = "";

            // data received will be byte[] type
            byte[] data = new byte[1024];
            int receivedDataLength = 0;
            try
            {
                receivedDataLength = server.Receive(data);
                message = Encoding.ASCII.GetString(data, 0, receivedDataLength);
            }
            catch (SocketException e)
            {
                handleSocketException(e);
            }

            return message;
        }

        private static void sendMessage(Socket server, string message)
        {
            try
            {
                server.Send(Encoding.ASCII.GetBytes(message));
            }
            catch (SocketException e)
            {
                handleSocketException(e);
            }
        }
        
        // attempts to make a socket connection to the server
        private static bool connect(ref Socket server)
        {
            // create endpoint to connect to
            IPEndPoint ip = new IPEndPoint(IPAddress.Parse(HttpConfig.baseAddress), HttpConfig.port);

            // create buffer to contain server response
            byte[] data = new byte[1024];
            int receivedDataLength = 0;

            // attempt to connect to the server
            try
            {
                server.Connect(ip);

                // receive the servers response to connecting
                receivedDataLength = server.Receive(data);
            }
            catch (SocketException e)
            {
                handleSocketException(e);
                return false;
            }

            // handle connection response internally
            string message = Encoding.ASCII.GetString(data, 0, receivedDataLength);
            Console.WriteLine(message);

            return true;
        }

        private static void handleSocketException(SocketException e)
        {
            Console.WriteLine(e.InnerException);
            Console.ReadLine();
        }

    }
}
