#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <thread>
#include <pthread.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <ctime>


using namespace std;

#define MAX_THREADS_AMOUNT 15
#define MAX_AMOUNT 200
const char FILE_NAME[] = "records.txt";


// Creatiing a mutex for each client account
struct s_Cliacc
{
    
    int acc_no; 
    pthread_mutex_t mutex;  
    float bal;
    string name;
};

struct s_Table
{
    s_Cliacc* accnts[MAX_AMOUNT];
    int accnts_size;
};

// global instance
s_Table account_table;
pthread_mutex_t table_mutex;
s_Cliacc new_account[MAX_AMOUNT];
int transactions_count = 0;
int transactions_usedtime = 0; 


s_Cliacc* find_or_create_account(int ACCT_Number, string ACCT_Name, float Balance)
{
    s_Cliacc* account = NULL;

   
    pthread_mutex_lock(&table_mutex);

   
    for (int i = 0; i < account_table.accnts_size; i++)
    {
        if (ACCT_Number == account_table.accnts[i]->acc_no)
        {
            account = account_table.accnts[i];
            break;
        }
    }

    if (account == NULL)
    {
        new_account[account_table.accnts_size].acc_no = ACCT_Number;
        new_account[account_table.accnts_size].name = ACCT_Name;
        new_account[account_table.accnts_size].bal = Balance;
        pthread_mutex_init(&new_account[account_table.accnts_size].mutex, NULL);
        account_table.accnts[account_table.accnts_size] = &new_account[account_table.accnts_size];
        account_table.accnts_size++;
        cout << " A new account is created: No: " << account_table.accnts[account_table.accnts_size-1]->acc_no << "; Name: " << account_table.accnts[account_table.accnts_size-1]->name << "; Balance: " << account_table.accnts[account_table.accnts_size-1]->bal << ".\n";    
    }

    // release the lock
    pthread_mutex_unlock(&table_mutex);
    return account;
}


vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}


void initial_accounts(){
    account_table.accnts_size = 0;

    string line;
    ifstream recordsfile (FILE_NAME);

    if (!recordsfile.is_open()){
	    cout << "Unable to open the '" << FILE_NAME << "' file, please check the name!\n";
	    exit(0);
       
    } else {
	    while ( getline(recordsfile, line) ){ //reading from records
            try{
                vector<string> v = split(line, " ");
                if (v.size() >= 3){
                    float balance = std::stof(v[2].c_str());
                    int account_no = std::stoi(v[0].c_str());
                    string name = v[1];
                    //create accounts
                    find_or_create_account(account_no, name, balance);
                } else {
                    cout << "Exception Caught "<< ".\n";
                }
            }catch(std::exception const & e) {
                cout << "Exception Caught " ;
            }
        }
        recordsfile.close();
        cout << "Server: accounts initialization finished!\n" << endl;
    }
}

int tran_operatns(float t_stamp, int acc_no, string tran_type, float tran_amt){
	//struct timeval begin, end;
    s_Cliacc* found_account_ptr = find_or_create_account(acc_no, "", 0);
    int result = 0;

    cout << "Transaction No." << transactions_count << " ongoing: " << t_stamp << " " << acc_no  << " " << tran_type << " " << tran_amt << endl;
    //gettimeofday(&begin, NULL);
    //long starttime= system.nanoTime();
    auto start_time = std::chrono::high_resolution_clock::now();
    //https://levelup.gitconnected.com/8-ways-to-measure-execution-time-in-c-c-48634458d0f9
    
    pthread_mutex_lock(&found_account_ptr->mutex);

    if (tran_type.compare("w") == 0){
	    if (found_account_ptr->bal >= tran_amt){
            found_account_ptr->bal -= tran_amt;
            transactions_count++;
	    }
	    else {
            result = -1;
        }
    }

    
	    else if (tran_type.compare("d") == 0){
	    found_account_ptr->bal += tran_amt;
	    transactions_count++;

    }
 
    

    // release the lock
    pthread_mutex_unlock(&found_account_ptr->mutex);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    //gettimeofday(&end, NULL);
    //long endtime= system.nanoTime();
    cout << "Transaction No." << transactions_count -1 << " Done!" << endl;

    auto sec= end_time - start_time;
    transactions_usedtime += sec.count();
    cout << "Average Transaction time : " << transactions_usedtime / transactions_count << endl;
    
    ofstream fileOUT("avg_transaction_complt_time_log.txt", ios::app); 
    fileOUT << "Overall transaction average complete time (ms) is: " << transactions_usedtime / transactions_count << endl; 
    fileOUT.close();
    
    return result;
}

