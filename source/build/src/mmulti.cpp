// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#include "baselayer.h"
#include "compat.h"
#include "mmulti.h"

int myconnectindex, numplayers;
int connectpoint2[MAXPLAYERS];

#ifndef EDUKE32_STANDALONE

#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_IPV6
//#define MMULTI_DEBUG_SENDRECV
//#define MMULTI_DEBUG_SENDRECV_WIRE

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#define IPV6_RECVPKTINFO IPV6_PKTINFO
#define IS_INVALID_SOCKET(sock) (sock == INVALID_SOCKET)

#ifndef CMSG_FIRSTHDR
#define CMSG_FIRSTHDR(m) WSA_CMSG_FIRSTHDR(m)
#endif
#ifndef CMSG_NXTHDR
#define CMSG_NXTHDR(m,c) WSA_CMSG_NXTHDR(m,c)
#endif
#ifndef CMSG_LEN
#define CMSG_LEN(l) WSA_CMSG_LEN(l)
#endif
#ifndef CMSG_SPACE
#define CMSG_SPACE(l) WSA_CMSG_SPACE(l)
#endif
#define CMSG_DATA(c) WSA_CMSG_DATA(c)

LPFN_WSASENDMSG WSASendMsgPtr;
LPFN_WSARECVMSG WSARecvMsgPtr;

#else

#ifdef __APPLE__
# define __APPLE_USE_RFC_3542
#endif

#ifdef __GNUC__
#ifndef __USE_GNU
# define __USE_GNU
#endif
#endif

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#define SOCKET int

#include <sys/time.h>
static int GetTickCount(void)
{
	struct timeval tv;
	int ti;
	if (gettimeofday(&tv,NULL) < 0) return 0;
	// tv is sec.usec, GTC gives msec
	ti = tv.tv_sec * 1000;
	ti += tv.tv_usec / 1000;
	return ti;
}

#define IS_INVALID_SOCKET(sock) (sock < 0)

#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define MAXPLAYERS 16
#define MAXPAKSIZ 256 //576

#define PAKRATE 250  //Packet rate/sec limit ... necessary?
#define SIMMIS 0     //Release:0  Test:100 Packets per 256 missed.
#define SIMLAG 0     //Release:0  Test: 10 Packets to delay receipt
#define PRESENCETIMEOUT 2000
#if (SIMLAG > 1)
static int simlagcnt[MAXPLAYERS];
static unsigned char simlagfif[MAXPLAYERS][SIMLAG+1][MAXPAKSIZ+2];
#endif
#if ((SIMMIS != 0) || (SIMLAG != 0))
#pragma message("\n\nWARNING! INTENTIONAL PACKET LOSS SIMULATION IS ENABLED!\nREMEMBER TO CHANGE SIMMIS&SIMLAG to 0 before RELEASE!\n\n")
#endif

int networkmode = -1;
int connecthead;

static int tims, lastsendtims[MAXPLAYERS], lastrecvtims[MAXPLAYERS], prevlastrecvtims[MAXPLAYERS];
static unsigned char pakbuf[MAXPAKSIZ], playerslive[MAXPLAYERS];

#define FIFSIZ 512 //16384/40 = 6min:49sec
static int ipak[MAXPLAYERS][FIFSIZ], icnt0[MAXPLAYERS];
static int opak[MAXPLAYERS][FIFSIZ], ocnt0[MAXPLAYERS], ocnt1[MAXPLAYERS];
static unsigned char pakmem[4194304]; static int pakmemi = 1;

#define NETPORT 0x5bd9
static SOCKET mysock = -1;
static int domain = PF_UNSPEC;
#ifdef USE_IPV6
static int forceipv4 = 0;
#endif
static struct sockaddr_storage otherhost[MAXPLAYERS], snatchhost;	// IPV4/6 address of peers
static struct in_addr replyfrom4[MAXPLAYERS], snatchreplyfrom4;		// our IPV4 address peers expect to hear from us on
static struct in6_addr replyfrom6[MAXPLAYERS], snatchreplyfrom6;	// our IPV6 address peers expect to hear from us on
static int netready = 0;

static int lookuphost(const char *name, struct sockaddr *host, int warnifmany);
static int issameaddress(struct sockaddr *a, struct sockaddr *b);
static const char *presentaddress(struct sockaddr *a);
static void savesnatchhost(int other);

void netuninit ()
{
#ifdef _WIN32
	if (mysock != INVALID_SOCKET) closesocket(mysock);
	WSACleanup();
	mysock = INVALID_SOCKET;

	WSASendMsgPtr = NULL;
	WSARecvMsgPtr = NULL;
#else
	if (mysock >= 0) close(mysock);
	mysock = -1;
#endif
	domain = PF_UNSPEC;
}

