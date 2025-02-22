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
protected:
    int index = 0;

public:
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
    ASClass(ASNumber as_number, IPAddress address){
        this->as_number = as_number;
        this->network_address = address;
        this->policy = {Policy::LocPrf, Policy::PathLength};
        this->routing_table = RoutingTable{policy, address};
    }

    void show_AS(void){
        std::cout << "====================" << "\n";
        std::cout << "AS NUMBER : \033[36m" << as_number << "\033[39m\n";
        std::cout << "network   : \033[36m" << network_address << "\033[39m\n";
        std::cout << "policy    : \033[36m";
        for(const Policy& p : policy){
            std::cout << p << " ";
        }
        std::cout << "\033[39m\n";

        std::cout << "routing table: (best path: \033[32m>\033[39m )" << "\n";
        for(auto it = routing_table.table.begin(); it != routing_table.table.end(); it++){
            std::cout << "  " << it->first << "\n";
            for(const Route& r : it->second){
                if(r.best_path){
                    std::cout << "  \033[32m>\033[39m ";
                }else{
                    std::cout << "    ";
                }
                std::cout << "path: ";
                for(const variant<ASNumber, Itself>& p : r.path.path){
                    print_path(p);
                    std::cout << "-";
                }
                std::cout << "\b, ";
                std::cout << "LocPrf: " << r.LocPrf << ", ";
                std::cout << "come_from: " << r.come_from << "\n";
            }
        }
        std::cout << "====================" << "\n";

        return;
    }
};

class ASCLassList{
private:
    IPAddressGenerator ip_gen = IPAddressGenerator{};

public:
    map<ASNumber, ASClass> class_list = {};

public:
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


#endif
