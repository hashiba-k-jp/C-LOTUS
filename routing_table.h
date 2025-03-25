#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

class RoutingTable{
public:
    map<IPAddress, vector<Route*>> table;
    vector<Policy> policy;
    SecurityProtocol security_protocol;

public:
    RoutingTable() {}
    RoutingTable(vector<Policy> policy, const IPAddress network){
        this->policy = policy;
        table[network] = {new Route{Path{{Itself::I}}, ComeFrom::Customer, 1000, true, empty_sec_valid()}};
    }

    map<IPAddress, const Route*> get_best_route_list(void){
        map<IPAddress, const Route*> best_route_list;

        for(auto it = table.begin(); it != table.end(); it++){
            IPAddress address = it->first;
            vector<Route*> route_list = it->second;
            for(const Route* r : route_list){
                if(r->best_path){
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

        Route* new_route = new Route{path, come_from, LocPrf, false, empty_sec_valid()};

        security_protocol.new_route_security_validation(new_route, update_msg);

        if(table.count(network) > 0){ /* when the network already has several routes. */
            table[network].push_back(new_route);

            Route* best = nullptr;
            for(Route* r : table[network]){
                if(r->best_path){
                    best = r;
                    break;
                }
            }
            if(best == nullptr){
                /* if the routing table does not have any best path to the dst prefix. */
                if(!security_protocol.check_best_path(policy, new_route->security_valid)){
                    new_route->best_path = false;
                    return nullopt;
                }else{
                    new_route->best_path = true;
                    return RouteDiff{come_from, path, network};
                }
            }else{
                int new_length, best_length;
                if(!security_protocol.check_best_path(policy, new_route->security_valid)){
                    return nullopt;
                }
                for(const Policy& p : policy){
                    switch(p) {
                        case Policy::LocPrf:
                            if(new_route->LocPrf > best->LocPrf){
                                new_route->best_path = true;
                                best->best_path = false;
                                return RouteDiff{new_route->come_from, new_route->path, network};
                            }else if(new_route->LocPrf == best->LocPrf){
                                continue;
                            }else if(new_route->LocPrf < best->LocPrf){
                                return nullopt;
                            }
                            break;
                        case Policy::PathLength:
                            new_length = new_route->path.size();
                            best_length = best->path.size();
                            if(new_length < best_length){
                                new_route->best_path = true;
                                best->best_path = false;
                                return RouteDiff{new_route->come_from, new_route->path, network};
                            }else if(new_length == best_length){
                                continue;
                            }else if(new_length > best_length){
                                return nullopt;
                            }
                            break;
                        default:
                            continue;
                    }
                }
            }
        }else{ /* when the network DOES NOT HAVE any routes. */
            // SECURITY CHECK;
            if(!security_protocol.check_best_path(policy, new_route->security_valid)){
                new_route->best_path = false;
                table[network].push_back(new_route);
                return nullopt;
            }else{
                new_route->best_path = true;
                table[network].push_back(new_route);
                return RouteDiff{come_from, path, network};
            }
        }
        return nullopt;
    }
};

#endif
