#include "as_class.hpp"

ASClass::ASClass(ASNumber as_number, IPAddress address, vector<Policy> policy, optional<RoutingTable> given_routing_table){
    this->as_number = as_number;
    this->network_address = address;
    this->policy = policy;
    if(given_routing_table == nullopt){
        this->routing_table = RoutingTable{policy, address};
    }else{
        this->routing_table = *given_routing_table;
    }
}

void ASClass::show_route(const Route* r){
    if(r->best_path){
        std::cout << "  \033[32m>\033[39m ";
    }else{
        std::cout << "    ";
    }
    std::cout << "\033[1mLocPrf:\033[0m "    << std::setw(4) << r->LocPrf << ", ";
    std::cout << "\033[1mcome_from\033[0m: " << std::setw(8) << r->come_from << ", ";
    if(r->aspv != nullopt){
        std::cout << "\033[1mASPV\033[0m: "      << std::setw(7) << r->aspv.value() << ", ";
    }else if(r->aspv == nullopt){
        std::cout << "\033[1mASPV\033[0m: "      << "-------" << ", ";
    }
    if(r->isec_v != nullopt){
        std::cout << "\033[1mIsec\033[0m: "      << std::setw(7) << r->isec_v.value() << ", ";
    }else if(r->isec_v == nullopt){
        std::cout << "\033[1mIsec\033[0m: "      << "-------" << ", ";
    }

    std::cout << "\033[1mpath\033[0m: "      << string_path(r->path) << "\n";
    return;
}

void ASClass::show_AS(void){
    std::cout << "====================" << "\n";
    std::cout << "\033[1mAS NUMBER\033[0m : \033[36m" << as_number << "\033[39m\n";
    std::cout << "\033[1mnetwork\033[0m   : \033[36m" << network_address << "\033[39m\n";
    std::cout << "\033[1mpolicy\033[0m    : \033[36m";
    for(const Policy& p : policy){
        std::cout << p << " ";
    }
    std::cout << "\033[39m\n";

    std::cout << "routing table: (best path: \033[32m>\033[39m )" << "\n";
    for(auto it = routing_table.table.begin(); it != routing_table.table.end(); it++){
        std::cout << "  " << it->first << "\n";
        for(const Route* r : it->second){
            show_route(r);
        }
    }
    std::cout << "====================" << "\n";

    return;
}

vector<Message> ASClass::receive_init(Message init_msg){
    // "init_msg" has only the members "type" and "src".
    map<IPAddress, const Route*> best_route_list = routing_table.get_best_route_list();
    vector<Message> new_update_message_list;
    ASNumber update_src = as_number;
    ASNumber update_dst = init_msg.src;

    if(*init_msg.come_from == ComeFrom::Customer){
        for(auto it = best_route_list.begin(); it != best_route_list.end(); it++){
            IPAddress address = it->first;
            const Route* r = it->second;
            Message new_update_message;
            if(r->path == ITSELF_VEC){
                new_update_message = Message{MessageType::Update, update_src, update_dst, address, Path{{update_src}}, nullopt};
            }else{
                Path p = r->path;
                p.push_back(update_src);
                new_update_message = Message{MessageType::Update, update_src, update_dst, address, p, nullopt};
            }
            new_update_message_list.push_back(new_update_message);
        }
    }else if(*init_msg.come_from == ComeFrom::Peer || *init_msg.come_from == ComeFrom::Provider){
        for(auto it = best_route_list.begin(); it != best_route_list.end(); it++){
            IPAddress address = it->first;
            const Route* r = it->second;
            Message new_update_message;
            if(r->come_from == ComeFrom::Customer){
                if(r->path == ITSELF_VEC){
                    new_update_message = Message{MessageType::Update, update_src, update_dst, address, Path{{update_src}}, nullopt};
                }else{
                    Path p = r->path;
                    p.push_back(update_src);
                    new_update_message = Message{MessageType::Update, update_src, update_dst, address, p, nullopt};
                }
            }
            new_update_message_list.push_back(new_update_message);
        }
    }
    return new_update_message_list;
}

optional<RouteDiff> ASClass::update(Message update_msg){
    for(const variant<ASNumber, Itself>& as_on_path : *update_msg.path){
        if(as_number == as_on_path){
            return nullopt;
        }
    }
    optional<RouteDiff> route_diff = routing_table.update(update_msg);
    if(route_diff == nullopt){
        return nullopt;
    }else{
        route_diff->path.push_back(as_number);
        return route_diff;
    }
}

void ASClass::change_policy(bool onoff, Policy p, int priority){
    if(onoff /* == true */){
        policy.insert(policy.begin() + (priority - 1), p);
    }else{
        auto erase_p = find(policy.begin(), policy.end(), p);
        policy.erase(erase_p);
    }
    routing_table.policy = policy;
}


IPAddressGenerator::IPAddressGenerator(int index){
    this->index = index;
}

IPAddress IPAddressGenerator::get_unique_address(void){
    index += 1;
    return IPAddress{"10." + to_string(index/256) + "." + to_string(index%256) + ".0/24"};
}


ASClassList::ASClassList(int index){
    this->class_list = {};
    this->ip_gen = IPAddressGenerator(index);
}

ASClass* ASClassList::get_AS(ASNumber asn){
    auto it = class_list.find(asn);
    if (it != class_list.end()) {
        return &it->second;
    }
    return nullptr;
}

void ASClassList::add_AS(ASNumber asn){
    if(get_AS(asn) == nullptr){
        IPAddress address = ip_gen.get_unique_address();
        this->class_list[asn] = ASClass{asn, address};
    }else{
        std::cout << asn << " has been already exists" << std::endl;
    }
    return;
}

void ASClassList::show_AS(ASNumber asn){
    ASClass* as_class = get_AS(asn);
    if(as_class != nullptr){
        as_class->show_AS();
    }else{
        std::cout << "\033[33m[WARN] AS " << asn << " has NOT been registered.\033[00m" << std::endl;
    }
    return;
}

void ASClassList::show_AS_list(void){
    for(auto it = class_list.begin(); it != class_list.end(); it++){
        ASClass as_class = it->second;
        as_class.show_AS();
    }
    return;
}
