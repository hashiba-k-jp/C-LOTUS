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

#include "util.hpp"
#include "data_struct.hpp"
#include "security.hpp"

class RouteSecurity{
public:
    optional<ASPV> aspv;
    optional<Isec> isec_v;
};

class IRoute{
public:
    virtual void show_route(void) const = 0;
    virtual ~IRoute() = default;
};

class Route : public IRoute{
public:
    Path path;
    ComeFrom come_from;
    int LocPrf;
    bool best_path;
    RouteSecurity route_sec;

public:
    Route(Path path, ComeFrom come_from, int LocPrf, bool best_path, RouteSecurity route_sec)
    : path(path), come_from(come_from), LocPrf(LocPrf), best_path(best_path), route_sec(route_sec) {}
    void show_route(void) const override;
};

class IRoutingTable{
public:
    virtual void show_table(void) const = 0;
    virtual void add_policy(Policy new_policy, int priority) = 0;
    virtual ~IRoutingTable() = default;
};

class RoutingTable : public IRoutingTable{
public:
    map<IPAddress, vector<Route*>> table;
    vector<Policy> policy;
    map<ASNumber, vector<ASNumber>> public_aspa_list;
    vector<ASNumber> isec_adopted_as_list;
    map<ASNumber, vector<ASNumber>> public_ProConID;
    shared_ptr<ISecurityManager> sec_manager;

public:
    RoutingTable(map<IPAddress, vector<Route*>> table = {}, vector<Policy> policy={Policy::LocPrf, Policy::PathLength})
    : table(table), policy(policy) {}
    void show_table(void) const override;
    void add_policy(Policy new_policy, int priority) override;
    map<IPAddress, const Route*> get_best_route_list(void);
    ASPV verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider);
    ASPV aspv(const Route r, ASNumber neighbor_as);
    optional<Isec> isec_v(const Route r, const Message update_msg);
    void new_route_security_validation(Route* route, Message update_msg);
    optional<RouteDiff> update(Message update_msg);
};

#endif
