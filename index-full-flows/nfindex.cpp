/*
 *  Compression test 
 *  Strip nfrecords
 *  
 *  Orginal nfcapd file size: 103 MB 
 *  file of sequences of flow_record_t consumes 171MB
 *  gzip the entire file with bash command line: 58MB
 *  Using string buffers program uses at peak +- 500MB if RAM
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
#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
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
    stringstream buffer;
    stringstream comp_buffer;
    if (argc != 2) {
        fprintf(stderr,"An nfcapd file needs to be passed as command line argument\n");
        return (EXIT_FAILURE);
    }

    //f.open("test.dat", ios::out | ios::binary);
    //boost::iostreams::filtering_streambuf<boost::iostreams::output> out;    
    //out.push(boost::iostreams::zlib_compressor());Y
    //out.push(boost::iostreams::gzip_compressor());
    //out.push(f); 
    //char data[5] = {'a', 'b', 'c', 'd', 'e'};    
    //boost::iostreams::copy(boost::iostreams::basic_array_source<char>(data, sizeof(data)), out);
    //return 0;
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
                    //FIXME the previous sample code can only compress data 
                    //one time with the copy method? A mix of compressed data
                    //and uncompressed data must go through another string 
                    //buffer
                    buffer.write((char*)&flow, sizeof(flow_record_t));
                }
            }
        } while (rec);
        /* Close the nfcapd file and free up internal states */
        libcleanup(states);
    }
    
    boost::iostreams::filtering_streambuf<boost::iostreams::output> out;    
    out.push(boost::iostreams::gzip_compressor()); 
    out.push(comp_buffer);
    boost::iostreams::copy(buffer, out);

    f.open("test.dat.gz", ios::out | ios::binary);
    f<<"Some gzip compressed data follows this string";
    f.write(comp_buffer.str().c_str(), comp_buffer.str().length());
    f.close();
    cout << "Press a key to terminate "<<endl;
    cin >> c;
    return(EXIT_SUCCESS);
} 