int netinit (int portnum)
{
#ifdef _WIN32
	WSADATA ws;
	u_long off = 0, on = 1;
#else
	unsigned int off = 0, on = 1;
#endif

#ifdef _WIN32
	if (WSAStartup(0x202, &ws) != 0) return(0);
#endif

#ifdef USE_IPV6
	domain = forceipv4 ? PF_INET : PF_INET6;
#else
	domain = PF_INET;
#endif

	while (domain != PF_UNSPEC) {
		// Tidy up from last cycle.
#ifdef _WIN32
		if (mysock != INVALID_SOCKET) closesocket(mysock);
		WSASendMsgPtr = NULL;
		WSARecvMsgPtr = NULL;
#else
		if (mysock >= 0) close(mysock);
#endif

		mysock = socket(domain, SOCK_DGRAM, 0);
		if (IS_INVALID_SOCKET(mysock)) {
			if (domain == PF_INET6) {
				// Retry for IPV4.
				initprintf("mmulti warning: could not create IPV6 socket, trying for IPV4.\n");
				domain = PF_INET;
				continue;
			} else {
				// No IPV4 is a total loss.
				initprintf("mmulti error: could not create IPV4 socket, no multiplayer possible.\n");
				break;
			}
		}

#ifdef _WIN32
		DWORD len;
		GUID sendguid = WSAID_WSASENDMSG, recvguid = WSAID_WSARECVMSG;
		if (WSAIoctl(mysock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&sendguid, sizeof(sendguid), &WSASendMsgPtr, sizeof(WSASendMsgPtr),
				&len, NULL, NULL) == SOCKET_ERROR) {
			initprintf("mmulti error: could not get sendmsg entry point.\n");
			break;
		}
		if (WSAIoctl(mysock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&recvguid, sizeof(recvguid), &WSARecvMsgPtr, sizeof(WSARecvMsgPtr),
				&len, NULL, NULL) == SOCKET_ERROR) {
			initprintf("mmulti error: could not get recvmsg entry point.\n");
			break;
		}
#endif

		// Set non-blocking IO on the socket.
#ifdef _WIN32
		if (ioctlsocket(mysock, FIONBIO, &on) != 0)
#else
		if (ioctl(mysock, FIONBIO, &on) != 0)
#endif
		{
			initprintf("mmulti error: could not enable non-blocking IO on socket.\n");
			break;
		}

		// Allow local address reuse.
		if (setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) != 0) {
			initprintf("mmulti error: could not enable local address reuse on socket.\n");
			break;
		}

		// Request that we receive IPV4 packet info.
#if defined(__linux) || defined(_WIN32)
		if (setsockopt(mysock, IPPROTO_IP, IP_PKTINFO, (const char *)&on, sizeof(on)) != 0)
#else
		if (domain == PF_INET && setsockopt(mysock, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on)) != 0)
#endif
		{
			if (domain == PF_INET) {
				initprintf("mmulti error: could not enable IPV4 packet info on socket.\n");
				break;
			} else {
				initprintf("mmulti warning: could not enable IPV4 packet info on socket.\n");
			}
		}

		if (domain == PF_INET6) {
			// Allow dual-stack IPV4/IPV6 on the socket.
			if (setsockopt(mysock, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&off, sizeof(off)) != 0) {
				initprintf("mmulti warning: could not enable dual-stack socket, retrying for IPV4.\n");
				domain = PF_INET;
				continue;
			}

			// Request that we receive IPV6 packet info
			if (setsockopt(mysock, IPPROTO_IPV6, IPV6_RECVPKTINFO, (const char *)&on, sizeof(on)) != 0) {
				initprintf("mmulti error: could not enable IPV6 packet info on socket.\n");
				break;
			}

			struct sockaddr_in6 host;
			memset(&host, 0, sizeof(host));
			host.sin6_family = AF_INET6;
			host.sin6_port = htons(portnum);
			host.sin6_addr = in6addr_any;
			if (bind(mysock, (struct sockaddr *)&host, sizeof(host)) != 0) {
				// Retry for IPV4.
				domain = PF_INET;
				continue;
			}
		} else {
			struct sockaddr_in host;
			memset(&host, 0, sizeof(host));
			host.sin_family = AF_INET;
			host.sin_port = htons(portnum);
			host.sin_addr.s_addr = INADDR_ANY;
			if (bind(mysock, (struct sockaddr *)&host, sizeof(host)) != 0) {
				// No IPV4 is a total loss.
				break;
			}
		}

		// Complete success.
		return 1;
	}

	netuninit();

	return 0;
}

int netsend (int other, void *dabuf, int bufsiz) //0:buffer full... can't send
{
	char msg_control[1024];
	if (otherhost[other].ss_family == AF_UNSPEC) return(0);

	if (otherhost[other].ss_family != domain) {
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
		debugprintf("mmulti debug send error: tried sending to a different protocol family\n");
#endif
		return 0;
	}

#ifdef MMULTI_DEBUG_SENDRECV_WIRE
	{
		int i;
		const unsigned char *pakbuf = dabuf;
		debugprintf("mmulti debug send: "); for(i=0;i<bufsiz;i++) debugprintf("%02x ",pakbuf[i]); debugprintf("\n");
	}
#endif

#ifdef _WIN32
	WSABUF iovec;
	WSAMSG msg;
	WSACMSGHDR *cmsg;
	DWORD len = 0;

	iovec.buf = (char *)dabuf;
	iovec.len = bufsiz;
	msg.name = (LPSOCKADDR)&otherhost[other];
	if (otherhost[other].ss_family == AF_INET) {
		msg.namelen = sizeof(struct sockaddr_in);
	} else {
		msg.namelen = sizeof(struct sockaddr_in6);
	}
	msg.lpBuffers = &iovec;
	msg.dwBufferCount = 1;
	msg.Control.buf = msg_control;
	msg.Control.len = sizeof(msg_control);
	msg.dwFlags = 0;
#else
	struct iovec iovec;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	int len;

	iovec.iov_base = dabuf;
	iovec.iov_len = bufsiz;
	msg.msg_name = &otherhost[other];
	if (otherhost[other].ss_family == AF_INET) {
		msg.msg_namelen = sizeof(struct sockaddr_in);
	} else {
		msg.msg_namelen = sizeof(struct sockaddr_in6);
	}
	msg.msg_iov = &iovec;
	msg.msg_iovlen = 1;
	msg.msg_control = msg_control;
	msg.msg_controllen = sizeof(msg_control);
	msg.msg_flags = 0;
#endif

	len = 0;
	memset(msg_control, 0, sizeof(msg_control));

	cmsg = CMSG_FIRSTHDR(&msg);
#ifndef __APPLE__
	// OS X doesn't implement setting the UDP4 source. We'll
	// just have to cross our fingers.
	if (replyfrom4[other].s_addr != INADDR_ANY) {
		cmsg->cmsg_level = IPPROTO_IP;
#if defined(__linux) || defined(_WIN32)
		cmsg->cmsg_type = IP_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
		#ifdef _WIN32
		((struct in_pktinfo *)CMSG_DATA(cmsg))->ipi_addr = replyfrom4[other];
		#else
		((struct in_pktinfo *)CMSG_DATA(cmsg))->ipi_spec_dst = replyfrom4[other];
		#endif
		len += CMSG_SPACE(sizeof(struct in_pktinfo));
#else
		cmsg->cmsg_type = IP_SENDSRCADDR;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_addr));
		*(struct in_addr *)CMSG_DATA(cmsg) = replyfrom4[other];
		len += CMSG_SPACE(sizeof(struct in_addr));
