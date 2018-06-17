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

namespace TestTCPServer
{
    public static class HttpConfig
    {
        public static int port = 5000;
    }

    class Server
    {
        /// <summary>
        /// The main program loop
        /// </summary>
        public static void run()
        {
            // set the ip address range and port to listen on and define the type of socket to be used
            IPEndPoint ip = new IPEndPoint(IPAddress.Any, HttpConfig.port);
            Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            Console.WriteLine("Waiting for a client...");

            // have the socket listen to the endpoint
            socket.Bind(ip);
            socket.Listen(10);
            
            // while({ while() {}} only supports a single client
            // TODO: implement some form of threading to support multiple clients
            bool runServer = true;
            while (runServer)
            {
                // create a new socket to recieve data from a newly connected client
                Socket client = socket.Accept();
                IPEndPoint clientep = (IPEndPoint)client.RemoteEndPoint;
                Console.WriteLine("{0} >>> Connected at port {1}", clientep.Address, clientep.Port);

                // send an onConnect message to the client
                string welcome = "Welcome";
                byte[] dataSend = new byte[1024];
                dataSend = Encoding.ASCII.GetBytes(welcome);
                client.Send(dataSend, dataSend.Length, SocketFlags.None);
                Console.WriteLine("{0} >>> Sent welcome", clientep.Address);
                
                // listen for incoming requests from the client
                while (true)
                {
                    if (client.Connected == false)
                        break;
                    
                    // attempt to read data sent by the connected client
                    string messageReceived = readMessage(client, clientep);

                    if (messageReceived == "")
                        continue;
                    
                    // create a response to send to the client
                    string messageOut = createMessageResponse(messageReceived);

                    // attempt to send the response
                    sendMessage(client, clientep, messageOut);

                    runServer = handleMessageInternally(messageReceived, client, clientep);
                }
            } // END while(run)

            // TODO: disconnect all connected servers when exiting
            socket.Close();
        } // END run()

        /// <summary>
        /// Takes string data sent by a socket and uses it to determine changes to the server's internal state.
        /// </summary>
        /// <param name="message">Data that was sent by the client.</param>
        /// <param name="client">The socket that setn the data.</param>
        /// <param name="clientep">The address of the socket that sent the data.</param>
        /// <returns>HACK: Returns true as long as the server should continue running.</returns>
        private static bool handleMessageInternally(string message, Socket client, IPEndPoint clientep)
        {
            // handle received data internally
            if (message == "qqq") // client was closed
            {
                client.Close();
                Console.WriteLine("{0} >>> Disconnected", clientep.Address);
            }
            else if (message == "qqqs") // client was closed and server will close too
            {
                client.Close();
                Console.WriteLine("{0} >>> Disconnected", clientep.Address);
                return false;
            }
            else
            {
                Console.WriteLine("{0} >>> Says: " + message, clientep.Address);
            }

            return true;
        }

        /// <summary>
        /// Takes a string data sent by a socket and determines what to respond with.
        /// </summary>
        /// <param name="message">String sent by a socket.</param>
        /// <returns>Returns a string to send to the socket</returns>
        private static string createMessageResponse(string message)
        {
            // placeholder
            return "send successful";
        }

        /// <summary>
        /// Converts an input string to bytes then sends the bytes to the socket.
        /// </summary>
        /// <param name="client">The socket to send data to.</param>
        /// <param name="clientep">The address the socket is connected to.</param>
        /// <param name="message">The data to send.</param>
        /// <exception cref="SocketException">Thrown when connecting to a client fails</exception>
        private static void sendMessage(Socket client, IPEndPoint clientep, string message)
        {
            // a socket exception can force a client to disconnect
            // so check that the client is connected first
            if (client.Connected == false)
                return;

            try
            {
                client.Send(Encoding.ASCII.GetBytes(message));
            }
            catch (SocketException e)
            {
                handleSocketException(e, client, clientep);
            }
        }

        /// <summary>
        /// Reads incoming bytes from a socket then converts it to a string.
        /// </summary>
        /// <param name="client">The socket to read from.</param>
        /// <param name="clientep">The address the socket is connected to.</param>
        /// <returns>Returns a string representing the data that was sent.</returns>
        /// <exception cref="SocketException">Thrown when connecting to a client fails</exception>
        private static string readMessage(Socket client, IPEndPoint clientep)
        {
            string message = "";
            
            // a socket exception can force a client to disconnect
            // so check that the client is connected first
            if (client.Connected == false)
                return message;

            byte[] dataReceive = new byte[1024];
            try
            {
                // data will be copied directly into the buffer
                int len = client.Receive(dataReceive, SocketFlags.None);
                // convert the received data in a string
                // make sure to use the length returned by client.receive(...)
                message = Encoding.ASCII.GetString(dataReceive, 0, len);
            }
            catch (SocketException e)
            {
                handleSocketException(e, client, clientep);
            }

            return message;
        }

        /// <summary>
        /// Prints the message from caught exceptions then disconnected the socket that threw the exception.
        /// </summary>
        /// <param name="e">The exception that was thrown.</param>
        /// <param name="client">The socket that threw the exception.</param>
        /// <param name="clientep">The address the socket is connected to.</param>
        private static void handleSocketException(SocketException e, Socket client, IPEndPoint clientep)
        {
            // handle the event of a client disconnecting
            Console.WriteLine("{0} >>> Connection was lost", clientep.Address);
            Console.WriteLine(e.Message);
            Console.WriteLine(e.InnerException);

            // this can run more than once per loop so
            // check if client is connected before attempting to disconnect.
            if (client.Connected == true)
                client.Disconnect(true);

            return;
        }
    }
}
