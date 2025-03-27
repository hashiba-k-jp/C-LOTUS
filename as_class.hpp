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

#include "util.h"
#include "data_struct.h"
#include "routing_table.h"

class ASClass{
public:
    ASNumber as_number;
    IPAddress network_address;
    vector<Policy> policy;
    RoutingTable routing_table;

public:
    ASClass() {}
    ASClass(ASNumber as_number, IPAddress address, vector<Policy> policy={Policy::LocPrf, Policy::PathLength}, optional<RoutingTable> given_routing_table=nullopt);
    void show_route(const Route* r);
    void show_AS(void);
    vector<Message> receive_init(Message init_msg);
    optional<RouteDiff> update(Message update_msg);
    void change_policy(bool onoff, Policy p, int priority);
};

class IPAddressGenerator{
public:
    int index = 0;
public:
    IPAddressGenerator(int index=0);
    IPAddress get_unique_address(void);
};

class ASClassList{
public:
    IPAddressGenerator ip_gen = IPAddressGenerator{};
    map<ASNumber, ASClass> class_list = {};
public:
    ASClassList(int index=0);
    ASClass* get_AS(ASNumber asn);
    void add_AS(ASNumber asn);
    void show_AS(ASNumber asn);
    void show_AS_list(void);
};

#endif
