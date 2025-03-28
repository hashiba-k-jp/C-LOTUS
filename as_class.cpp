#include "as_class.hpp"

ASClass::ASClass(ASNumber as_number, IPAddress address, vector<Policy> policy, optional<RoutingTable> given_routing_table){
    this->as_number = as_number;
    this->network_address = address;
    if(given_routing_table == nullopt){
        this->routing_table = RoutingTable{policy, address};
    }else{
        this->routing_table = *given_routing_table;
    }
}

void ASClass::show_AS(void){
    std::cout << "====================" << "\n";
    std::cout << "\033[1mAS NUMBER\033[0m : \033[36m" << as_number << "\033[39m\n";
    std::cout << "\033[1mnetwork\033[0m   : \033[36m" << network_address << "\033[39m\n";
    routing_table.show_table();
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

void ASClass::add_policy(Policy new_policy, int priority){
    routing_table.add_policy(new_policy, priority);
    return;
}
