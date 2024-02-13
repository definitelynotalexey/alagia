# Alagia - A TCP client/server framework
! Keep in mind this was done in an hour or two as a fun project, I'm aware of bad practices here and will be fixing them when I have some time.

### Why
Saw way too many "protection libraries" and "protection services" appear in the last weeks, all of them just blindly paste keyauth sample and antidebugs into "their" loaders,
while completely ignoring the importance of protecting the clients' product itself. 

This should work as a base for someone looking to create a server<->client communication. 

### Goal
The goal for this is to provide cheat developers a way to load their cheats somewhat securely.
My goal is **NOT** to get this to a p2c level but rather help devs who share cheats with their friends that eventually get leaked for whatever reason. Fuck da pay-to-paste sellers.

Features worth mentioning?:
- Timestamps on packets to prevent replay attacks.
- The server prevents auth-skipping.
- Server handles sessions.
- A way of sending and receiving packets, just tell the handler what you're sending/expecting, and all will be good. (shared/Packet.h)

To do:
- Proper encryption
- Product loading
- Heartbeat
- Blacklist
- File streaming

Thanks to my brother 178317892745891 for giving me the chance to post this when I lost the source and for numerous life saving advice.
