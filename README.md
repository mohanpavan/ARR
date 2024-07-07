### Task Description
Implement an application that involves a server and a client that communicate over sockets by sending ethernet packets.

#### Ethernet Packets
The ethernet packet needs to have the layout of a proper ethernet packet. The source and destination MAC addresses need to be command-line arguments. The payload of each packet should be a string. The first bit should be 0 for the "accepting ethernet packet" action and 1 for the "retrieve a stored ethernet packet" one. The rest 7 bits of the message should contain an identifier and the remainder should be the actual message.

#### Server
The server needs to provide one socket for interacting with the clients. Once a packet arrives, the server needs to extract the payload and then identify the action by the first bit:
* If it's 0, the server needs to extract the ID and the message and store both in the buffer.
* If it's 1, the server needs to extract the ID and send back the corresponding message stored in the buffer. If there's no message associated with the requested ID, you are free to choose what to return. Any data after the 8th bit of the request should be omitted in this case.

#### Client
The client needs to provide one socket for interacting with the server. For each action the client wants to take, they need to create an ethernet packet based on the format explained in the `Ethernet Packets` section and send it over the socket to the server.

#### Buffer
The buffer stores collections of IDs and messages, has a maximum capacity and provides two functions:
* A setter which accepts the ID and the message of the ethernet packet and stores them.
* A getter which accepts an ID and returns the corresponding message, if it exists. If the ID doesn't exist, it should return a message of your choice.

<b>Note</b>: A setter can succeed only if there is enough space in the buffer, namely the capacity is not full. If it's full, the buffer should delete the first inserted element before inserting the new one.

##### Notes
* Use C++17 or higher.
* Use CMake for building the application.
* Assume Ubuntu 22.04 as the base system.
* Please invest 10-12 hours at most.

#### Deliverables:
* Source code.
* Necessary files for building the application.
* Provide your solution as a git repository, please do not use public hosting services like github or gitlab. Instead, send back an archive with the .git directory included. Ensure that commit history reflects the progression of work on a real task.

