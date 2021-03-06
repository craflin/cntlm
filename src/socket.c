/*
 * These are socket routines for the main module of CNTLM
 *
 * CNTLM is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * CNTLM is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright (c) 2007 David Kubicek
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef _WIN32
#include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <syslog.h>

#include "utils.h"
#include "socket.h"

extern int debug;

/*
 * gethostbyname() wrapper. Return 1 if OK, otherwise 0.
 */
int so_resolv(struct in_addr *host, const char *name) {
	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int rc = getaddrinfo(name, NULL, &hints, &res);
	if (rc != 0) {
		if (debug)
			printf("so_resolv: %s failed: %s (%d)\n", name, gai_strerror(rc), rc);
		return 0;
	}

	if (debug)
		printf("Resolve %s:\n", name);
	int addr_set = 0;
	for (p = res; p != NULL; p = p->ai_next) {
		struct sockaddr_in *ad = (struct sockaddr_in*)(p->ai_addr);
		if (ad == NULL) {
			freeaddrinfo(res);
			return 0;
		}
		if (!addr_set) {
			memcpy(host, &ad->sin_addr, sizeof(ad->sin_addr));
			addr_set = 1;
			if (debug)
				printf("  -> %s\n", inet_ntoa(ad->sin_addr));
		} else
			if (debug)
				printf("     %s\n", inet_ntoa(ad->sin_addr));
	}

	freeaddrinfo(res);

	return addr_set;
}

/*
 * Connect to a host. Host is required to be resolved
 * in the struct in_addr already.
 * Returns: socket descriptor
 */
int so_connect(struct in_addr host, int port) {
	int fd, rc;
	struct sockaddr_in saddr;

	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		if (debug)
			printf("so_connect: create: %s\n", so_strerror(so_errno));
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr = host;

	rc = connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));
	if (rc < 0) {
		if (debug)
			printf("so_connect: %s\n", so_strerror(so_errno));
		so_close(fd);
		return -1;
	}

	return fd;
}

/*
 * Bind the specified port and listen on it.
 * Return socket descriptor if OK, otherwise 0.
 */
int so_listen(int port, struct in_addr source) {
	struct sockaddr_in saddr;
	int fd;
	socklen_t clen;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		if (debug)
			printf("so_listen: new socket: %s\n", so_strerror(so_errno));
		return -1;
	}

	clen = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&clen, sizeof(clen));
	memset((void *)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = source.s_addr;

	if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr))) {
		syslog(LOG_ERR, "Cannot bind port %d: %s!\n", port, so_strerror(so_errno));
		so_close(fd);
		return -1;
	}

	if (listen(fd, SOMAXCONN)) {
		so_close(fd);
		return -1;
	}

	return fd;
}

/*
 * Return 1 if data is available on the socket,
 * 0 if connection was closed
 * -1 if error (errno is set)
 */
int so_recvtest(int fd) {
	char buf;
	int i;
#ifdef _WIN32
    fd_set rds;
    FD_ZERO(&rds);
    FD_SET(fd, &rds);
    struct timeval tv = {0};
    if(select(0, &rds, NULL, NULL, &tv) != 1)
    {
        WSASetLastError(WSAEWOULDBLOCK);
        return -1;
    }
	i = recv(fd, &buf, 1, MSG_PEEK);
#elif !defined(MSG_DONTWAIT)
	unsigned int flags;

	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	i = recv(fd, &buf, 1, MSG_PEEK);
	fcntl(fd, F_SETFL, flags);
#else
	i = recv(fd, &buf, 1, MSG_DONTWAIT | MSG_PEEK);
#endif

	return i;
}

/*
 * Reliable way of finding out whether a connection was closed
 * on the remote end, without actually reading from it.
 */
int so_closed(int fd) {
	int i;

	if (fd == -1)
		return 1;

	i = so_recvtest(fd);
#ifdef _WIN32
	return (i == 0 || (i == -1 && so_errno != WSAEWOULDBLOCK));
#else
	return (i == 0 || (i == -1 && so_errno != EAGAIN && so_errno != ENOENT));   /* ENOENT, you ask? Perhaps AIX devels could explain! :-( */
#endif
}

/*
 * Receive a single line from the socket. This is no super-efficient
 * implementation, but more than we need to read in a few headers.
 * What's more, the data is actually recv'd from a socket buffer.
 *
 * I had to time this in comparison to recv with block read :) and
 * the performance was very similar. Given the fact that it keeps us
 * from creating a whole buffering scheme around the socket (HTTP 
 * connection is both line and block oriented, switching back and forth),
 * it is actually OK.
 */
int so_recvln(int fd, char **buf, int *size) {
	int len = 0;
	int r = 1;
	char c = 0;
	char *tmp;

	while (len < *size-1 && c != '\n') {
		r = so_read(fd, &c, 1);
		if (r <= 0)
			break;

		(*buf)[len++] = c;

		/*
		 * End of buffer, still no EOL? Resize the buffer
		 */
		if (len == *size-1 && c != '\n') {
			if (debug)
				printf("so_recvln(%d): realloc %d\n", fd, *size*2);
			*size *= 2;
			tmp = realloc(*buf, *size);
			if (tmp == NULL)
				return -1;
			else
				*buf = tmp;
		}
	}
	(*buf)[len] = 0;

	return r;
}

int so_read(int fd,  void *buf, int len)
{
    return recv(fd, buf, len, 0);
}

int so_write(int fd,  const void *buf, int len)
{
#ifndef _WIN32
    return send(fd, buf, len, MSG_NOSIGNAL);
#else
    return send(fd, buf, len, 0);
#endif
}

int so_close(int fd)
{
#ifndef _WIN32
    return close(fd);
#else
    return closesocket(fd);
#endif
}

const char* so_strerror(int errnum)
{
#ifdef _WIN32
	return "";
#else
	return strerror(errnum);
#endif
}

int so_geterrno()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}
