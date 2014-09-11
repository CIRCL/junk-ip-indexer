/*
 * Issues:
 *
 * The index contains more addresses than there are in the nfcapd file?
 * Bug in libnfdump, index, indexing code?
 * TODO write C program that just dumps the addresses and compare
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

    if (argc != 2) {
        fprintf(stderr,"An nfcapd file needs to be passed as command line argument\n");
        return (EXIT_FAILURE);
    }
    
    /* Initialize libnfdump */
    states = initlib(NULL, argv[1],NULL);
 
    if (states) {
        do {
            rec = get_next_record(states);
            if (rec) {
            }
        } while (rec);
        
        /* Close the nfcapd file and free up internal states */
        libcleanup(states);
    }
    return(EXIT_SUCCESS);
} 
