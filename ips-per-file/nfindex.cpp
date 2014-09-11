/*
 * 70MB of memory are consumed.
 * Serialized files has: 14MB
 */
#include <libnfdump/libnfdump.h>
#include <map>
#include <vector>
#define TCP 6
#define FLAG_IPV6_ADDR     1
#include <stdint.h>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
using namespace std; 
namespace fs = boost::filesystem;

int main (int argc, char* argv[])
{
    libnfstates_t* states;
    master_record_t* rec;
    uint64_t num_v6_flows;
    char c;
    if (argc != 2) {
        fprintf(stderr,"An nfcapd file needs to be passed as command line argument\n");
        return (EXIT_FAILURE);
    }

    /* Index how many times an IP address emerged in a file */
    map < uint32_t, uint32_t > index;
    
    /* Initialize libnfdump */
    states = initlib(NULL, argv[1],NULL);
    if (states) {
        do {
            rec = get_next_record(states);
            if (rec) {
                if ( (rec->flags & FLAG_IPV6_ADDR ) != 0 ) {
                    //FIXME Cannot handle v6 flows but count them
                    num_v6_flows++;
                } else {
                    index[rec->v4.dstaddr]++;
                    index[rec->v4.srcaddr]++;
                }
            }
        } while (rec);
        
        /* Close the nfcapd file and free up internal states */
        libcleanup(states);
    }
    // Serialize the data
    ofstream idxFile;
    idxFile.open("test.dat", ofstream::binary);
    boost::archive::text_oarchive oarch(idxFile);
    oarch<<index;
    idxFile.close();
    cout << "Press a key to terminate "<<endl;
    cin >> c;
    return(EXIT_SUCCESS);
} 
