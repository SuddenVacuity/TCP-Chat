using System;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace TestTCPClient
{
    static public class HttpConfig
    {
        static public string baseAddress = "127.0.0.1";
        static public int port = 5000;
    }
    
    class Client
    {
        static string helpText = 
            "\nEnter text to send it to the server" + 
            "\nCommands" + 
            "\n   help - displays this text" + 
            "\n   qqq - Close the client" + 
            "\n   qqqs - Closes the server and the client";

        static public void run()
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
            while (true)
            {
                // take in user input to be sent and handled internally
                string input = Console.ReadLine();
                
                // THIRD handle any input that should not be sent to the server
                // before data is sent then skip the remainder of the loop
                if (input == "help")
                {
                    // don't send client help command
                    Console.WriteLine(helpText);
                    continue;
                }

                // FOURTH attempt to send data to the server
                // the server must have an active socket open connection to the client
                // data sent must be byte[] type
                try
                {
                    server.Send(Encoding.ASCII.GetBytes(input));
                }
                catch (SocketException e)
                {
                    handleSocketException(e);
                    break;
                }

                // FIFTH handle input the was sent to the server internally
                if (input == "exit")
                    break;
                if (input == "closeserverandexit")
                    break;

                // SIXTH wait to receive data as a response from the server
                // wait for a response from the server
                // when using receive the server must actually send a response or it will hang waiting
                // data received will be byte[] type
                byte[] data = new byte[1024];
                int receivedDataLength = 0;
                try
                {
                    receivedDataLength = server.Receive(data);
                }
                catch (SocketException e)
                {
                    handleSocketException(e);
                    break;
                }

                // SEVENTH handle the server response internally
                string stringData = Encoding.ASCII.GetString(data, 0, receivedDataLength);
                Console.WriteLine(stringData);
            }

            // LAST close the socket connection
            server.Shutdown(SocketShutdown.Both);
            server.Close();
        }
        
        // attempts to make a socket connection to the server
        static private bool connect(ref Socket server)
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
