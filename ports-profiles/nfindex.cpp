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

class NetflowIndex {
    private:
        int num_v6_flows;
        map < uint32_t, map < uint16_t, uint32_t > > ips;
    public:
        NetflowIndex() : num_v6_flows(0) {};
        void createIndex(string nfcapdFileName);
        void storeIndex(string indexFileName);
        void loadIndex(string indexFileName);
        void dumpIndex();
        void bulkImport(string rootDirectory, string indexFileName);
        void info();
        void queryFull(string ipaddr);
};

void NetflowIndex::createIndex(string nfcapdFileName)
{
    libnfstates_t* states;                                               
    master_record_t* rec;
   //If frequency is 32bit or 8bit does not change a lot in memory consumption
    //of the overall program. 32 bit integers consume 430MB and 8 bit 429MB. 
    //TODO  Count in 32bit integers and store the log or sqrt value of the 
    //counters to save storage space.
    //FIXME ports do not need to be sorted per port but by frequency
    int num_ips = 1;
    char x;
    /* Initialize libnfdump */                                                  
    states = initlib(NULL, (char*)nfcapdFileName.c_str(),NULL);
    if (states) {                                                               
        do {                                                                    
            rec = get_next_record(states);                                      
            if (rec) {
                if ( (rec->flags & FLAG_IPV6_ADDR ) != 0 ) {
                    //FIXME Cannot handle v6 flows but count them
                    num_v6_flows++;
                } else {
                    ips[rec->v4.dstaddr][rec->srcport]++;
                    ips[rec->v4.dstaddr][rec->dstport]++;
                    ips[rec->v4.srcaddr][rec->srcport]++;
                    ips[rec->v4.srcaddr][rec->dstport]++;
                }
            }                                                               
        } while (rec);                          
    }
    //free libnfdump internal states
    libcleanup(states);
    cout << "[ERROR] Could not handle "<<num_v6_flows << " IPv6 flow records"
         <<endl;
}

void NetflowIndex::dumpIndex()
{
    uint32_t freq;
    uint32_t ip;
    char ipaddr [32];
        map < uint32_t, map < uint16_t, uint32_t > > ::iterator it;
        pair < uint32_t, map < uint16_t, uint32_t > > p;
    pair < uint16_t, uint32_t> q;
    uint16_t port;
    map < uint16_t, uint32_t > m;
    map < uint16_t, uint32_t >::iterator jt;
     for (it = ips.begin(); it != ips.end(); ++it) {
        p = *it;
        ip = p.first;
        m = p.second;
        for (jt = m.begin(); jt != m.end(); ++jt) {
            q = *jt;
            port = q.first;
            freq = q.second;
    // make IP address readable
    // IPs are sorted in host byte order for the dotted decimal notations
    // the ips must be converted in network byte order. Hence, the ips do
    // not look sorted
    ip = htonl(ip);
    inet_ntop(AF_INET, &ip, ipaddr, sizeof(ipaddr));

            cout << ip <<" "<< ipaddr << " " << port << " " << freq << endl;
        }
    }
}

// Print some random data about the map
void NetflowIndex::info()
{
    //FIXME does this correspond to the number of keys?
    cout << "[INFO] Number of IPs "<<ips.size() << endl;
}

void NetflowIndex::storeIndex(string indexFileName)
{
    ofstream idxFile;
    idxFile.open(indexFileName.c_str(), ofstream::binary);
    boost::archive::text_oarchive oarch(idxFile);
    oarch<<ips;
    idxFile.close();
}

void NetflowIndex::loadIndex(string indexFileName)
{
    ifstream idxFile;
    idxFile.open(indexFileName.c_str(), ifstream::binary);
    boost::archive::text_iarchive iarch(idxFile);
    iarch >> ips;
}

void NetflowIndex::bulkImport(string rootDirectory, string indexFileName)
{
    fs::directory_iterator it(rootDirectory);
    fs::directory_iterator end;
    string filename;
    for (it; it != end; ++it) {
        //FIXME remove double / if needed
        filename = rootDirectory + "/" + it->path().filename().string();
        cout << "[INFO] Indexing filename " << filename <<endl;
        createIndex(filename);
    }
    storeIndex(indexFileName);
}

void NetflowIndex::queryFull(string ipaddr)
{
    uint32_t ip;
    map < uint32_t, map < uint16_t, uint32_t > > ::iterator it;
    pair < uint32_t, map < uint16_t, uint32_t > > p;
    pair < uint16_t, uint32_t> q;
    map < uint16_t, uint32_t > m;
    map < uint16_t, uint32_t > :: iterator jt;
    // dumpIndex(); --> data is there

    inet_pton(AF_INET, ipaddr.c_str(), &ip);
    cout << "Raw IP Network " << ip << endl;
    //ip = ntohl(ip);
    cout << "Raw IP Host " << ip << endl;
    cout << "# Port distribution for " << ipaddr << endl;
    cout << "# Port|Frequency" << endl;
    m = ips[ip];
    for (jt=m.begin(); jt != m.end(); ++jt) {
        q = *jt;
        cout << q.first << " "<< q.second << endl;
    }
    //for (it=ips.find(ip); it != ips.end(); ++it) {
    //    p = *it;
    //    cout << "FOUND IP "<<p.first <<endl;
    //}
    //FIXME Sort according port that was used most
    //for (it = ips[ip].begin(); it != ips[ip].end(); ++it) {
    //    cout << (*it).first << "|" << (*it).second << endl;
    //}
}

