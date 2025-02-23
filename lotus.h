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

    void add_messages(MessageType msgtype, ASNumber src, optional<ASNumber> dst=nullopt, optional<IPAddress> address=nullopt, optional<Path> path=nullopt){
        if(get_AS(src) == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << src << " has NOT been registered, the message CANNOT be added.\033[00m" << std::endl;
            return;
        }
        if(msgtype == MessageType::Update){
            if(get_AS(*dst) == nullptr){
                std::cout << "\033[33m[WARN] Since AS " << *dst << " has NOT been registered, the message CANNOT be added.\033[00m" << std::endl;
                return;
            }
        }

        message_queue.push(Message{msgtype, src, dst, address, path, nullopt});
        return;
    }

    queue<Message> get_messages(void){
        return message_queue;
    }

    void show_messages(void){
        if(message_queue.size() == 0){
            std::cout << "\033[32m[INFO] No messages in the queue.\033[39m" << '\n';
            return;
        }
        std::cout << "++++++++++++++++++++" << "\n";
        std::cout << "MESSAGES" << "\n";
        queue<Message> tmp_msg_queue = message_queue;
        while(!tmp_msg_queue.empty()){
            const Message& msg = tmp_msg_queue.front();
            if(msg.type == MessageType::Init){
                std::cout << "  + [" << msg.type << "]   src: " << msg.src << '\n';
            }else if(msg.type == MessageType::Update){
                std::cout << "  + [" << msg.type << "] src: " << msg.src << ", dst: " << *msg.dst << ", network: " << *msg.address << ", path: ";
                for(const variant<ASNumber, Itself>& p : msg.path->path){
                    print_path(p);
                    std::cout << "-";
                }
                std::cout << "\b \n";
            }
            tmp_msg_queue.pop();
        }
        std::cout << "++++++++++++++++++++" << "\n";
        return;
    }

    void add_all_init(void){
        for(auto it = as_class_list.class_list.begin(); it != as_class_list.class_list.end(); it++){
            ASNumber as_number = it->second.as_number;
            add_messages(MessageType::Init, as_number);
        }
        return;
    }

    vector<Connection> get_connection_with(ASNumber as_number){
        vector<Connection> connected_with;
        for(const Connection& c : connection_list){
            if(c.src == as_number || c.dst == as_number){
                connected_with.push_back(c);
                continue;
            }
        }
        return connected_with;
    }

    ComeFrom as_a_is_what_on_c(ASNumber as_number, Connection c){
        // E.g. c.type is down, and as_number is the src -> "The AS is the Provider on the connection."
        if(c.type == ConnectionType::Peer){
            return ComeFrom::Peer;
        }else if(c.type == ConnectionType::Down){
            if(as_number == c.src){
                return ComeFrom::Provider;
            }else if(as_number == c.dst){
                return ComeFrom::Customer;
            }
        }
        // ERROR
    }

    void run(void){
        while(!message_queue.empty()){
            Message& msg = message_queue.front();
            if(msg.type == MessageType::Init){
                for(const Connection& c : get_connection_with(msg.src)){
                    msg.come_from = as_a_is_what_on_c(msg.src, c);
                    ASNumber receive_as;
                    if(msg.src == c.src){
                        receive_as = c.dst;
                    }else /* msg.src == c.dst */{
                        receive_as = c.src;
                    }
                    vector<Message> new_update_message_list = as_class_list.get_AS(receive_as)->receive_init(msg);
                    for(const Message& new_update_msg : new_update_message_list){
                        message_queue.push(new_update_msg);
                    }
                }
            }else if(msg.type == MessageType::Update){
                ASClass* as_class = get_AS(*msg.dst);
                vector<Connection> connection_with_dst = get_connection_with(*msg.dst);
                optional<Connection> connection = nullopt;
                for(const Connection& c : connection_with_dst){
                    if(msg.src == c.src || msg.src == c.dst){
                        connection = c;
                        break;
                    }
                }
                if(connection == nullopt){return; /* assert False */}

                msg.come_from = as_a_is_what_on_c(msg.src, *connection);
                optional<RouteDiff> route_diff = as_class->update(msg);
                if(route_diff == nullopt){
                    // continue;
                }else if(route_diff->come_from == ComeFrom::Customer){
                    for(const Connection& c : connection_with_dst){
                        Message new_update_message;
                        new_update_message.type = MessageType::Update;
                        new_update_message.src = *msg.dst;
                        new_update_message.path = route_diff->path;
                        new_update_message.address = route_diff->address;
                        if(c.src == *msg.dst){
                            new_update_message.dst = c.dst;
                        }else if(c.dst == *msg.dst){
                            new_update_message.dst = c.src;
                        }
                        message_queue.push(new_update_message);
                    }
                }else if(route_diff->come_from == ComeFrom::Peer || route_diff->come_from == ComeFrom::Provider){
                    for(const Connection& c : connection_with_dst){
                        if(c.type == ConnectionType::Down && c.src == *msg.dst){
                            Message new_update_message;
                            new_update_message.type = MessageType::Update;
                            new_update_message.src = *msg.dst;
                            new_update_message.dst = c.dst;
                            new_update_message.path = route_diff->path;
                            new_update_message.address = route_diff->address;
                            message_queue.push(new_update_message);
                        }
                    }
                }
            }
            message_queue.pop();
        }
        return;
    }
};
