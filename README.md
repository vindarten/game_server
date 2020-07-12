# Multiplayer game server
This project is an implementation of a server program that plays the role of a host in the Manager game(http://www.stolyarov.info/books/pdf/gameserv.pdf) and allows players on different machines to take part in the game using the local network.

The project also includes a programmable robot that can take part in the game, simulating the actions of human players. The directory './robots/' contains examples of scripts of various difficulty levels, which are fed to the robot.

## Compilation
```
make
# server only
make server
# robot only
make robot
```

## Usage
Server start
```
./bin/server <number_of_players> <port>
```
Server connection
```
telnet <ip> <port>
```
Robot start: TODO