#endif
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}
#endif
	if (!IN6_IS_ADDR_UNSPECIFIED(&replyfrom6[other])) {
		cmsg->cmsg_level = IPPROTO_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
		((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_addr = replyfrom6[other];
		len += CMSG_SPACE(sizeof(struct in6_pktinfo));
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}
#ifdef _WIN32
	msg.Control.len = len;
	if (len == 0) {
		msg.Control.buf = NULL;
	}
#else
	msg.msg_controllen = len;
	if (len == 0) {
		msg.msg_control = NULL;
	}
#endif

#ifdef _WIN32
	if (WSASendMsgPtr(mysock, &msg, 0, &len, NULL, NULL) == SOCKET_ERROR)
#else
	if ((len = sendmsg(mysock, &msg, 0)) < 0)
#endif
	{
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
		debugprintf("mmulti debug send error: %s\n", strerror(errno));
#endif
		return 0;
	}

	return 1;
}

int netread (int *other, void *dabuf, int bufsiz) //0:no packets in buffer
{
	char msg_control[1024];
	int i;

#ifdef _WIN32
	WSABUF iovec;
	WSAMSG msg;
	WSACMSGHDR *cmsg;
	DWORD len = 0;

	iovec.buf = (char *)dabuf;
	iovec.len = bufsiz;
	msg.name = (LPSOCKADDR)&snatchhost;
	msg.namelen = sizeof(snatchhost);
	msg.lpBuffers = &iovec;
	msg.dwBufferCount = 1;
	msg.Control.buf = msg_control;
	msg.Control.len = sizeof(msg_control);
	msg.dwFlags = 0;

	if (WSARecvMsgPtr(mysock, &msg, &len, NULL, NULL) == SOCKET_ERROR) return 0;
#else
	struct iovec iovec;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	int len;

	iovec.iov_base = dabuf;
	iovec.iov_len = bufsiz;
	msg.msg_name = &snatchhost;
	msg.msg_namelen = sizeof(snatchhost);
	msg.msg_iov = &iovec;
	msg.msg_iovlen = 1;
	msg.msg_control = msg_control;
	msg.msg_controllen = sizeof(msg_control);
	msg.msg_flags = 0;

	if ((len = recvmsg(mysock, &msg, 0)) < 0) return 0;
#endif
	if (len == 0) return 0;

	if (snatchhost.ss_family != domain) {
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
		debugprintf("mmulti debug recv error: received from a different protocol family\n");
#endif
		return 0;
	}

#if (SIMMIS > 0)
	if ((rand()&255) < SIMMIS) return(0);
#endif

	// Decode the message headers to record what of our IP addresses the
	// packet came in on. We reply on that same address so the peer knows
	// who it came from.
	memset(&snatchreplyfrom4, 0, sizeof(snatchreplyfrom4));
	memset(&snatchreplyfrom6, 0, sizeof(snatchreplyfrom6));
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
#if defined(__linux) || defined(_WIN32)
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
			snatchreplyfrom4 = ((struct in_pktinfo *)CMSG_DATA(cmsg))->ipi_addr;
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
			debugprintf("mmulti debug recv: received at %s\n", inet_ntoa(snatchreplyfrom4));
#endif
		}
#else
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVDSTADDR) {
			snatchreplyfrom4 = *(struct in_addr *)CMSG_DATA(cmsg);
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
			debugprintf("mmulti debug recv: received at %s\n", inet_ntoa(snatchreplyfrom4));
#endif
		}
#endif
		else if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO) {
			snatchreplyfrom6 = ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_addr;
#ifdef MMULTI_DEBUG_SENDRECV_WIRE
			char addr[INET6_ADDRSTRLEN+1];
			debugprintf("mmulti debug recv: received at %s\n", inet_ntop(AF_INET6, &snatchreplyfrom6, addr, sizeof(addr)));
#endif
		}
	}

#ifdef MMULTI_DEBUG_SENDRECV_WIRE
	{
		const unsigned char *pakbuf = dabuf;
		debugprintf("mmulti debug recv: ");
		for(i=0;i<len;i++) debugprintf("%02x ",pakbuf[i]);
		debugprintf("\n");
	}
