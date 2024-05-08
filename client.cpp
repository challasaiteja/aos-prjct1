#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

#define PORT 8000

const char FILE_NAME[] = "transactions_32clients_w_rate.txt";


   
int main(int argc, char const *argv[])
{
	if(argc!=3) {
	printf("Please provide port no. and IP address in command line argument.\n");
	}
	else {
    ofstream fileOUT("avg_transaction_time.txt", ios::app); 
    if (argc == 2) {
        fileOUT << "Current rate is: " << argv[1] << endl;
    }
    fileOUT.close(); 


    int client_sock_fd;
    char buffer[1024] = {0};
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr= inet_addr(argv[2]);
    server_addr.sin_port = htons(atoi(argv[1]));  
    
    
    
    if ((client_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf(" Unable to create socket \n");
        return -1;
    }
   
    
    if (connect(client_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf(" Connection Failed \n");
        return -1;
    }

    char server_ip[INET_ADDRSTRLEN]="";
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
      
    string line;
    ifstream TRANS_FILE (FILE_NAME);
    if (!(TRANS_FILE.is_open())){
	    TRANS_FILE.close();
	    cout << "Unable to open the '" << FILE_NAME << "' file, please check the name!\n";
	    return -1;
  
    } else {
	    while ( getline(TRANS_FILE, line) ){



            
            send(client_sock_fd , line.c_str() , line.length()+1 , 0 );
           // cout << "Request for the transaction " << line << endl;
            read( client_sock_fd , buffer, 1024);
            printf("Client: read: '%s'\n",buffer );
            memset(buffer, '\0', strlen(buffer));

        }
    }
    close(client_sock_fd);
    return 0;
	}
}
