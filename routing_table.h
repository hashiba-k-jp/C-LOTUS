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
        if(table.count(network) > 0){ /* when the network already has several routes. */
            optional<Route> best = nullopt;
            for(const Route& r : table[network]){
                if(r.best_path){
                    best = r;
                    break;
                }
            }
            if(best == nullopt){
                /* raise Best PathNotExist */
                new_route.best_path = true;
                table[network].push_back(new_route);
                return nullopt; /* assert False */
            }else{
                for(const Policy& p : policy){
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
                            int new_length = new_route.path.size();
                            int best_length = best->path.size();
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
                    }
                }
            }
        }else{ /* when the network DOES NOT HAVE any routes. */
            new_route.best_path = true;
            table[network] = {new_route};
            return RouteDiff{come_from, path, network};
        }
        return nullopt; /* assert False*/
    }
};


#endif
