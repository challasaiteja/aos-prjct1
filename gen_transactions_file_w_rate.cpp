#include <iostream>
#include <fstream>
using namespace std;


int main(int argc, char *argv[]) {
    /* deal with input arguments*/
    std::cout << "print arguments:\nargc == " << argc << '\n'; // input count
    if (argc == 2){
        std::cout << "argv is " << atof(argv[1]) << '\n'; // input milliseconds
    }
    float rate = atof(argv[1])/float(1000); // convert to seconds
    std::cout << "rate is " << rate << '\n';


    ofstream myfile;
    myfile.open ("Transactions_25clients_w_rate.txt");

    for (float i = 0; i < 10000; i++){
        float timestamp = 5 + i/(1/rate);
        string type = "d";
        if (int(i) % 2 == 0){ type = "w"; }
        string aline = std::to_string(timestamp) + " 101 " + type + " " + std::to_string(1); 
        myfile << aline << endl;
    }

    
    myfile.close();
    return 0;
}
