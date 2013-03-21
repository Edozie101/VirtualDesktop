#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "iphone_orientation_listener.h"

void* setup_socket(void *inputData);

#define BUFLEN 512
#define NPACK 1000
#define PORT 1982

static pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void diep(const char *s)
{
  perror(s);
//  exit(1);
}

static double matrix[9] = {1, 0, 0,
						   0, 1, 0,
						   0, 0, 1};

void print_local_addresses() {
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;

	std::cout << "Listening to following addresses: " << std::endl;
	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
		} else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
		}
	}
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}

static pthread_t listener_thread;
void setup_iphone_listener() {
	pthread_create(&listener_thread, NULL, setup_socket, NULL);
	print_local_addresses();
}
void* setup_socket(void *inputData)
{
  struct sockaddr_in si_me, si_other;
  int s;
  socklen_t slen=sizeof(si_other);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    diep("socket");

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (const sockaddr*)&si_me, sizeof(si_me))==-1)
      diep("bind");

  std::cerr << "BOUND" << std::endl;
  double buf2[512];
//  for (i=0; i<NPACK; i++) {
  while(1) {
	  const ssize_t count = recvfrom(s, buf2, BUFLEN, 0, (sockaddr*)&si_other, &slen);
    if (count ==-1)
      diep("recvfrom()");
//    printf("Received packet from %s:%d\nCount: %d\n\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), count);//, buf[0]);
    for(int i2 = 0; i2 < 9; ++i2) {
    	pthread_mutex_lock(&mymutex);
//    	printf("%f, ", buf2[i2]);
    	matrix[i2] = buf2[i2];
    	pthread_mutex_unlock(&mymutex);
//    	if((i2+1)%3 == 0) printf("\n");
    }
//    printf("\n");
//    std::cerr << "RECEIVED" << std::endl;
  }

  return NULL;
}

static double matrix_returned[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
};
const double *get_orientation() {
        int i2 = 0;
        pthread_mutex_lock(&mymutex);
        for(int i = 0; i < 12/*16*/; ++i) {
                if((i%4)!=3) {
                        matrix_returned[i] = matrix[i2++];
//                      std::cerr << matrix_returned[i];
//                      if(i2%3 == 0) std::cerr  << std::endl;
                }
//              std::cerr << matrix_returned[i];
//              if((i+1)%4 == 0) std::cerr  << std::endl;
        }
//      std::cerr << std::endl;
        pthread_mutex_unlock(&mymutex);
        return matrix_returned;
}

static float matrix_returned_f[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
};
const float *get_orientation_f() {
        int i2 = 0;
        pthread_mutex_lock(&mymutex);
        for(int i = 0; i < 12/*16*/; ++i) {
                if((i%4)!=3) {
                        matrix_returned_f[i] = matrix[i2++];
//                      std::cerr << matrix_returned[i];
//                      if(i2%3 == 0) std::cerr  << std::endl;
                }
//              std::cerr << matrix_returned[i];
//              if((i+1)%4 == 0) std::cerr  << std::endl;
        }
//      std::cerr << std::endl;
        pthread_mutex_unlock(&mymutex);
        return matrix_returned_f;
}