void account_info(){
    cout << "\n Printing Account Info:" << endl;
    
    for (int i = 0; i < account_table.accnts_size; i++)
    {
        cout << "Account No.: " << account_table.accnts[i]->acc_no << " Name: " << account_table.accnts[i]->name << " Remain balance: " << account_table.accnts[i]->bal << endl;
    }
    cout << "Total " << account_table.accnts_size << " accounts." << endl << endl;
}


void *transaction_hanler(void * socket){
    
    int new_socket = *(int*)socket;
    char recv_buf[65536];
    memset(recv_buf, '\0', sizeof(recv_buf));

    float time = 0;
    while (recv(new_socket, recv_buf, sizeof(recv_buf), 0) > 0 ){
        printf("Server: recv from client tid(%ld): '%s' \n", pthread_self(), recv_buf);
        // parse and do transactions
        try {
            vector<string> v = split(recv_buf, " ");
            if (v.size() == 4){
                float timestamp = std::stof(v[0].c_str());
		int account_no = std::stoi(v[1].c_str());
                string transaction_type = v[2];
                float transaction_amount = std::stof(v[3].c_str());
                
                usleep( (timestamp - time)*1000000 ); 
                time = timestamp; 
                int transaction_result = tran_operatns(timestamp, account_no, transaction_type, transaction_amount);

                // sending a message to client
                string msg;
                if (transaction_result == 0){
                    msg = "From server: tid(" + std::to_string(pthread_self()) + ") Transaction " + v[0] + " " + v[1] + " " + transaction_type + " " + v[3] + " Completed.";
                } else if (transaction_result == -1){
                    msg = "From server: tid(" + std::to_string(pthread_self()) +  v[0] + "This " + v[1] + " " + transaction_type + " " + v[3] + " failed, due to low balance";
                } else {
                    msg = "From server: tid(" + std::to_string(pthread_self()) + ") Transaction " + v[0] + " " + v[1] + " " + transaction_type + " " + v[3] + " has failed.";
                }
                send(new_socket , msg.c_str(), msg.length()+1, 0 );
               
                // printings accounts information
                account_info();

            } 
	    else { 
                cout << "Server: Exception Caught, split result size: " << v.size() << ".\n";
            }
        } 
	catch (std::exception const & e) {
            cout << "Server: Exception Caught, transaction: '" << e.what() << "'.\n"; 
        }

        memset(recv_buf, '\0', strlen(recv_buf)); 
    }

    return 0;

}


 
int main(int argc, char *argv[]){

	if(argc !=3) {
	printf("\nPlease provide port no. and IP address in command line argument.\n");
	}
	else {
    initial_accounts();
    account_info();

    int server_sock_fd, new_socket, valread;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons( atoi(argv[1]) ); 
    int opt = 1;

    // Creating socket file descriptor 
    if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Server: socket failed");
        exit(1);
    }

    
    if (bind(server_sock_fd, (struct sockaddr *)&server_address,
                                 sizeof(server_address))<0)
    {
        perror("Server binding failed");
        exit(1);
    }

    if (listen(server_sock_fd, 7) < 0)
    {
        perror("Error while listening");
        exit(1);
    }
   
    // multi threads
    pthread_t thread_id[MAX_THREADS_AMOUNT]; 
    int thread_id_ctr = 0;

    while (1) 
    { 
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        if ((new_socket = accept(server_sock_fd, (struct sockaddr *)&client_addr,(socklen_t*)&length))<0)
        {
            perror("Error in accepting");
            exit(1);
        }

     
        char client_ip[INET_ADDRSTRLEN] = "";
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        // creating a thread for each client
        if (pthread_create(&thread_id[thread_id_ctr], NULL, transaction_hanler, (void *) &new_socket) < 0)
        {
            perror("Server: create thread failed");
            exit(1);
        }
        thread_id_ctr++;
        printf("Server: new thread created with threadid: %ld\n", pthread_self());
    }

    for (int i = 0; i < MAX_THREADS_AMOUNT; i++){
        pthread_join(thread_id[i], NULL);
    }
    pthread_exit(NULL);

    close(server_sock_fd);
    return 0;
}
}

/*References: https://cplusplus.com/reference/
 * https://en.cppreference.com/w/cpp/chrono
 * */