#endif

	(*other) = myconnectindex;
	for(i=0;i<MAXPLAYERS;i++) {
		if (issameaddress((struct sockaddr *)&snatchhost, (struct sockaddr *)&otherhost[i]))
			{ (*other) = i; break; }
	}

#if (SIMLAG > 1)
	i = simlagcnt[*other]%(SIMLAG+1);
	*(short *)&simlagfif[*other][i][0] = bufsiz; memcpy(&simlagfif[*other][i][2],dabuf,bufsiz);
	simlagcnt[*other]++; if (simlagcnt[*other] < SIMLAG+1) return(0);
	i = simlagcnt[*other]%(SIMLAG+1);
	bufsiz = *(short *)&simlagfif[*other][i][0]; memcpy(dabuf,&simlagfif[*other][i][2],bufsiz);
#endif

	return(1);
}

static int issameaddress(struct sockaddr *a, struct sockaddr *b) {
	if (a->sa_family != b->sa_family) {
		// Different families.
		return 0;
	}
	if (a->sa_family == AF_INET) {
		// IPV4.
		struct sockaddr_in *a4 = (struct sockaddr_in *)a;
		struct sockaddr_in *b4 = (struct sockaddr_in *)b;
		return a4->sin_addr.s_addr == b4->sin_addr.s_addr &&
			a4->sin_port == b4->sin_port;
	}
	if (a->sa_family == AF_INET6) {
		// IPV6.
		struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)a;
		struct sockaddr_in6 *b6 = (struct sockaddr_in6 *)b;
		return IN6_ARE_ADDR_EQUAL(&a6->sin6_addr, &b6->sin6_addr) &&
			a6->sin6_port == b6->sin6_port;
	}
	return 0;
}

static const char *presentaddress(struct sockaddr *a) {
	static char str[128+32];
	char addr[128];
	int port;

	if (a->sa_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)a;
		inet_ntop(AF_INET, &s->sin_addr, addr, sizeof(addr));
		port = ntohs(s->sin_port);
	} else if (a->sa_family == AF_INET6) {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)a;
		strcpy(addr, "[");
		inet_ntop(AF_INET6, &s->sin6_addr, addr+1, sizeof(addr)-2);
		strcat(addr, "]");
		port = ntohs(s->sin6_port);
	} else {
		return NULL;
	}

	strcpy(str, addr);
	sprintf(addr, ":%d", port);
	strcat(str, addr);

	return str;
}

//---------------------------------- Obsolete variables&functions ----------------------------------
unsigned char syncstate = 0;
void setpackettimeout (int UNUSED(datimeoutcount), int UNUSED(daresendagaincount)) {}
void genericmultifunction (int UNUSED(other), unsigned char *UNUSED(bufptr), int UNUSED(messleng), int UNUSED(command)) {}
int getoutputcirclesize () { return(0); }
void setsocket (int UNUSED(newsocket)) { }
void flushpackets () {}
void sendlogon () {}
void sendlogoff () {}
//--------------------------------------------------------------------------------------------------

static int crctab16[256];
static void initcrc16 ()
{
	int i, j, k, a;
	for(j=0;j<256;j++)
	{
		for(i=7,k=(j<<8),a=0;i>=0;i--,k=((k<<1)&65535))
		{
			if ((k^a)&0x8000) a = ((a<<1)&65535)^0x1021;
							 else a = ((a<<1)&65535);
		}
		crctab16[j] = (a&65535);
	}
}
#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctab16[((((unsigned short)crc)>>8)&65535)^dat])
static unsigned short getcrc16 (unsigned char *buffer, int bufleng)
{
	int i, j;

	j = 0;
	for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
	return((unsigned short)(j&65535));
}

void uninitmultiplayers () { netuninit(); }

static void initmultiplayers_reset(void)
{
	int i;

	initcrc16();
	memset(icnt0,0,sizeof(icnt0));
	memset(ocnt0,0,sizeof(ocnt0));
	memset(ocnt1,0,sizeof(ocnt1));
	memset(ipak,0,sizeof(ipak));
	//memset(opak,0,sizeof(opak)); //Don't need to init opak
	//memset(pakmem,0,sizeof(pakmem)); //Don't need to init pakmem
#if (SIMLAG > 1)
	memset(simlagcnt,0,sizeof(simlagcnt));
#endif

	lastsendtims[0] = GetTickCount();
	for(i=0;i<MAXPLAYERS;i++) {
		lastsendtims[i] = lastsendtims[0];
		lastrecvtims[i] = prevlastrecvtims[i] = 0;
		connectpoint2[i] = -1;
		playerslive[i] = 0;
	}
	connecthead = 0;
	numplayers = 1; myconnectindex = 0;

	memset(otherhost,0,sizeof(otherhost));
}

void initsingleplayers(void)
{
    initmultiplayers_reset();
}

	// Multiplayer command line summary. Assume myconnectindex always = 0 for 192.168.1.2
	//
	// /n0 (mast/slav) 2 player:               3 player:
	// 192.168.1.2     game /n0                game /n0:3
	// 192.168.1.100   game /n0 192.168.1.2    game /n0 192.168.1.2
	// 192.168.1.4                             game /n0 192.168.1.2
	//
	// /n1 (peer-peer) 2 player:               3 player:
	// 192.168.1.2     game /n1 * 192.168.1.100  game /n1 * 192.168.1.100 192.168.1.4
	// 192.168.1.100   game /n2 192.168.1.2 *    game /n1 192.168.1.2 * 192.168.1.4
	// 192.168.1.4                               game /n1 192.168.1.2 192.168.1.100 *
	// Note: '.' may also be used in place of '*'
