<h3>Goal for this project</h3>
The purpose of this project is to create a client/server pair where a server accepts clients,</br>
receives data from each client and sends the combined data from each client to each client.</br>

<h3>Steps to achieve project goal</h3>
1. Create a simple server that accepts a single client where the server can receive data from the client.</br>
2. Have the created server accept multiple clients and can receive data from all clients.</br>
3. Have the server/client pair collect and update data in a way that clients do not miss or receive duplicate data.</br>

<h3>Information on usage</h3>
By default the client will try to connect to a server running on the same machine(localhost).</br>
</br>
When using C++ client through the command line an alternate address can be entered.</br>

>Currently clients can only access servers on the same network.</br>

Start up the server on the desired machine then find its ip4 address.</br>
To find the machine's ip4 address open a terminal and based on your OS use the following command.</br>

 | OS       | Command        | Look for     |
 | ---      | ---            | ---          |
 | Windows  | ipconfig       | IPv4 Address |
 | Mac      | ifconfig       | inet         |
 | Linux    | nmcli dev list | IP4.ADDRESS  |
 | Ubuntu   | nmcli dev show | IP4.ADDRESS  |

This will give you an address that looks something like 192.168.1.xxx</br>
where xxx is a number unique to that machine on your network.</br>

>On a wireless network the address 192.168.1.1 is normally the address</br>
>of the wireless router and shouldn't be used.</br>

Then on the machine that will run the client open a terminal and enter:</br>
path\to\executable\Client.exe 192.168.1.xxx</br>