void print_usage(void) {
    cout<<"bnfindex [-h] [-i] [-c] [-r] [-d] [-n]\n";
    cout << "   -h --help   Shows this screen\n"
         << "   -i --index  filename of the index file\n"
         << "   -c --create create index file\n"
         << "   -d --dump   dump index file on standard output\n"
         << "   -n --nfcapd filename of the nfcapd file\n"
         << "   -b --bulk Import all nfcapd files of this directory in index\n"
         << "   -s --stats Print some random stats\n"
         << "   -r --rawquery Print the raw port distributions of the maps\n"
         << "                 regarding IP addresses in dotted decimal\n"
         << "                 notation. The IP addresses are read from stdin\n";
}

int main(int argc, char* argv[] ) {
    NetflowIndex nfidx;
    int next_option;
    const struct option long_options[] = {
                    { "help", 0, NULL, 'h' },
                    { "create", 0, NULL, 'c' },
                    { "dump", 0, NULL, 'd' },
                    { "index", 1, NULL, 'i' },
                    { "nfcapd", 1, NULL, 'n'},
                    { "bulk", 1, NULL, 'b' },
                    { "stats", 0, NULL, 's'},
                    { "rawquery", 1, NULL, 'r' },
                    {NULL,0,NULL,0},
                 };
    const char* const short_options = "b:hcdi:n:sr";
    bool shouldCreate = false;
    bool shouldDump   = false;
    bool shouldPrintStats = false;
    bool shouldQuery = false;
    string indexFileName;
    string nfcapdFileName;
    string rootDirectory;
    string ipaddr;
    do  {
        next_option = getopt_long(argc, argv, short_options, long_options,
                                  NULL);
        switch (next_option) {
            case 'h':
                print_usage();
                return 0;
            case 'c':
                shouldCreate = true;
                break;
            case 'd':
                shouldDump = true;
                break;
            case 'n':
                nfcapdFileName = string(optarg);
                break;
            case 'i':
                indexFileName = string(optarg);
                break;
            case 'b':
                // remove trailing slash if needed
                if (optarg[strlen(optarg)-1] == '/') {
                    optarg[strlen(optarg)-1] = 0;
                }
                rootDirectory = string(optarg);
                break;
            case 's':
                shouldPrintStats = true;
                break;
            case 'r':
                shouldQuery = true;
                break;
            case '?':
                cerr<<"An invalid command line was specified\n";
                break;
            case -1:
                break;
        }
    } while ( next_option != -1 );

    // Test mandatory options


    if ( shouldQuery ) {
        if ( indexFileName == "" ) {
            cerr<< "An index file name must be specified\n";
            return EXIT_FAILURE;
        }
        cout << "[INFO] Loading index ..."<< endl;
        nfidx.loadIndex(indexFileName);
        cout << "[INFO] Doing lookup ..."<< endl;
        // Read IP addresses from stdin
        while ( cin >> ipaddr ) {
            //FIXME Test if the ip address is valid
            nfidx.queryFull(ipaddr);            // Do the query
        }
        cout << "[INFO] Done (doing cleanup ...)" <<endl;
    }

    if ( shouldPrintStats ) {
        if ( indexFileName == "" ) {
            cerr<< "An index file name must be specified\n";
            return EXIT_FAILURE;
        }
        cout << "[INFO] Loading index "<<indexFileName << endl;
        nfidx.loadIndex(indexFileName);
        nfidx.info();
        return EXIT_SUCCESS;
    }

    if ( shouldDump ) {
        if ( indexFileName == "" ) {
            cerr<< "An index file name must be specified\n";
            return EXIT_FAILURE;
        }
        nfidx.loadIndex(indexFileName);
        nfidx.dumpIndex();
        return EXIT_SUCCESS;
    }

    if ( shouldCreate ) {
        if ( indexFileName  == "" ) {
            cerr << "An index file name must be specified\n";
            return EXIT_FAILURE;
        }

        if ( rootDirectory != "") {
            nfidx.bulkImport(rootDirectory, indexFileName);
            return EXIT_SUCCESS;
        }

        if ( nfcapdFileName == "" ) {
            cerr << "An nfcapd file name must be specified to create an index\n";
            return EXIT_FAILURE;
        }

        nfidx.createIndex(nfcapdFileName);
        nfidx.storeIndex(indexFileName);
    }

    return EXIT_SUCCESS;
}
