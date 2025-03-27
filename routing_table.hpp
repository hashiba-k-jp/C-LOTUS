#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <variant>

#include <yaml-cpp/yaml.h>
using namespace std;
using namespace YAML;

#include "util.h"
#include "data_struct.h"

class RoutingTable{
public:
    map<IPAddress, vector<Route*>> table;
    vector<Policy> policy;
    map<ASNumber, vector<ASNumber>> public_aspa_list;
    vector<ASNumber> isec_adopted_as_list;
    map<ASNumber, vector<ASNumber>> public_ProConID;

public:
    RoutingTable() {}
    RoutingTable(vector<Policy> policy, const IPAddress network);
    map<IPAddress, const Route*> get_best_route_list(void);
    ASPV verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider);
    ASPV aspv(const Route r, ASNumber neighbor_as);
    optional<Isec> isec_v(const Route r, const Message update_msg);
    void new_route_security_validation(Route* route, Message update_msg);
    optional<RouteDiff> update(Message update_msg);
};

#endif
