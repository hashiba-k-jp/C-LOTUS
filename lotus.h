#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "util.h"
#include "as_class.h"

using namespace std;

class LOTUS{
protected:
    ASCLassList as_class_list;
    queue<Message> message_queue;
    vector<Connection> connection_list;
    map<ASNumber, vector<ASNumber>> public_aspa_list;

    vector<Policy> parse_policy(YAML::Node as_node_policy){
        vector<Policy> policy;
        for(const auto& p : as_node_policy){
            if(p.as<string>() == "LocPrf"){
                policy.push_back(Policy::LocPrf);
            }else if(p.as<string>() == "PathLength"){
                policy.push_back(Policy::PathLength);
            }else{
                optional<Policy> sec_policy = security_policy(p.as<string>());
                if(sec_policy != nullopt){
                    policy.push_back(*sec_policy);
                }
            }
        }
        return policy;
    }

    Path parse_path(string path_string){
        Path path;
        vector<string> as_string_list;
        std::stringstream ss(path_string);
        std::string token;
        while (std::getline(ss, token, '-')) {
            as_string_list.push_back(token);
        }
        for(const string& as_string : as_string_list){
            if(as_string == "i"){
                path.push_back(Itself::I);
            }else{
                path.push_back(ASNumber(stoi(as_string)));
            }
        }
        return path;
    }

    ComeFrom parse_come_from(string come_from_string){
        if(come_from_string == "customer"){
            return ComeFrom::Customer;
        }else if(come_from_string == "provider"){
            return ComeFrom::Provider;
        }else if(come_from_string == "peer"){
            return ComeFrom::Peer;
        }
    }

    bool parse_tf(string tf_string){
        if(tf_string == "true"){
            return true;
        }else if(tf_string == "false"){
            return false;
        }
    }

    ConnectionType parse_type(string c_type_string){
        if(c_type_string == "down"){
            return ConnectionType::Down;
        }else if(c_type_string == "peer"){
            return ConnectionType::Peer;
        }
    }

    MessageType parse_message_type(string m_type_string){
        if(m_type_string == "init"){
            return MessageType::Init;
        }else if(m_type_string == "update"){
            return MessageType::Update;
        }
    }

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
                std::cout << "  + [" << msg.type << "] src: " << msg.src << ", dst: " << *msg.dst << ", network: " << *msg.address << ", path: " << string_path(*msg.path) << "\n";
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

    /* OVERRIDE THIS FUNCTION (and type if necessary) TO ADD SECURITY OBJECTS */
    virtual void parse_security_objects(YAML::Node node){
        return;
    }

    /* OVERRIDE THIS FUNCTION TO ADD SECURITY POLICIES */
    virtual optional<Policy> security_policy(string policy_string){
        return nullopt;

        /* EXAMPLE */
        // if(policy_string == "ASPA"){
        //     return Policy::ASPA;
        // }else{
        //     return nullopt;
        // }
    }

    void file_import(string file_path){
        ifstream file(file_path);
        string line;
        if(file){
            try{
                std::cout << "\033[32m[INFO] Parsing \"" << file_path << "\".\033[00m" << std::endl;

                YAML::Node imported = YAML::Load(file);

                /* AS LIST */
                int index = imported["IP_gen_seed"].as<int>();
                ASCLassList new_as_class_list{index};

                YAML::Node as_list = imported["AS_list"];
                for(const auto& as_node : as_list){
                    ASNumber as_number = as_node["AS"].as<ASNumber>();

                    IPAddress address = as_node["network_address"].as<IPAddress>();

                    RoutingTable routing_table;
                    for(const auto& r : as_node["routing_table"]){
                        IPAddress route_address = r.first.as<IPAddress>();
                        for(const auto& route : r.second){
                            Path path = parse_path(route["path"].as<string>());
                            ComeFrom come_from = parse_come_from(route["come_from"].as<string>());
                            int LocPrf = route["LocPrf"].as<int>();
                            bool best_path = parse_tf(route["best_path"].as<string>());
                            routing_table.table[route_address].push_back(Route{path, come_from, LocPrf, best_path});
                        }
                    }
                    vector<Policy> policy = parse_policy(as_node["policy"]);
                    routing_table.policy = policy;
                    ASClass new_as = {as_number, address, policy, routing_table};
                    new_as_class_list.class_list[as_number] = new_as;
                }
                as_class_list = new_as_class_list;

                /* CONNECTION LIST */
                connection_list = {};
                for(const auto& c_node : imported["connection"]){
                    ASNumber src = c_node["src"].as<ASNumber>();
                    ASNumber dst = c_node["dst"].as<ASNumber>();
                    ConnectionType type = parse_type(c_node["type"].as<string>());
                    connection_list.push_back(Connection{type, src, dst});
                }

                /* MESSAGES LIST */
                message_queue = {};
                for(const auto& m_node : imported["message"]){
                    ASNumber src = m_node["src"].as<ASNumber>();
                    MessageType msgtype = parse_message_type(m_node["type"].as<string>());
                    if(msgtype == MessageType::Init){
                        add_messages(msgtype, src);
                    }else if(msgtype == MessageType::Update){
                        ASNumber dst = m_node["dst"].as<ASNumber>();
                        IPAddress address = m_node["network"].as<IPAddress>();
                        Path path = parse_path(m_node["path"].as<string>());
                        add_messages(msgtype, src, dst, address, path);
                    }
                }

                /* SECURITY OBJECTS (RPKI) */
                parse_security_objects(imported);

            }catch(YAML::ParserException &e){
                std::cout << "\033[33m[WARN] The file \"" << file_path << "\" is INVALID as a yaml file.\033[00m" << std::endl;
                std::cerr << "\033[33m       " << e.what() << "\033[00m\n";
            }
        }else{
            std::cout << "\033[33m[WARN] The file \"" << file_path << "\" does NOT exist.\033[00m" << std::endl;
        }
        return;
    }

};
