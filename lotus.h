#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "util.h"
#include "as_class.h"

using namespace std;

class LOTUS{
protected:
    ASCLassList as_class_list;
    queue<Message> message_queue;
    vector<Connection> connection_list;
    map<ASNumber, vector<ASNumber>> public_aspa_list;

public:
    void add_AS(ASNumber asn){
        as_class_list.add_AS(asn);
        return;
    }

    ASClass* get_AS(ASNumber asn){
        return as_class_list.get_AS(asn);
    }

    void show_AS(ASNumber asn){
        as_class_list.show_AS(asn);
        return;
    }

    void show_AS_list(void){
        as_class_list.show_AS_list();
        return;
    }

    void add_connection(ConnectionType type, ASNumber src, ASNumber dst){
        ASClass* src_as_class = get_AS(src);
        ASClass* dst_as_class = get_AS(dst);
        if(src_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << src << " has NOT been registered, the connection CANNOT be added.\033[00m" << std::endl;
            return;
        }
        if(dst_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << dst << " has NOT been registered, the connection CANNOT be added.\033[00m" << std::endl;
            return;
        }

        connection_list.push_back(Connection{type, src, dst});
        return;
    }

    vector<Connection> get_connection(void){
        return connection_list;
    }

    void show_connection(void){
        std::cout << "********************" << "\n";
        std::cout << "CONNECTIONS" << "\n";
        for(const Connection& c : connection_list){
            std::cout << "  * src: " << c.src << ", dst: " << c.dst << ", type: " << c.type << "\n";
        }
        std::cout << "********************" << "\n";
        return;
    }
};
