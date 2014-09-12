/*
 * Copy flows and let them close to each others
 * For one nfcapd file only 117MB of memory is consumed although the flows
 * are duplicated
 * 
 * Alternative: create blocks of flows and put bloomfilters in front
 * 
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

class FlowRecord {
    public:
        uint32_t srcaddr;
        uint32_t dstaddr;
        uint16_t srcport;
        uint16_t dstport;
        uint32_t first;
        uint32_t last;
        uint8_t prot;
        uint64_t dPkts;
        uint64_t dOctets;
};

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
    map < uint32_t, FlowRecord > index;
    
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
                    // Index the full flow for the source IP
                    index[rec->v4.srcaddr].srcaddr = rec->v4.srcaddr;
                    index[rec->v4.srcaddr].dstaddr = rec->v4.dstaddr;
                    index[rec->v4.srcaddr].srcport = rec->srcport;
                    index[rec->v4.srcaddr].dstport = rec->dstport;
                    index[rec->v4.srcaddr].first = rec->first;
                    index[rec->v4.srcaddr].last = rec->last;
                    index[rec->v4.srcaddr].prot = rec->prot;            
                    index[rec->v4.srcaddr].dPkts = rec->dPkts;
                    index[rec->v4.srcaddr].dOctets = rec->dOctets;
                    // Index the full flow for the destination IP
                    index[rec->v4.dstaddr].srcaddr = rec->v4.srcaddr;
                    index[rec->v4.dstaddr].dstaddr = rec->v4.dstaddr;
                    index[rec->v4.dstaddr].srcport = rec->srcport;
                    index[rec->v4.dstaddr].dstport = rec->dstport;
                    index[rec->v4.dstaddr].first = rec->first;
                    index[rec->v4.dstaddr].last = rec->last;
                    index[rec->v4.dstaddr].prot = rec->prot;            
                    index[rec->v4.dstaddr].dPkts = rec->dPkts;
                    index[rec->v4.dstaddr].dOctets = rec->dOctets;
                }
            }
        } while (rec);
        
        /* Close the nfcapd file and free up internal states */
        libcleanup(states);
    }
    cout << "Press a key to terminate "<<endl;
    cin >> c;
    return(EXIT_SUCCESS);
} 