int initmultiplayersparms(int argc, char const * const argv[])
{
	int i, j, daindex, danumplayers, danetmode, portnum = NETPORT;
	struct sockaddr_storage resolvhost;

	initmultiplayers_reset();
	danetmode = 255; daindex = 0; danumplayers = 0;

	for (i=0;i<argc;i++) {
		if (argv[i][0] != '-' && argv[i][0] != '/') continue;

		// -p1234 = Listen port
		if ((argv[i][1] == 'p' || argv[i][1] == 'P') && argv[i][2]) {
			char *p;
			j = strtol(argv[i]+2, &p, 10);
			if (!(*p) && j > 0 && j<65535) portnum = j;

			initprintf("mmulti: Using port %d\n", portnum);
			continue;
		}

		// -nm,   -n0   = Master/slave, 2 players
		// -nm:n, -n0:n = Master/slave, n players
		// -np,   -n1   = Peer-to-peer
		if ((argv[i][1] == 'N') || (argv[i][1] == 'n') || (argv[i][1] == 'I') || (argv[i][1] == 'i'))
		{
			danumplayers = 2;
			if (argv[i][2] == '0' || argv[i][2] == 'm' || argv[i][2] == 'M')
			{
				danetmode = MMULTI_MODE_MS;
				initprintf("mmulti: Master-slave mode\n");

				if ((argv[i][3] == ':') && (argv[i][4] >= '0') && (argv[i][4] <= '9'))
				{
					char *p;
					j = strtol(argv[i]+4, &p, 10);
					if (!(*p) && j > 0 && j <= MAXPLAYERS) {
						danumplayers = j;
						initprintf("mmulti: %d-player game\n", danumplayers);
					} else {
						initprintf("mmulti error: Invalid number of players\n");
						return 0;
					}
				}
			}
			else if (argv[i][2] == '1' || argv[i][2] == 'p' || argv[i][2] == 'P')
			{
				danetmode = MMULTI_MODE_P2P;
				initprintf("mmulti: Peer-to-peer mode\n");
			}
			continue;
		}

#ifdef USE_IPV6
		// -forceipv4 == Force IPv4
		if (!Bstrcasecmp(argv[i]+1, "forceipv4"))
		{
			forceipv4 = 1;
			continue;
		}
#endif
	}

	if (!netinit(portnum)) {
		initprintf("mmulti error: Could not initialise networking\n");
		return 0;
	}

	for(i=0;i<argc;i++)
	{
		if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
			continue;
		}

		if (danetmode == MMULTI_MODE_MS && daindex >= 1) {
			initprintf("mmulti warning: Too many host names given for master-slave mode\n");
			continue;
		}
		if (daindex >= MAXPLAYERS) {
			initprintf("mmulti error: More than %d players provided\n", MAXPLAYERS);
			netuninit();
			return 0;
		}

		if ((argv[i][0] == '*' || argv[i][0] == '.') && argv[i][1] == 0) {
			// 'self' placeholder
			if (danetmode == MMULTI_MODE_MS) {
				initprintf("mmulti: %c is not valid in master-slave mode\n", argv[i][0]);
				netuninit();
				return 0;
			} else {
				myconnectindex = daindex++;
				initprintf("mmulti: This machine is player %d\n", myconnectindex);
			}
			continue;
		}

		if (!lookuphost(argv[i], (struct sockaddr *)&resolvhost, danetmode == MMULTI_MODE_P2P)) {
			initprintf("mmulti error: Could not resolve %s\n", argv[i]);
			netuninit();
			return 0;
		} else {
			memcpy(&otherhost[daindex], &resolvhost, sizeof(resolvhost));
			initprintf("mmulti: Player %d at %s (%s)\n", daindex,
				presentaddress((struct sockaddr *)&resolvhost), argv[i]);
			daindex++;
		}
	}

	if ((danetmode == 255) && (daindex)) { danumplayers = 2; danetmode = MMULTI_MODE_MS; } //an IP w/o /n# defaults to /n0
	if ((danumplayers >= 2) && (daindex) && (danetmode == MMULTI_MODE_MS)) myconnectindex = 1;
	if (daindex > danumplayers) danumplayers = daindex;
	else if (danumplayers == 0) danumplayers = 1;

	if (danetmode == MMULTI_MODE_MS && myconnectindex == 0) {
		initprintf("mmulti: This machine is master\n");
	}

	networkmode = danetmode;
	numplayers = danumplayers;

	connecthead = 0;
	for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
	connectpoint2[numplayers-1] = -1;

	netready = 0;

	if (numplayers >= 2) {
		return 1;
	} else {
		netuninit();
		return 0;
	}
}

