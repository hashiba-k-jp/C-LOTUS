#ifndef AS_CLASS_H
#define AS_CLASS_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include "util.h"
#include "routing_table.h"

using namespace std;

class IPAddressGenerator{
public:
    int index = 0;

public:
    IPAddressGenerator(int index=0){
        this->index = index;
    }

    IPAddress get_unique_address(void){
        index += 1;
        return IPAddress{"10." + to_string(index/256) + "." + to_string(index%256) + ".0/24"};
    }
};

class ASClass{
public:
    ASNumber as_number;
    IPAddress network_address;
    vector<Policy> policy;
    RoutingTable routing_table;

public:
    ASClass() {}
    ASClass(ASNumber as_number, IPAddress address, vector<Policy> policy={Policy::LocPrf, Policy::PathLength}, optional<RoutingTable> given_routing_table=nullopt){
        this->as_number = as_number;
        this->network_address = address;
        this->policy = {Policy::LocPrf, Policy::PathLength};
        if(given_routing_table == nullopt){
            this->routing_table = RoutingTable{policy, address};
        }else{
            this->routing_table = *given_routing_table;
        }
    }

    /* OVERRIDE THIS FUNCTION TO SHOW SECURITY POLICIES */
    void show_route(const Route r){
        if(r.best_path){
            std::cout << "  \033[32m>\033[39m ";
        }else{
            std::cout << "    ";
        }
        std::cout << "\033[1mLocPrf:\033[0m "    << std::setw(4) << r.LocPrf << ", ";
        std::cout << "\033[1mcome_from\033[0m: " << std::setw(7) << r.come_from << ", ";
        std::cout << "\033[1mpath\033[0m: "      << string_path(r.path) << "\n";
        // std::cout << "\033[1mpath\033[0m:"       << r.security_policy << "\n";
        return;
    }

    void show_AS(void){
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
            for(const Route& r : it->second){
                show_route(r);
            }
        }
        std::cout << "====================" << "\n";

        return;
    }

    vector<Message> receive_init(Message init_msg){
        // "init_msg" has only the members "type" and "src".
        map<IPAddress, Route> best_route_list = routing_table.get_best_route_list();
        vector<Message> new_update_message_list;
        ASNumber update_src = as_number;
        ASNumber update_dst = init_msg.src;

        if(*init_msg.come_from == ComeFrom::Customer){
            for(auto it = best_route_list.begin(); it != best_route_list.end(); it++){
                IPAddress address = it->first;
                Route r = it->second;
                Message new_update_message;
                if(r.path == ITSELF_VEC){
                    new_update_message = Message{MessageType::Update, update_src, update_dst, address, Path{{update_src}}, nullopt};
                }else{
                    Path p = r.path;
                    p.push_back(update_src);
                    new_update_message = Message{MessageType::Update, update_src, update_dst, address, p, nullopt};
                }
                new_update_message_list.push_back(new_update_message);
            }
        }else if(*init_msg.come_from == ComeFrom::Peer || *init_msg.come_from == ComeFrom::Provider){
            for(auto it = best_route_list.begin(); it != best_route_list.end(); it++){
                IPAddress address = it->first;
                Route r = it->second;
                Message new_update_message;
                if(r.come_from == ComeFrom::Customer){
                    if(r.path == ITSELF_VEC){
                        new_update_message = Message{MessageType::Update, update_src, update_dst, address, Path{{update_src}}, nullopt};
                    }else{
                        Path p = r.path;
                        p.push_back(update_src);
                        new_update_message = Message{MessageType::Update, update_src, update_dst, address, p, nullopt};
                    }
                }
                new_update_message_list.push_back(new_update_message);
            }
        }
        return new_update_message_list;
    }

    optional<RouteDiff> update(Message update_msg){
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
};


class ASClassList{
public:
    IPAddressGenerator ip_gen = IPAddressGenerator{};
    map<ASNumber, ASClass> class_list = {};

public:
    ASClassList(int index=0){
        this->class_list = {};
        this->ip_gen = IPAddressGenerator(index);
    }

    ASClass* get_AS(ASNumber asn){
        auto it = class_list.find(asn);
        if (it != class_list.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void add_AS(ASNumber asn){
        if(get_AS(asn) == nullptr){
            IPAddress address = ip_gen.get_unique_address();
            this->class_list[asn] = ASClass{asn, address};
        }else{
            std::cout << asn << " has been already exists" << std::endl;
        }
        return;
    }

    void show_AS(ASNumber asn){
        ASClass* as_class = get_AS(asn);
        if(as_class != nullptr){
            as_class->show_AS();
        }else{
            std::cout << "\033[33m[WARN] AS " << asn << " has NOT been registered.\033[00m" << std::endl;
        }
        return;
    }

    void show_AS_list(void){
        for(auto it = class_list.begin(); it != class_list.end(); it++){
            ASClass as_class = it->second;
            as_class.show_AS();
        }

        return;
    }
};

namespace YAML{
    template<>
    struct convert<ASClass> {
        static Node encode(const ASClass& as_class) {
            Node node;
            node["AS"]              = as_class.as_number;
            node["network_address"] = as_class.network_address;
            node["policy"]          = as_class.policy;
            node["routing_table"]   = as_class.routing_table;
            return node;
        }
        static bool decode(const Node& node, ASClass& as_class) {
            if(!node.IsSequence()){
                return false;
            }
            as_class.as_number       = node["AS"].as<ASNumber>();
            as_class.network_address = node["network_address"].as<IPAddress>();
            as_class.policy          = node["policy"].as<vector<Policy>>();
            as_class.routing_table   = node["routing_table"].as<RoutingTable>();
            as_class.routing_table.policy = as_class.policy;
            return true;
        }
    };

    template<>
    struct convert<ASClassList> {
        static Node encode(const ASClassList& as_class_list) {
            Node node;
            for(auto it = as_class_list.class_list.begin(); it != as_class_list.class_list.end(); it++){
                node["AS_list"].push_back(it->second);
            }
            node["IP_gen_seed"] = as_class_list.ip_gen.index;
            return node;
        }
        static bool decode(const Node& node, ASClassList& as_class_list) {
            if(!node.IsSequence()){
                return false;
            }
            for(const auto& as_class : node["AS_list"]){
                as_class_list.class_list[as_class["AS"].as<ASNumber>()] = as_class.as<ASClass>();
            }
            as_class_list.ip_gen = IPAddressGenerator{node["IP_gen_seed"].as<int>()};
            return true;
        }
    };
}

#endif
