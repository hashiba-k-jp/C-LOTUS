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
    map<ASNumber, vector<ASNumber>> public_aspa_list;

public:
    RoutingTable() {}
    RoutingTable(vector<Policy> policy, const IPAddress network){
        this->policy = policy;
        table[network] = {Route{Path{{Itself::I}}, ComeFrom::Customer, 1000, true, nullopt}};
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

    ASPV verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider){
        auto it = public_aspa_list.find(get<ASNumber>(customer));

        if(it == public_aspa_list.end()){
            return ASPV::Unknown;
        }else{
            const vector<ASNumber>& provider_list = it->second;
            if(find(provider_list.begin(), provider_list.end(), get<ASNumber>(provider)) != provider_list.end()){
                return ASPV::Valid;
            }else{
                return ASPV::Invalid;
            }
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    ASPV aspv(const Route r, ASNumber neighbor_as){
        // The last node of the path of the route from another AS MUST NOT be Itself::I,
        // thus comparing only to ASNumber is enough.

        // Note: (I-D [https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/])
        // If there are no hops or just one hop between the apexes of the up-ramp and the down-ramp, then the AS_PATH is valid (valley free).

        if(const ASNumber* p = get_if<ASNumber>(&r.path.back()); p && *p != neighbor_as){
            return ASPV::Invalid;
        }
        ASPV semi_state = ASPV::Valid;
        ASPV pair_check;
        switch(r.come_from){
            case ComeFrom::Customer:
            case ComeFrom::Peer:
                for(size_t i = 0; i < r.path.size() - 1; ++i){
                    pair_check = verify_pair(r.path[i], r.path[i+1]);
                    if(pair_check == ASPV::Invalid){
                        return ASPV::Invalid;
                    }else if(pair_check == ASPV::Unknown){
                        semi_state = ASPV::Unknown;
                    }
                }
                return semi_state;
            case ComeFrom::Provider:
                bool upflow_fragment = true;
                for(size_t i = 0; (upflow_fragment&&i<r.path.size()-1)||(!upflow_fragment&&i<r.path.size()); ++i){
                    if(upflow_fragment){
                        // r.path.size() <= i+1, IndexError
                        pair_check = verify_pair(r.path[i], r.path[i+1]);
                        if(pair_check == ASPV::Invalid){
                            upflow_fragment = false;
                        }else if(pair_check == ASPV::Unknown){
                            semi_state = ASPV::Unknown;
                        }
                    }else if(upflow_fragment == false){
                        // if r.path.size() <= i, IndexError.
                        pair_check = verify_pair(r.path[i], r.path[i-1]);
                        if(pair_check == ASPV::Invalid){
                            return ASPV::Invalid;
                        }else if(pair_check == ASPV::Unknown){
                            semi_state = ASPV::Unknown;
                        }
                    }
                }
                return semi_state;
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    void new_route_security_validation(Route* route, Message update_msg){
        route->aspv = aspv(*route, update_msg.src);
        // other security function should be added here.
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
        Route new_route = Route{path, come_from, LocPrf, false, nullopt};

        new_route_security_validation(&new_route, update_msg);

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
                if(policy.front() == Policy::Aspa && new_route.aspv == ASPV::Invalid){
                    new_route.best_path = false;
                    return nullopt;
                }else{
                    new_route.best_path = true;
                    table[network].push_back(new_route);
                    return RouteDiff{come_from, path, network};
                }
            }else{
                int new_length, best_length;
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
                        case Policy::Aspa:
                            if(new_route.aspv == ASPV::Invalid){
                                return nullopt;
                            }
                            break;
                        default:
                            throw logic_error("\n\033[31m[ERROR] Invalid Policy type: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
                            break;
                    }
                }
            }
        }else{ /* when the network DOES NOT HAVE any routes. */
            // SECURITY CHECK;
            if(policy.front() == Policy::Aspa && new_route.aspv == ASPV::Invalid){
                new_route.best_path = false;
                table[network].push_back(new_route);
                return nullopt;
            }else{
                new_route.best_path = true;
                table[network].push_back(new_route);
                return RouteDiff{come_from, path, network};
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
