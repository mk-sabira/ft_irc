# ft_irc

A real-time chat server built in C++98, implementing the IRC protocol (RFC 2812) for client communication via text-based channels. 

Overview

This server enables clients to connect, join channels, and exchange messages. 
Key features:
  TCP Socket Programming: Uses socket(), bind(), listen() for reliable connections.
  Non-blocking I/O: Handles multiple clients with poll().
  
IRC Commands: 
  PASS
  NICK
  USER
  JOIN
  PRIVMSG
  TOPIC
  INVITE
  KICK
  MODE
  PART
  PING/PONG
  QUIT.

Channel Modes: 
  i (invite-only)
  t (topic)
  k (key)
  o (operator)
  l (limit).

Tested with nc and LimeChat; no memory leaks (Valgrind-verified).

Prerequisites:
  OS: Linux/Unix
  Compiler: g++ (C++98 standard).

Tools: 
  make
  nc (Netcat)
  Valgrind.
  
Port:
  6667 (default).


Setup

Clone the repository:
  git clone [YOUR_REPOSITORY_LINK]
  cd irc-server

Compile the server:
  make

Run the server (port 6667, password (example: 123):
  ./ircserv 6667 123



Testing

Connect with nc:
  nc -C localhost 6667
  
Register and join a channel:
  PASS 123
  NICK Alice
  USER alice 0 * Alice Smith
  JOIN #test
  PRIVMSG #test :Hello!

Expected: :Alice!alice@localhost JOIN :#test and message broadcast.

Test channel commands (in another nc session):

Connect as Bob (NICK Bob, etc.).

Alice:
  INVITE Bob #test, 
  TOPIC #test :Welcome, 
  MODE #test +i, 
  KICK #test Bob :Bye.

Bob: Try JOIN #test (should fail if +i set until invited).

Check memory leaks:
  valgrind --leak-check=full ./ircserv 6667 123

Resources:
  RFC 2812 – IRC Protocol
  Beej’s Guide to Network Programming
  Computer Networks by Douglas E. Comer
  LimeChat (reference client)
  Libera.Chat (reference server)
