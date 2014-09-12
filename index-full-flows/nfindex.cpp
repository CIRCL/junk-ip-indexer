/*
 *  Compression test 
 *  Strip nfrecords
 *  
 *  Orginal nfcapd file size: 103 MB 
 *  file of sequences of flow_record_t consumes 171MB
 *  gzip the entire file with bash command line: 58MB
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
#include <stdlib.h>
using namespace std; 

namespace fs = boost::filesystem;

typedef struct flow_record_s {    
        uint32_t srcaddr;
        uint32_t dstaddr;
        uint16_t srcport;
        uint16_t dstport;
        uint32_t first;
        uint32_t last;
        uint8_t prot;
        uint64_t dPkts;
        uint64_t dOctets;
} flow_record_t;

int main (int argc, char* argv[])
{
    libnfstates_t* states;
    master_record_t* rec;
    flow_record_t flow;
    ofstream f;
    uint64_t num_v6_flows;
    char c;
    if (argc != 2) {
        fprintf(stderr,"An nfcapd file needs to be passed as command line argument\n");
        return (EXIT_FAILURE);
    }

    
    f.open("test.dat", ios::out | ios::binary);
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
                    flow.srcaddr = rec->v4.srcaddr;
                    flow.srcport = rec->srcport;
                    flow.dstaddr = rec->v4.dstaddr;
                    flow.first = rec->first;
                    flow.last = rec->last;
                    flow.prot = rec->prot;
                    flow.dPkts = rec->dPkts;
                    flow.dOctets = rec->dOctets;
                    f.write((char*)&flow, sizeof(flow_record_t));
                }
            }
        } while (rec);
        f.close();
        /* Close the nfcapd file and free up internal states */
        libcleanup(states);
    }
    cout << "Press a key to terminate "<<endl;
    cin >> c;
    return(EXIT_SUCCESS);
} 
