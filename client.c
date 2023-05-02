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
    
    sleep(1);
    if (sendto(rdv, "hi", 2, 0, (struct sockaddr*)(&addr_rdv), sizeof(addr_rdv)) == -1)
        diep("sendto");

    printf("sent hi to %s:%d\n", argv[1], atoi(argv[2]));

    // create a buffer buf to receive data
    char buf[64] = {0};
    if (recvfrom(rdv, &buf, sizeof(buf), 0, (struct sockaddr*)(&addr_rdv), &slen) == -1)
        diep("recvfrom");

    printf("Received packet %s from %s:%d\n", &buf, inet_ntoa(addr_rdv.sin_addr), ntohs(addr_rdv.sin_port));

    // parse the received data, it contains the index, the ip, and the port, separated by a colon
    char *token1 = strtok(buf, ":");
    int index1 = atoi(token1);
    token1 = strtok(NULL, ":");
    char *ip1 = token1;
    token1 = strtok(NULL, ":");
    int port1 = atoi(token1);

    printf("I am player index: %d, ip: %s, port: %d\n", index1, ip1, port1);

    if (recvfrom(rdv, &buf, sizeof(buf), 0, (struct sockaddr*)(&addr_rdv), &slen) == -1)
        diep("recvfrom");

    printf("Received packet %s from %s:%d\n", &buf, inet_ntoa(addr_rdv.sin_addr), ntohs(addr_rdv.sin_port));

    char *token2 = strtok(buf, ":");
    int index2 = atoi(token2);
    token2 = strtok(NULL, ":");
    char *ip2 = token2;
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
    
    memset((char *) &addr_me, 0, sizeof(addr_me));
    addr_me.sin_family = AF_INET;
    addr_me.sin_port = htons(port1);
    addr_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // address of the peer
    memset((char *) &addr_peer, 0, sizeof(addr_peer));
    addr_peer.sin_family = AF_INET;
    addr_peer.sin_port = htons(port2);
    if (inet_aton(ip2, &addr_peer.sin_addr) == 0)
        diep("aton");

    // bind the socket to a port
    if (bind(p2p, (struct sockaddr*)(&addr_me), sizeof(addr_me)) == -1)
        diep("bind");
    
    sleep(1);
    // send a hello message to the peer
    if (sendto(p2p, "hello", 5, 0, (struct sockaddr*)(&addr_peer), sizeof(addr_peer)) == -1)
        diep("sendto");

    printf("sent hello to %s:%d\n", inet_ntoa(addr_peer.sin_addr), ntohs(addr_peer.sin_port));

    // receive a message from the peer
    char buf2[64] = {0};
    if (recvfrom(p2p, &buf2, sizeof(buf), 0, (struct sockaddr*)(&addr_peer), &slen) == -1)
        diep("recvfrom");

    printf("Received packet %s from %s:%d\n", &buf2, inet_ntoa(addr_peer.sin_addr), ntohs(addr_peer.sin_port));

    // send hohai to the peer
    sleep(1);
    if (sendto(p2p, "hohai", 5, 0, (struct sockaddr*)(&addr_peer), sizeof(addr_peer)) == -1)
        diep("sendto");
    
    // receive a message from the peer
    char buf3[64] = {0};
    if (recvfrom(p2p, &buf3, sizeof(buf), 0, (struct sockaddr*)(&addr_peer), &slen) == -1)
        diep("recvfrom");

    printf("Received packet %s from %s:%d\n", &buf3, inet_ntoa(addr_peer.sin_addr), ntohs(addr_peer.sin_port));
}
