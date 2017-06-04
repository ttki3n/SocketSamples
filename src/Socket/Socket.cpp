#include "Socket.h"
#include "SimpleFilesLogger.h"




#define SCASE(x,result) case x: result = #x;break;
//This will print all information of addrinfo
void addrinfo_printtoString(const addrinfo& af_input)
{
	std::string _flags, _family, _socktype, _protocol, ai_canonname;
	/*
	{
		int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
		int                 ai_family;      // PF_xxx
		int                 ai_socktype;    // SOCK_xxx
		int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
		size_t              ai_addrlen;     // Length of ai_addr
		char *              ai_canonname;   // Canonical name for nodename
		_Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
		struct addrinfo *   ai_next;        // Next structure in linked list
	}
	*/

	/*
	#define AI_PASSIVE                  0x00000001  // Socket address will be used in bind() call
	#define AI_CANONNAME                0x00000002  // Return canonical name in first ai_canonname
	#define AI_NUMERICHOST              0x00000004  // Nodename must be a numeric address string
	#define AI_NUMERICSERV              0x00000008  // Servicename must be a numeric port number

	#define AI_ALL                      0x00000100  // Query both IP6 and IP4 with AI_V4MAPPED
	#define AI_ADDRCONFIG               0x00000400  // Resolution only if global address configured
	#define AI_V4MAPPED                 0x00000800  // On v6 failure, query v4 and convert to V4MAPPED format

	#define AI_NON_AUTHORITATIVE        0x00004000  // LUP_NON_AUTHORITATIVE
	#define AI_SECURE                   0x00008000  // LUP_SECURE
	#define AI_RETURN_PREFERRED_NAMES   0x00010000  // LUP_RETURN_PREFERRED_NAMES

	#define AI_FQDN                     0x00020000  // Return the FQDN in ai_canonname
	#define AI_FILESERVER               0x00040000  // Resolving fileserver name resolution
	#define AI_DISABLE_IDN_ENCODING     0x00080000  // Disable Internationalized Domain Names handling
	#define AI_EXTENDED                 0x80000000      // Indicates this is extended ADDRINFOEX(2/..) struct

	*/
	switch (af_input.ai_flags)
	{

		SCASE(AI_PASSIVE, _flags)
			SCASE(AI_CANONNAME, _flags)
			SCASE(AI_NUMERICHOST, _flags)


	default:
		_flags = "ai_flags has no match";
		break;
	}
	/*
	#define AF_UNSPEC       0               // unspecified
	#define AF_UNIX         1               // local to host (pipes, portals)
	#define AF_INET         2               // internetwork: UDP, TCP, etc.
	#define AF_IMPLINK      3               // arpanet imp addresses
	#define AF_PUP          4               // pup protocols: e.g. BSP
	#define AF_CHAOS        5               // mit CHAOS protocols
	#define AF_NS           6               // XEROX NS protocols
	#define AF_IPX          AF_NS           // IPX protocols: IPX, SPX, etc.
	#define AF_ISO          7               // ISO protocols
	#define AF_OSI          AF_ISO          // OSI is ISO
	#define AF_ECMA         8               // european computer manufacturers
	#define AF_DATAKIT      9               // datakit protocols
	#define AF_CCITT        10              // CCITT protocols, X.25 etc
	#define AF_SNA          11              // IBM SNA
	#define AF_DECnet       12              // DECnet
	#define AF_DLI          13              // Direct data link interface
	#define AF_LAT          14              // LAT
	#define AF_HYLINK       15              // NSC Hyperchannel
	#define AF_APPLETALK    16              // AppleTalk
	#define AF_NETBIOS      17              // NetBios-style addresses
	#define AF_VOICEVIEW    18              // VoiceView
	#define AF_FIREFOX      19              // Protocols from Firefox
	#define AF_UNKNOWN1     20              // Somebody is using this!
	#define AF_BAN          21              // Banyan
	#define AF_ATM          22              // Native ATM Services
	#define AF_INET6        23              // Internetwork Version 6
	#define AF_CLUSTER      24              // Microsoft Wolfpack
	#define AF_12844        25              // IEEE 1284.4 WG AF
	#define AF_IRDA         26              // IrDA
	#define AF_NETDES       28              // Network Designers OSI & gateway

	*/

	switch (af_input.ai_family)
	{
		SCASE(AF_UNSPEC, _family)
			SCASE(AF_UNIX, _family)
			SCASE(AF_INET, _family)
	default:
		_family = "ai_family has no match";
		break;
	}

	/*
	#define SOCK_STREAM     1               // stream socket
	#define SOCK_DGRAM      2               // datagram socket
	#define SOCK_RAW        3               // raw-protocol interface
	#define SOCK_RDM        4               // reliably-delivered message
	#define SOCK_SEQPACKET  5               // sequenced packet stream
	*/

	switch (af_input.ai_socktype)
	{
		SCASE(SOCK_STREAM, _socktype)
			SCASE(SOCK_DGRAM, _socktype)
			SCASE(SOCK_RAW, _socktype)
			SCASE(SOCK_RDM, _socktype)
			SCASE(SOCK_SEQPACKET, _socktype)
	default:
		_socktype = "ai_socktye has no match";
	}

	switch (af_input.ai_protocol)
	{
		SCASE(IPPROTO_HOPOPTS, _protocol)
			SCASE(IPPROTO_TCP, _protocol)
			SCASE(IPPROTO_UDP, _protocol)
			SCASE(IPPROTO_IPV6, _protocol)
			SCASE(IPPROTO_IPV4, _protocol)
	default:
		_protocol = "ai_protocol has no match";
	}

	char ipstr[INET6_ADDRSTRLEN];

	// convert the IP to a string :
	inet_ntop(af_input.ai_family, get_in_addr(af_input.ai_addr), ipstr, sizeof(ipstr));



	LOGGER_INFO("addrinfo_printtoString :\n ai_flags = %s\n ai_family = %s\n ai_socktype = %s\n ai_protocol = %s \
				\n ai_canonname = %s\n IP address = %s\n",
		_flags.c_str(),
		_family.c_str(),
		_socktype.c_str(),
		_protocol.c_str(),
		af_input.ai_canonname,
		ipstr);
}


void sockaddr_printtoString(const sockaddr& sa_input)
{
	/*
	typedef struct sockaddr {

#if (_WIN32_WINNT < 0x0600)
		u_short sa_family;
#else
		ADDRESS_FAMILY sa_family;           // Address family.
#endif //(_WIN32_WINNT < 0x0600)

		CHAR sa_data[14];                   // Up to 14 bytes of direct address.
	} SOCKADD
	*/
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}