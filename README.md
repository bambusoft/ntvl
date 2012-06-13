NTVL - Nat Traversal VLAN / VPN
===============================

Nat Traversal VLAN / VPN, is a networking tool to create a public/private network on the Internet  bypassing firewalls.

You can find the latest version at
	http://ntvl.bambusoft.mx
	https://github.com/bambusoft/ntvl/downloads
	
NTVL can also use as a layer-two peer-to-peer virtual private network (VPN) which allows users to exploit features typical of P2P applications
at network instead of application level. This means that users can gain native IP visibility (e.g. two PCs belonging to the same
ntvl network can ping each other) and be reachable with the same network IP address regardless of the network where they currently belong.

In a nutshell, as OpenVPN moved SSL from application (e.g. used to implement the https protocol) to network protocol, NTVL moves P2P from
application to network level.

The main ntvl design features are:

- An ntvl is an encrypted layer two private network based on a P2P protocol.
- Encryption is performed on nodes using open protocols with user-defined encryption keys:
  you control your security without delegating it to companies as it happens with Skype or Hamachi.
- Each ntvl user can simultaneously belong to multiple networks (a.k.a. communities).
- Ability to cross NAT and firewalls in the reverse traffic direction (i.e. from outside to inside) so that ntvl nodes are reachable
  even if running on a private network. Firewalls no longer are an obstacle to direct communications at IP level.
- NTVL networks are not meant to be self-contained, but it is possible to route traffic across ntvl and non-ntvl networks.

The natvl architecture is based on three components:

nodes:
	Applications installed on user PCs that allow the natvl network to be build.
	Practically each node creates a tun/tap device that is then the entry point to the ntvl network.
	
supernode:
	It is used by nodes at startup or for reaching nodes behind symmetrical firewalls.
	This application is basically a directory register and a packet router for those nodes that cannot talk directly.
	
tunnel:
	Complementary tool to make a point-to-point tunnel

Nodes talk by means of virtual tap interfaces. Each tap interface is an natvl node.
Each PC can have multiple tap interfaces, one per ntvl network, so that the same PC can belong to multiple communities

This work is based on the following projects:
	nat-traverse at http://m19s28.dyndns.org/iblech/nat-traverse/
	chownat at http://samy.pl/chownat/
	pwnat at http://samy.pl/pwnat/
	udp-tunnel at http://code.google.com/p/udptunnel/
	n2n Layer Two Peer to Peer VPN project at http://www.ntop.org/products/n2n/

Thanks to Ingo Blechschmidt, Samy Kamkar, Daniel Meekins, Luca Deri, Richard Andrews and many others for their initial work.
