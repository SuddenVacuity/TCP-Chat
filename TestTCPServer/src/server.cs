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
    static public class HttpConfig
    {
        static public int port = 5000;
    }

    class Server
    {
        static public void run()
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
            bool run = true;
            while (run)
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
                    
                    // create a buffer to hoad data
                    // TODO: make buffer safe for varying data lengths
                    byte[] dataReceive = new byte[1024];
                    int len = 0;

                    // attempt to read data sent by the connected client
                    try
                    {
                        // data will be copied directly into the buffer
                        len = client.Receive(dataReceive, SocketFlags.None);
                    }
                    catch (SocketException e)
                    {
                        handleSocketException(e, client, clientep);
                        break;
                    }
                    
                    // create a response to send to the client
                    string messageOut = "send successful";
                    byte[] response = new byte[1024];
                    response = Encoding.ASCII.GetBytes(messageOut);

                    // attempt to send the response
                    try
                    {
                        client.Send(response);
                    }
                    catch (SocketException e)
                    {
                        handleSocketException(e, client, clientep);
                        break;
                    }

                    // convert the received data in a string
                    // make sure to use the length returned by client.receive(...)
                    string messageReceived = Encoding.ASCII.GetString(dataReceive, 0, len);

                    // hadle received data internally
                    if (messageReceived == "qqq")
                    {
                        client.Close();
                        Console.WriteLine("{0} >>> Disconnected", clientep.Address);
                        break;
                    }
                    if (messageReceived == "qqqs")
                    {
                        client.Close();
                        Console.WriteLine("{0} >>> Disconnected", clientep.Address);
                        run = false;
                        break;
                    }

                    Console.WriteLine("{0} >>> Says: " + messageReceived, clientep.Address);
                }
            } // END while(run)

            // TODO: disconnect all connected servers when exiting
            socket.Close();
        } // END run()

        private static void handleSocketException(SocketException e, Socket client, IPEndPoint clientep)
        {
            Console.WriteLine("{0} >>> Connection was lost", clientep.Address);
            Console.WriteLine(e.Message);
            Console.WriteLine(e.InnerException);

            client.Disconnect(true);
            return;
        }
    }
}
