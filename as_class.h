#ifndef AS_CLASS_H
#define AS_CLASS_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include "util.h"
#include "routing_table.h"

using namespace std;

class IPAddressGenerator{
protected:
    int index = 1;

public:
    IPAddress get_unique_address(void){
        index += 1;
        return IPAddress{"10." + to_string(index/256) + "." + to_string(index%256) + ".0/24"};
    }
};

class ASClass{
protected:
    const ASNumber as_number_;
    const IPAddress network_address_;
    vector<Policy> policy;
    RoutingTable routing_table;

public:
    ASClass(ASClass as_number, IPAddress address);
};

class ASCLassList{
protected:
    map<ASNumber, ASClass> class_list;
    IPAddressGenerator ip_gen;
};


#endif