int initmultiplayerscycle(void)
{
	int i, k, dnetready = 1;

	getpacket(&i,0);

	tims = GetTickCount();

	if (networkmode == MMULTI_MODE_MS && myconnectindex == connecthead)
	{
		// The master waits for all players to check in.
		for(i=numplayers-1;i>0;i--) {
			if (i == myconnectindex) continue;
			if (otherhost[i].ss_family == AF_UNSPEC) {
				// There's a slot to be filled.
				dnetready = 0;
			} else if (lastrecvtims[i] == 0) {
				// There's a player who hasn't checked in.
				dnetready = 0;
			} else if (prevlastrecvtims[i] != lastrecvtims[i]) {
				if (!playerslive[i]) {
					initprintf("mmulti: Player %d is here\n", i);
					playerslive[i] = 1;
				}
				prevlastrecvtims[i] = lastrecvtims[i];
			} else if (tims - lastrecvtims[i] > PRESENCETIMEOUT) {
				if (playerslive[i]) {
					initprintf("mmulti: Player %d has gone\n", i);
					playerslive[i] = 0;
				}
				prevlastrecvtims[i] = lastrecvtims[i];
				dnetready = 0;
			}
		}
	}
	else
	{
		if (networkmode == MMULTI_MODE_MS) {
			// As a slave, we send the master pings. The netready flag gets
			// set by getpacket() and is sent by the master when it is OK
			// to launch.
			dnetready = 0;
			i = connecthead;

		} else {
			// In peer-to-peer mode we send pings to our peer group members
			// and wait for them all to check in. The netready flag is set
			// when everyone responds together within the timeout.
			i = numplayers - 1;
		}
		while (1) {
			if (i != myconnectindex) {
				if (tims < lastsendtims[i]) lastsendtims[i] = tims;
				if (tims >= lastsendtims[i]+250) //1000/PAKRATE)
				{
#ifdef MMULTI_DEBUG_SENDRECV
					debugprintf("mmulti debug: sending player %d a ping\n", i);
#endif

					lastsendtims[i] = tims;

						//   short crc16ofs;       //offset of crc16
						//   int icnt0;           //-1 (special packet for MMULTI.C's player collection)
						//   ...
						//   unsigned short crc16; //CRC16 of everything except crc16
					k = 2;
					*(int *)&pakbuf[k] = -1; k += 4;
					pakbuf[k++] = 0xaa;
					*(unsigned short *)&pakbuf[0] = (unsigned short)k;
					*(unsigned short *)&pakbuf[k] = getcrc16(pakbuf,k); k += 2;
					netsend(i,pakbuf,k);
				}

				if (lastrecvtims[i] == 0) {
					// There's a player who hasn't checked in.
					dnetready = 0;
				} else if (prevlastrecvtims[i] != lastrecvtims[i]) {
					if (!playerslive[i]) {
						initprintf("mmulti: Player %d is here\n", i);
						playerslive[i] = 1;
					}
					prevlastrecvtims[i] = lastrecvtims[i];
				} else if (prevlastrecvtims[i] - tims > PRESENCETIMEOUT) {
					if (playerslive[i]) {
						initprintf("mmulti: Player %d has gone\n", i);
						playerslive[i] = 0;
					}
					prevlastrecvtims[i] = lastrecvtims[i];
					dnetready = 0;
				}
			}

			if (networkmode == MMULTI_MODE_MS) {
				break;
			} else {
				if (--i < 0) break;
			}
		}
	}

	netready = netready || dnetready;

	return !netready;
}

void initmultiplayers (int argc, char const * const argv[])
{
	if (initmultiplayersparms(argc,argv))
	{
		while (initmultiplayerscycle())
		{
		}
	}
}

static int lookuphost(const char *name, struct sockaddr *host, int warnifmany)
{
	struct addrinfo * result, *res;
	struct addrinfo hints;
	char *wname, *portch;
	int error, port = 0, found = 0;

	// ipv6 for future thought:
	//  [2001:db8::1]:1234

	wname = strdup(name);
	if (!wname) {
		return 0;
	}

	// Parse a port.
	portch = strrchr(wname, ':');
	if (portch) {
		*(portch++) = 0;
		if (*portch != 0) {
			port = strtol(portch, NULL, 10);
		}
	}
	if (port < 1025 || port > 65534) port = NETPORT;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_family = domain;
	if (domain == PF_INET6) {
		hints.ai_flags |= AI_V4MAPPED;
	}
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	error = getaddrinfo(wname, NULL, &hints, &result);
	if (error) {
		initprintf("mmulti error: problem resolving %s (%s)\n", name, gai_strerror(error));
		free(wname);
		return 0;
	}

	for (res = result; res; res = res->ai_next) {
		if (res->ai_family == PF_INET && !found) {
			memcpy((struct sockaddr_in *)host, (struct sockaddr_in *)res->ai_addr, sizeof(struct sockaddr_in));
			((struct sockaddr_in *)host)->sin_port = htons(port);
			found = 1;
		} else if (res->ai_family == PF_INET6 && !found) {
			memcpy((struct sockaddr_in6 *)host, (struct sockaddr_in6 *)res->ai_addr, sizeof(struct sockaddr_in6));
			((struct sockaddr_in6 *)host)->sin6_port = htons(port);
			found = 1;
		} else if (found && warnifmany) {
			initprintf("mmulti warning: host name %s has another address: %s\n", wname, presentaddress(res->ai_addr));
		}
	}

	freeaddrinfo(result);
	free(wname);
	return found;
}

