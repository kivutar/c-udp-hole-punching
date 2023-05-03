#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void diep(char *s)
{
    perror(s);
    exit(1);
}

char* receive_data(int s, struct sockaddr_in addr)
{
    int slen = sizeof(struct sockaddr_in);
    char* buf = malloc(64 * sizeof(char));
    memset(buf, 0, 64);

    if (recvfrom(s, buf, 64 * sizeof(char), 0, (struct sockaddr*)(&addr), &slen) == -1)
        diep("recvfrom");

    printf("Received packet %s from %s:%d\n", buf, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return buf;
}

void send_data(int s, struct sockaddr_in addr, char* msg)
{
    sleep(1);
    if (sendto(s, msg, strlen(msg), 0, (struct sockaddr*)(&addr), sizeof(addr)) == -1)
        diep("sendto");

    printf("Sent %s to %s:%d\n", msg, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int main(int argc, char* argv[])
{
    struct sockaddr_in addr_me, addr_rdv, addr_peer;
    int slen = sizeof(struct sockaddr_in);

    int rdv = 0;
    if ((rdv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &addr_rdv, 0, sizeof(addr_rdv));
    addr_rdv.sin_family = AF_INET;
    addr_rdv.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], &addr_rdv.sin_addr)==0)
        diep("aton");

    send_data(rdv, addr_rdv, "hi");

    char* buf = receive_data(rdv, addr_rdv);

    // parse the received data, it contains the index, the ip, and the port, separated by a colon
    char *token1 = strtok(buf, ":");
    int index1 = atoi(token1);
    token1 = strtok(NULL, ":");
    char *ip1 = token1;
    token1 = strtok(NULL, ":");
    int port1 = atoi(token1);

    printf("I am player index: %d, ip: %s, port: %d\n", index1, ip1, port1);

    char* buf2 = receive_data(rdv, addr_rdv);

    char* token2 = strtok(buf2, ":");
    int index2 = atoi(token2);
    token2 = strtok(NULL, ":");
    char* ip2 = token2;
    token2 = strtok(NULL, ":");
    int port2 = atoi(token2);

    printf("I see player index: %d, ip: %s, port: %d\n", index2, ip2, port2);

    close(rdv);

    if (strcmp(ip1, ip2) == 0) {
        strcpy(ip1, "127.0.0.1");
        strcpy(ip2, "127.0.0.1");
    }

    // create a socket for the peers
    int p2p = 0;
    if ((p2p = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    // own address
    memset((char *) &addr_me, 0, sizeof(addr_me));
    addr_me.sin_family = AF_INET;
    addr_me.sin_port = htons(port1);
    addr_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // peer address
    memset((char *) &addr_peer, 0, sizeof(addr_peer));
    addr_peer.sin_family = AF_INET;
    addr_peer.sin_port = htons(port2);
    if (inet_aton(ip2, &addr_peer.sin_addr) == 0)
        diep("aton");

    // bind the socket to a port
    if (bind(p2p, (struct sockaddr*)(&addr_me), sizeof(addr_me)) == -1)
        diep("bind");

    // send hello to the peer
    send_data(p2p, addr_peer, "hello");

    // receive hello from the peer
    char* buf3 = receive_data(p2p, addr_peer);

    // send hohai to the peer
    send_data(p2p, addr_peer, "hohai");

    // receive hohai from the peer
    char* buf4 = receive_data(p2p, addr_peer);
}
