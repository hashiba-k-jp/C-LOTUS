#include "routing_table.hpp"

void Route::show_route(void) const{
    if(best_path){
        std::cout << "  \033[32m>\033[39m ";
    }else{
        std::cout << "    ";
    }
    std::cout << "\033[1mLocPrf:\033[0m "    << std::setw(4) << LocPrf << ", ";
    std::cout << "\033[1mcome_from\033[0m: " << std::setw(8) << come_from << ", ";
    if(route_sec.aspv != nullopt){
        std::cout << "\033[1mASPV\033[0m: "      << std::setw(7) << route_sec.aspv.value() << ", ";
    }else if(route_sec.aspv == nullopt){
        std::cout << "\033[1mASPV\033[0m: "      << "-------" << ", ";
    }
    if(route_sec.isec_v != nullopt){
        std::cout << "\033[1mIsec\033[0m: "      << std::setw(7) << route_sec.isec_v.value() << ", ";
    }else if(route_sec.isec_v == nullopt){
        std::cout << "\033[1mIsec\033[0m: "      << "-------" << ", ";
    }

    std::cout << "\033[1mpath\033[0m: "      << string_path(path) << "\n";
    return;
}

void RoutingTable::show_table(void) const{
    cout << "routing table: (best path: \033[32m>\033[39m )\n";
    for(auto it = table.begin(); it != table.end(); it++){
        std::cout << "  " << it->first << "\n";
        for(const Route* r : it->second){
            r->show_route();
        }
    }
}

RoutingTable::RoutingTable(vector<Policy> policy, const IPAddress network){
    this->policy = policy;
    table[network] = {new Route{Path{{Itself::I}}, ComeFrom::Customer, 1000, true, RouteSecurity{nullopt, nullopt}}};
}

map<IPAddress, const Route*> RoutingTable::get_best_route_list(void){
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

ASPV RoutingTable::verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider){
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

ASPV RoutingTable::aspv(const Route r, ASNumber neighbor_as){
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

optional<Isec> RoutingTable::isec_v(const Route r, const Message update_msg){
    // REFERENCE
    // C. Morris, A. Herzberg, B. Wang, and S. Secondo,
    // "BGP-iSec: Improved Security of Internet Routing Against Post-ROV Attacks",
    // in USENIX Network and Distributed System Security (NDSS) Symposium, 2024.
    // https://dx.doi.org/10.14722/ndss.2024.241035

    // Origin = X0 -> X1 -> ... -> Xl -> Y = *update_msg.dst
    // update_msg.path = {X0, X1, ..., Xl}, and Y is not included.

    if(update_msg.type == MessageType::Init){
        return nullopt;
    }

    // if the AS Y is not adopted AS, iSec should not evaluated.
    if(!contains(isec_adopted_as_list, *update_msg.dst)){
        return nullopt;
    }

    // If the origin AS does not adopted, iSec should not evaluated.
    if(!contains(isec_adopted_as_list, get<ASNumber>((*update_msg.path).front()))){
        return nullopt;
    }

    if(update_msg.come_from == ComeFrom::Provider){
        return Isec::Valid;
    }else{
        vector<ASNumber> adopted_path_as = {};
        for(const auto as_number : *update_msg.path){
            // The type of as_number MUST be ASNumber
            if(contains(isec_adopted_as_list, get<ASNumber>(as_number))){
                adopted_path_as.push_back(get<ASNumber>(as_number));
            }
        }
        int i = 0;
        while(i < static_cast<int>(size(adopted_path_as)) - 1){
            if(!contains(public_ProConID[adopted_path_as[i]], adopted_path_as[i+1])){
                return Isec::Invalid;
            }
            ++i;
        }
        if(update_msg.come_from == ComeFrom::Peer){
            return Isec::Valid;
        }else if(update_msg.come_from == ComeFrom::Customer){
            if(contains(public_ProConID[adopted_path_as.back()], *update_msg.dst)){
                return Isec::Valid;
            }else{
                return Isec::Invalid;
            }
        }
    }
    throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
}

void RoutingTable::new_route_security_validation(Route* route, Message update_msg){
    route->route_sec.aspv = aspv(*route, update_msg.src);
    route->route_sec.isec_v = isec_v(*route, update_msg);
    // other security function should be added here.
}

optional<RouteDiff> RoutingTable::update(Message update_msg){
    IPAddress network  = *update_msg.address;
    Path path          = *update_msg.path;
    ComeFrom come_from = *update_msg.come_from;
    int LocPrf;
    switch(come_from){
        case ComeFrom::Customer: LocPrf = 200; break;
        case ComeFrom::Peer:     LocPrf = 100; break;
        case ComeFrom::Provider: LocPrf = 50;  break;
    }
    Route* new_route = new Route{path, come_from, LocPrf, false, RouteSecurity{nullopt, nullopt}};

    new_route_security_validation(new_route, update_msg);

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
            /* raise BestPathNotExist */
            if(policy.front() == Policy::Aspa && new_route->route_sec.aspv == ASPV::Invalid){
                new_route->best_path = false;
                return nullopt;
            }else{
                new_route->best_path = true;
                return RouteDiff{come_from, path, network};
            }
        }else{
            int new_length, best_length;
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
                    case Policy::Aspa:
                        if(new_route->route_sec.aspv == ASPV::Invalid){
                            return nullopt;
                        }
                        break;
                    case Policy::Isec:
                        if(new_route->route_sec.isec_v == Isec::Invalid){
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
        if(contains(policy, Policy::Aspa) && new_route->route_sec.aspv == ASPV::Invalid){
            new_route->best_path = false;
            table[network].push_back(new_route);
            return nullopt;
        }
        if(contains(policy, Policy::Isec) && new_route->route_sec.isec_v == Isec::Invalid){
            new_route->best_path = false;
            table[network].push_back(new_route);
            return nullopt;
        }
        new_route->best_path = true;
        table[network].push_back(new_route);
        return RouteDiff{come_from, path, network};
    }
    return nullopt;
}