void dosendpackets (int other)
{
	int i, j, k;

	if (otherhost[other].ss_family == AF_UNSPEC) return;

		//Packet format:
		//   short crc16ofs;       //offset of crc16
		//   int icnt0;           //earliest unacked packet
		//   char ibits[32];       //ack status of packets icnt0<=i<icnt0+256
		//   while (short leng)    //leng: !=0 for packet, 0 for no more packets
		//   {
		//      int ocnt;         //index of following packet data
		//      char pak[leng];    //actual packet data :)
		//   }
		//   unsigned short crc16; //CRC16 of everything except crc16


	tims = GetTickCount();
	if (tims < lastsendtims[other]) lastsendtims[other] = tims;
	if (tims < lastsendtims[other]+1000/PAKRATE) return;
	lastsendtims[other] = tims;

	k = 2;
	*(int *)&pakbuf[k] = icnt0[other]; k += 4;
	memset(&pakbuf[k],0,32);
	for(i=icnt0[other];i<icnt0[other]+256;i++)
		if (ipak[other][i&(FIFSIZ-1)])
			pakbuf[((i-icnt0[other])>>3)+k] |= (1<<((i-icnt0[other])&7));
	k += 32;

	while ((ocnt0[other] < ocnt1[other]) && (!opak[other][ocnt0[other]&(FIFSIZ-1)])) ocnt0[other]++;
	for(i=ocnt0[other];i<ocnt1[other];i++)
	{
		j = *(short *)&pakmem[opak[other][i&(FIFSIZ-1)]]; if (!j) continue; //packet already acked
		if (k+6+j+4 > (int)sizeof(pakbuf)) break;

		*(unsigned short *)&pakbuf[k] = (unsigned short)j; k += 2;
		*(int *)&pakbuf[k] = i; k += 4;
		memcpy(&pakbuf[k],&pakmem[opak[other][i&(FIFSIZ-1)]+2],j); k += j;
	}
	*(unsigned short *)&pakbuf[k] = 0; k += 2;
	*(unsigned short *)&pakbuf[0] = (unsigned short)k;
	*(unsigned short *)&pakbuf[k] = getcrc16(pakbuf,k); k += 2;
	netsend(other,pakbuf,k);
}

void sendpacket (int other, unsigned char *bufptr, int messleng)
{
	if (numplayers < 2) return;

	if (pakmemi+messleng+2 > (int)sizeof(pakmem)) pakmemi = 1;
	opak[other][ocnt1[other]&(FIFSIZ-1)] = pakmemi;
	*(short *)&pakmem[pakmemi] = messleng;
	memcpy(&pakmem[pakmemi+2],bufptr,messleng); pakmemi += messleng+2;
	ocnt1[other]++;

	dosendpackets(other);
}

	//passing bufptr == 0 enables receive&sending raw packets but does not return any received packets
	//(used as hack for player collection)
