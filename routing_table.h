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
public:
    map<IPAddress, vector<Route>> table;
    vector<Policy> policy;

public:
    RoutingTable() {}
    RoutingTable(vector<Policy> policy, const IPAddress network){
        this->policy = policy;
        table[network] = {Route{Path{{Itself::I}}, ComeFrom::Customer, 1000, true}};
    }
};


#endif
