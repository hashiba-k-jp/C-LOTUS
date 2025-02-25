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

    map<IPAddress, Route> get_best_route_list(void){
        map<IPAddress, Route> best_route_list;

        for(auto it = table.begin(); it != table.end(); it++){
            IPAddress address = it->first;
            vector<Route> route_list = it->second;
            for(const Route& r : route_list){
                if(r.best_path){
                    best_route_list[address] = r;
                }
            }
        }

        return best_route_list;
    }

    /* OVERRIDE THIS FUNCTION TO ADD SECURITY POLICIES */
    pair<bool, optional<RouteDiff>> update_by_security_policy(Policy p, Route new_route, Route best){
        // If the best route is not decided, return the pair of **TRUE** and any RouteDiff (RouteDiff should discarded in call function).
        // otherwise, return the pair of false and RouteDiff (nullopt is acceptable if no RouteDiff).
        // ** true means "continue" in call function.
        switch(p) {
            // case Policy::SomeSecurityPolicy: // may define another function to handle security policy.
            //     break;
            default:
                // If all Policy are not covered in this switch-case (except for defalut Policy::LocPrf and Policy::PathLength),
                // this program raise error as follows.
                throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
        }
        return {false, nullopt};
    }

    /* OVERRIDE THIS FUNCTION TO ADD SECURITY POLICIES */
    /* BE SURE THAT THE ROUTE STRUCTURE HAS CORRESPONDING MEMBER */
    Route new_route_security_validation(Route r, Message update_msg){
        // if(contains(r.policy, Policy::SomeSecurityPolicy1)){
        //     Route["<security_policy1>"] = some_security_validation_function1(r);
        // }
        // if(contains(r.policy, Policy::SomeSecurityPolicy2)){
        //     Route["<security_policy2>"] = some_security_validation_function2(r);
        // }
        // ...
        return r;
    }

    bool security_check(Route r){
        // switch(some_security_validation_function(r)){
        //     case "Invalid":
        //         return false;
        //     default:
        //         return true;
        // }
        return true;
    }


    optional<RouteDiff> update(Message update_msg){
        IPAddress network  = *update_msg.address;
        Path path          = *update_msg.path;
        ComeFrom come_from = *update_msg.come_from;
        int LocPrf;
        switch(come_from){
            case ComeFrom::Customer: LocPrf = 200; break;
            case ComeFrom::Peer:     LocPrf = 100; break;
            case ComeFrom::Provider: LocPrf = 50;  break;
        }
        Route new_route = Route{path, come_from, LocPrf, false};

        new_route = new_route_security_validation(new_route, update_msg);

        if(table.count(network) > 0){ /* when the network already has several routes. */
            Route* best = nullptr;
            for(Route& r : table[network]){
                if(r.best_path){
                    best = &r;
                    break;
                }
            }
            if(best == nullptr){
                /* raise BestPathNotExist */
                if(!security_check(new_route)){
                    new_route.best_path = false;
                    return nullopt;
                }else{
                    new_route.best_path = true;
                    table[network].push_back(new_route);
                    return RouteDiff{come_from, path, network};
                }
            }else{
                for(const Policy& p : policy){
                    int new_length, best_length;
                    switch(p) {
                        case Policy::LocPrf:
                            if(new_route.LocPrf > best->LocPrf){
                                new_route.best_path = true;
                                best->best_path = false;
                                table[network].push_back(new_route);
                                return RouteDiff{new_route.come_from, new_route.path, network};
                            }else if(new_route.LocPrf == best->LocPrf){
                                continue;
                            }else if(new_route.LocPrf < best->LocPrf){
                                table[network].push_back(new_route);
                                return nullopt;
                            }
                            break;
                        case Policy::PathLength:
                            new_length = new_route.path.size();
                            best_length = best->path.size();
                            if(new_length < best_length){
                                new_route.best_path = true;
                                best->best_path = false;
                                table[network].push_back(new_route);
                                return RouteDiff{new_route.come_from, new_route.path, network};
                            }else if(new_length == best_length){
                                continue;
                            }else if(new_length > best_length){
                                table[network].push_back(new_route);
                                return nullopt;
                            }
                            break;
                        default: /* security policies */
                            pair<bool, optional<RouteDiff>> security_policy_res = update_by_security_policy(p, new_route, *best);
                            if(security_policy_res.first){
                                 continue;
                            }else{
                                 return security_policy_res.second;
                            }
                            break;
                    }
                }
            }
        }else{ /* when the network DOES NOT HAVE any routes. */
            // SECURITY CHECK;
            if(security_check(new_route)){
                new_route.best_path = true;
                table[network].push_back(new_route);
                return RouteDiff{come_from, path, network};
            }else{
                new_route.best_path = false;
                table[network].push_back(new_route);
                return nullopt;
            }
        }
        return nullopt; /* assert False*/
    }
};

namespace YAML{
    template<>
    struct convert<RoutingTable>{
        static Node encode(const RoutingTable& routing_table){
            Node node;
            // node["policy"]    = routing_table.policy;
            for(const auto& it : routing_table.table){
                node[it.first] = it.second;
            }
            return node;
        };
        static bool decode(const Node& node, RoutingTable& routing_table){
            if(!node.IsScalar()){
                return false;
            }
            routing_table.table = node["routing_table"].as<map<IPAddress, vector<Route>>>();
            return true;
        }
    };
}

#endif