int getpacket (int *retother, unsigned char *bufptr)
{
	int i, j, k, ic0, crc16ofs, messleng, other;
	static int warned = 0;

	if (numplayers < 2) return(0);

	if (netready)
	{
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			if (i != myconnectindex) dosendpackets(i);
			if ((networkmode == MMULTI_MODE_MS) && (myconnectindex != connecthead)) break; //slaves in M/S mode only send to master
		}
	}

	tims = GetTickCount();

	while (netread(&other,pakbuf,sizeof(pakbuf)))
	{
			//Packet format:
			//   short crc16ofs;       //offset of crc16
			//   int icnt0;           //earliest unacked packet
			//   char ibits[32];       //ack status of packets icnt0<=i<icnt0+256
			//   while (short leng)    //leng: !=0 for packet, 0 for no more packets
			//   {
			//      int ocnt;         //index of following packet data
			//      char pak[leng];    //actual packet data :)
			//   }
			//   unsigned short crc16; //CRC16 of everything except crc16
		k = 0;
		crc16ofs = (int)(*(unsigned short *)&pakbuf[k]); k += 2;

		if (crc16ofs+2 > (int)sizeof(pakbuf)) {
#ifdef MMULTI_DEBUG_SENDRECV
			debugprintf("mmulti debug: wrong-sized packet from %d\n", other);
#endif
		} else if (getcrc16(pakbuf,crc16ofs) != (*(unsigned short *)&pakbuf[crc16ofs])) {
#ifdef MMULTI_DEBUG_SENDRECV
			debugprintf("mmulti debug: bad crc in packet from %d\n", other);
#endif
		} else {
			ic0 = *(int *)&pakbuf[k]; k += 4;
			if (ic0 == -1)
			{
				// Peers send each other 0xaa and respond with 0xab containing their opinion
				// of the requesting peer's placement within the order.

				// Slave sends 0xaa to Master at initmultiplayerscycle() and waits for 0xab response.
				// Master responds to slave with 0xab whenever it receives a 0xaa - even if during game!
				if (pakbuf[k] == 0xaa)
				{
					int sendother = -1;
#ifdef MMULTI_DEBUG_SENDRECV
					const char *addr = presentaddress((struct sockaddr *)&snatchhost);
#endif

					if (networkmode == MMULTI_MODE_MS) {
						// Master-slave.
						if (other == myconnectindex && myconnectindex == connecthead) {
							// This the master and someone new is calling. Find them a place.
#ifdef MMULTI_DEBUG_SENDRECV
							debugprintf("mmulti debug: got ping from new host %s\n", addr);
#endif
							for (other = 1; other < numplayers; other++) {
								if (otherhost[other].ss_family == PF_UNSPEC) {
									sendother = other;
#ifdef MMULTI_DEBUG_SENDRECV
									debugprintf("mmulti debug: giving %s player %d\n", addr, other);
#endif
									break;
								}
							}
						} else {
							// Repeat back to them their position.
#ifdef MMULTI_DEBUG_SENDRECV
							debugprintf("mmulti debug: got ping from player %d\n", other);
#endif
							sendother = other;
						}
					} else {
						// Peer-to-peer mode.
						if (other == myconnectindex) {
#ifdef MMULTI_DEBUG_SENDRECV
							debugprintf("mmulti debug: got ping from unknown host %s\n", addr);
#endif
						} else {
#ifdef MMULTI_DEBUG_SENDRECV
							debugprintf("mmulti debug: got ping from peer %d\n", other);
#endif
							sendother = other;
						}
					}

					if (sendother >= 0) {
						savesnatchhost(sendother);
						lastrecvtims[sendother] = tims;

							//   short crc16ofs;        //offset of crc16
							//   int icnt0;             //-1 (special packet for MMULTI.C's player collection)
							//   char type;				// 0xab
							//   char connectindex;
							//   char numplayers;
							//   char netready;
							//   unsigned short crc16;  //CRC16 of everything except crc16
						k = 2;
						*(int *)&pakbuf[k] = -1; k += 4;
						pakbuf[k++] = 0xab;
						pakbuf[k++] = (char)sendother;
						pakbuf[k++] = (char)numplayers;
						pakbuf[k++] = (char)netready;
						*(unsigned short *)&pakbuf[0] = (unsigned short)k;
						*(unsigned short *)&pakbuf[k] = getcrc16(pakbuf,k); k += 2;
						netsend(sendother,pakbuf,k);
					}
				}
				else if (pakbuf[k] == 0xab)
				{
					if (networkmode == MMULTI_MODE_MS) {
						// Master-slave.
						if (((unsigned int)pakbuf[k+1] < (unsigned int)pakbuf[k+2]) &&
							 ((unsigned int)pakbuf[k+2] < (unsigned int)MAXPLAYERS) &&
							 other == connecthead)
						{
#ifdef MMULTI_DEBUG_SENDRECV
								debugprintf("mmulti debug: master gave us player %d in a %d player game\n",
									(int)pakbuf[k+1], (int)pakbuf[k+2]);
#endif

							if ((int)pakbuf[k+1] != myconnectindex ||
									(int)pakbuf[k+2] != numplayers)
							{
								myconnectindex = (int)pakbuf[k+1];
								numplayers = (int)pakbuf[k+2];

								connecthead = 0;
								for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
								connectpoint2[numplayers-1] = -1;
							}

							netready = netready || (int)pakbuf[k+3];

							savesnatchhost(connecthead);
							lastrecvtims[connecthead] = tims;
						}
					} else {
						// Peer-to-peer. Verify that our peer's understanding of the
						// order and player count matches with ours.
						if (myconnectindex != (int)pakbuf[k+1] ||
								numplayers != (int)pakbuf[k+2]) {
							if (!warned) {
								const char *addr = presentaddress((struct sockaddr *)&snatchhost);
								initprintf("mmulti error: host %s (peer %d) believes this machine is "
									"player %d in a %d-player game! The game will not start until "
									"every machine is in agreement.\n",
									addr, other, (int)pakbuf[k+1], (int)pakbuf[k+2]);
							}
							warned = 1;
						} else {
							savesnatchhost(other);
							lastrecvtims[other] = tims;
						}
					}
				}
			}
			else
			{
				if (other == myconnectindex) {
#ifdef MMULTI_DEBUG_SENDRECV
					debugprintf("mmulti debug: got a packet from unknown host %s\n",
								presentaddress((struct sockaddr *)&snatchhost));
					return 0;
#endif
				}
				if (ocnt0[other] < ic0) ocnt0[other] = ic0;
				for(i=ic0;i<min(ic0+256,ocnt1[other]);i++)
					if (pakbuf[((i-ic0)>>3)+k]&(1<<((i-ic0)&7)))
						opak[other][i&(FIFSIZ-1)] = 0;
				k += 32;

				messleng = (int)(*(unsigned short *)&pakbuf[k]); k += 2;
				while (messleng)
				{
					j = *(int *)&pakbuf[k]; k += 4;
					if ((j >= icnt0[other]) && (!ipak[other][j&(FIFSIZ-1)]))
					{
						if (pakmemi+messleng+2 > (int)sizeof(pakmem)) pakmemi = 1;
						ipak[other][j&(FIFSIZ-1)] = pakmemi;
						*(short *)&pakmem[pakmemi] = messleng;
						memcpy(&pakmem[pakmemi+2],&pakbuf[k],messleng); pakmemi += messleng+2;
					}
					k += messleng;
					messleng = (int)(*(unsigned short *)&pakbuf[k]); k += 2;
				}

				lastrecvtims[other] = tims;
			}
		}
	}

		//Return next valid packet from any player
	if (!bufptr) return(0);
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if (i != myconnectindex)
		{
			j = ipak[i][icnt0[i]&(FIFSIZ-1)];
			if (j)
			{
				messleng = *(short *)&pakmem[j]; memcpy(bufptr,&pakmem[j+2],messleng);
				*retother = i; ipak[i][icnt0[i]&(FIFSIZ-1)] = 0; icnt0[i]++;
				return(messleng);
			}
		}
		if ((networkmode == MMULTI_MODE_MS) && (myconnectindex != connecthead)) break; //slaves in M/S mode only send to master
	}

	return(0);
}

// Records the IP address of a peer, along with our IPV4 and/or IPV6 addresses
// their packet came in on. We send our reply from the same address.
void savesnatchhost(int other)
{
	if (other == myconnectindex) return;

	memcpy(&otherhost[other], &snatchhost, sizeof(snatchhost));
	replyfrom4[other] = snatchreplyfrom4;
	replyfrom6[other] = snatchreplyfrom6;
}

#endif // EDUKE32_STANDALONE
