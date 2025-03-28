#ifndef AS_CLASS_H
#define AS_CLASS_H

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

#include "util.hpp"
#include "data_struct.hpp"
#include "routing_table.hpp"

class ASClass{
public:
    ASNumber as_number;
    IPAddress network_address;
    vector<Policy> policy;
    RoutingTable routing_table;

public:
    ASClass() {}
    ASClass(ASNumber as_number, IPAddress address, vector<Policy> policy={Policy::LocPrf, Policy::PathLength}, optional<RoutingTable> given_routing_table=nullopt);
    void show_AS(void);
    vector<Message> receive_init(Message init_msg);
    optional<RouteDiff> update(Message update_msg);
    void add_policy(Policy new_policy, int priority);
};

#endif
