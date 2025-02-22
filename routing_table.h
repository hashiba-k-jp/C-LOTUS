#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H


#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "util.h"


class RoutingTable{
protected:
    map<IPAddress, vector<Route>> table;
    vector<Policy> policy;
    // aspa, isec, and other object should be added to child classes...

public:
    RoutingTable(vector<Policy> policy, const IPAddress network){
        table[network] = Route({Itself::I}, ComeFrom::Customer, 1000, true);
    }
};


#endif
