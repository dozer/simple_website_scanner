#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "hacking.h"
#include "hacking-network.h"

int main(int argc, char *argv[])
{
	struct hostent *host_info;
	struct in_addr *address, **addrlist_ptr;
	char **aliases;
	int sockfd;
	struct sockaddr_in target_addr;
	unsigned char buffer[4096];

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
		exit(1);
	}

	//Get host info
	host_info = gethostbyname(argv[1]);
	if(host_info == NULL) 
	{
		fprintf(stderr, "Couldn't lookup %s\n", argv[1]);
	}
	else 
	{
		printf("%s has the following config:\n", argv[1]);

		//Print host address
		address = (struct in_addr *) (host_info->h_addr);
		printf("\tAddress: %s\n", inet_ntoa(*address));
		
		//Print host address type
		char type[5] = "";
		if (host_info->h_addrtype == 2) 
		{
			strcpy(type, "IPv4");
		} 
		else 
		{ 
			strcpy(type, "IPv6");
		}
		printf("\tAddress type: %s\n", type);
		
		//Print host address length
		printf("\tAddress length: %d%s\n", host_info->h_length, " bytes");
		
		//Print host's alternate addresses
		addrlist_ptr = (struct in_addr **) host_info->h_addr_list;
		addrlist_ptr++; //Skips the first entry which was listed above
		if (*addrlist_ptr != NULL ) 
		{
			printf("\tAlternate addresses:\n");
		}
		while( *addrlist_ptr != NULL ) 
		{
        		printf("\t\t%s\n", inet_ntoa(**(addrlist_ptr++)));
    		}

		//Print host's aliases (alternate URL's)		
		aliases = host_info->h_aliases;			
		if (*aliases != NULL)
		{
			printf("\tKnown Aliases:\n");
		}
		while(*aliases != NULL)
		{
			printf("\t\t%s\n", *aliases);
			aliases++;
		}

		//Try to create socket
		if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
                	fatal("in socket");

        	target_addr.sin_family = AF_INET;  //Set family as IPv4
        	target_addr.sin_port = htons(80);  //Set port to 80
        	target_addr.sin_addr = *((struct in_addr *)host_info->h_addr);  //Set host to provided IP
        	memset(&(target_addr.sin_zero), '\0', 8); // zero the rest of the struct

		//Try to connect to target_addr
        	if (connect(sockfd, (struct sockaddr *)&target_addr, sizeof(struct sockaddr)) == -1)
                	fatal("connecting to target server");

		//Request HTTP headers
        	send_string(sockfd, "HEAD / HTTP/1.0\r\n\r\n");

		//Search headers for web server
        	while(recv_line(sockfd, buffer)) 
		{
                	if(strncasecmp(buffer, "Server:", 7) == 0) 
			{
				printf("\tWeb server: %s\n", buffer+8);
                        	exit(0);
                	}
        	}
	}
	return 0;
}
