# ft_irc

ft_irc is a project meant to teach about network programming, socket communication and protocol implementation. This involves creating and IRC (Internet Relay Chat) server that can handle multiple clients, manage channels and work as stated in IRC protocol.

This is is an implementation of IRC server developed as a group project.

---

**Features**
- Multi-client support
- Channel creation and management
- Handling server-client and client-client communication
- Support for following commands:
  - Invite (Invite user to a channel)
  - Join (Join a channel and create one if not existing)
  - Kick (Remove a user from a channel)
  - Mode (Change channel or user modes)
  - Nick (Change your nickname)
  - Part (Leave a channel)
  - Pass (Password for connection)
  - Privmsg (Send message to another user or a channel. When used in irssi use only /msg)
  - Quit (Disconnect from the server)
  - Topic (Set topic of a channel)
  - User (Register user to the server)

---

**Usage**

Build the project with: **make**

Run the server: **./ircserv [port] [password]**

In another terminal connect to the server, for example using irssi: **irssi -c localhost -p [port] -w [password]**

---

