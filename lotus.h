#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <variant>

#include <yaml-cpp/yaml.h>

using namespace std;
using namespace YAML;

#include "util.h"
#include "data_struct.h"
#include "security.h"
#include "routing_table.h"
#include "as_class.h"
#include "util_convert.h"

const vector<string> SPINNER = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

class LOTUS{
protected:
    queue<Message> message_queue;
    vector<Connection> connection_list;

public:
    SecurityProtocol security_protocol;
    ASClassList as_class_list;

public:
    void add_AS(ASNumber asn){
        if(asn == 0){
            std::cout << "\033[33m[WARN] Since AS " << asn << " is the special AS number, the AS was NOT added.\033[00m" << std::endl;
            return;
        }else if(as_class_list.class_list.count(asn)){
            std::cout << "\033[33m[WARN] Since AS " << asn << " already exists, the AS was NOT added.\033[00m" << std::endl;
            return;
        }else{
            as_class_list.add_AS(asn);
            return;
        }
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
        const Connection new_connection = Connection{type, src, dst};
        if(contains(connection_list, new_connection)){
            std::cout << "\033[33m[WARN] Attempted to add a duplicate connection {type = " << type << ", src = " << src << ", dst = " << dst << "}. Ignoring.\033[00m" << std::endl;
            return;
        }
        connection_list.push_back(new_connection);
        return;
    }

    vector<Connection> get_connection(void){
        return connection_list;
    }

    void show_connection(void){
        std::cout << "********************" << "\n";
        std::cout << "CONNECTIONS" << "\n";
        for(const Connection& c : connection_list){
            std::cout << "  * \033[1msrc\033[0m: " << c.src << ", \033[1mdst\033[0m: " << c.dst << ", \033[1mtype\033[0m: " << c.type << "\n";
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

    optional<Path> get_best_path_to(ASNumber origin_as_number, ASNumber destination_as_number){
        // return the best path from <origin_as_number> to <destination_as_number> if exists, otherwise nullopt.
        ASClass* origin_as_class = get_AS(origin_as_number);
        ASClass* destination_as_class = get_AS(destination_as_number);
        if(origin_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << origin_as_number << " has NOT been registered.\033[00m" << std::endl;
            return nullopt;
        }
        if(destination_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << destination_as_number << " has NOT been registered.\033[00m" << std::endl;
            return nullopt;
        }
        IPAddress destination_network = destination_as_class->network_address;
        if(origin_as_class->routing_table.table.find(destination_network) != origin_as_class->routing_table.table.end()){
            for(Route* r : origin_as_class->routing_table.table[destination_network]){
                if(r->best_path){
                    return r->path;
                }
            }
        }
        return nullopt;
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
                std::cout << "  + \033[1m[" << msg.type << "]\033[0m   \033[1msrc\033[0m: " << msg.src << '\n';
            }else if(msg.type == MessageType::Update){
                std::cout << "  + \033[1m[" << msg.type << "]\033[0m \033[1msrc\033[0m: " << msg.src << ", \033[1mdst\033[0m: " << *msg.dst << ", \033[1mnetwork\033[0m: " << *msg.address << ", \033[1mpath\033[0m: " << string_path(*msg.path) << "\n";
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
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    void run(bool print_progress=false){
        // Set ASPA to the routing table of all AS classes.
        for(auto it = as_class_list.class_list.begin(); it != as_class_list.class_list.end(); it++){
            it->second.routing_table.security_protocol = security_protocol;
        }
        int processed_msg_num = 0;
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
            if(print_progress){
                processed_msg_num++;
                std::cout << "\r\033[32m" << SPINNER[(processed_msg_num/2000)%10] << " Running LOTUS, " << std::right << std::setw(8) << processed_msg_num << " finished, " << std::right << std::setw(8) << message_queue.size() << " left.\033[00m" << std::flush;
            }
        }
        if(print_progress){
            std::cout << '\n';
        }
        return;
    }

    void file_import(string file_path, bool overwrite=true){
        // If overwrite is true,
        //   - ALL AS, MESSAGES, CONNECTIONS MADE BEFORE IMPORTING WILL BE REMOVED.
        // If false,
        //   - ONLY AS (AS_NUMBER AND POLICY) (except for security objects) WILL BE ADDED.
        //   - routing_table, network_address, messages, and connection will be discarded.
        //   - ASPA for newly imported AS will be imported, but providers not newly imported will be discarded.
        //   - Adoption of BGP-iSec will be imported only for newly imported AS.
        //   - All ProConID will be discarded. (use add_ProConID_all())
        ifstream file(file_path);
        string line;
        if(file){
            try{
                std::cout << "\033[32m[INFO] Parsing \"" << file_path << "\".\033[00m" << std::endl;

                YAML::Node imported = YAML::Load(file);
                vector<ASNumber> imported_as;

                /* AS LIST */
                if(overwrite){
                    as_class_list = ASClassList(imported["IP_gen_seed"].as<int>());
                    for(const auto& as_node : imported["AS_list"]){
                        ASNumber as_number = as_node["AS"].as<ASNumber>();
                        RoutingTable routing_table = as_node["routing_table"].as<RoutingTable>();
                        vector<Policy> policy = as_node["policy"].as<vector<Policy>>();
                        routing_table.policy = policy;
                        as_class_list.class_list[as_number] = ASClass{
                            as_number,
                            as_node["network_address"].as<IPAddress>(),
                            policy,
                            routing_table
                        };
                    }
                }else{
                    for(const auto& as_node : imported["AS_list"]){
                        ASNumber as_number = as_node["AS"].as<ASNumber>();
                        if(as_class_list.class_list.count(as_number)){
                            continue;
                        }
                        add_AS(as_number);
                        imported_as.push_back(as_number);
                        ASClass* new_as_class = get_AS(as_number);
                        new_as_class->policy = as_node["policy"].as<vector<Policy>>();
                    }
                }

                /* CONNECTION LIST */
                if(overwrite){
                    connection_list = {};
                    for(const auto& c_node : imported["connection"]){
                        ASNumber src = c_node["src"].as<ASNumber>();
                        ASNumber dst = c_node["dst"].as<ASNumber>();
                        ConnectionType type = c_node["type"].as<ConnectionType>();
                        connection_list.push_back(Connection{type, src, dst});
                    }
                }

                /* MESSAGES LIST */
                if(overwrite){
                    message_queue = {};
                    for(const auto& m_node : imported["message"]){
                        ASNumber src = m_node["src"].as<ASNumber>();
                        MessageType msgtype = m_node["type"].as<MessageType>();
                        if(msgtype == MessageType::Init){
                            add_messages(msgtype, src);
                        }else if(msgtype == MessageType::Update){
                            ASNumber dst = m_node["dst"].as<ASNumber>();
                            IPAddress address = m_node["network"].as<IPAddress>();
                            Path path = parse_path(m_node["path"].as<string>());
                            add_messages(msgtype, src, dst, address, path);
                        }
                    }
                }

                /* ASPA */
                if(overwrite){
                    security_protocol.RPKI.public_aspa_list = {};
                    for(const auto& aspa_node : imported["ASPA"]){
                        ASNumber customer = aspa_node.first.as<ASNumber>();
                        for(const auto& provider : aspa_node.second){
                            security_protocol.RPKI.public_aspa_list[customer].push_back(provider.as<ASNumber>());
                        }
                    }
                }else{
                    for(const auto& aspa_node : imported["ASPA"]){
                        ASNumber customer = aspa_node.first.as<ASNumber>();
                        if(contains(imported_as, customer)){
                            for(const auto& provider : aspa_node.second){
                                ASNumber provider_as = provider.as<ASNumber>();
                                if(contains(imported_as, provider_as)){
                                    security_protocol.RPKI.public_aspa_list[customer].push_back(provider_as);
                                }
                            }
                        }
                    }
                }

                /* BGP-iSec */
                if(overwrite){
                    security_protocol.RPKI.isec_adopted_as_list = {};
                    for(const auto& as_it : imported["security_protocol.RPKI.isec_adopted_as_list"]){
                        security_protocol.RPKI.isec_adopted_as_list.push_back(as_it.as<ASNumber>());
                    }
                }else{
                    for(const auto& as_it : imported["security_protocol.RPKI.isec_adopted_as_list"]){
                        ASNumber as_number = as_it.as<ASNumber>();
                        if(contains(imported_as, as_number)){
                            security_protocol.RPKI.isec_adopted_as_list.push_back(as_number);
                        }
                    }
                }

                if(overwrite){
                    security_protocol.RPKI.public_ProConID = {};
                    for(const auto& isec_node : imported["security_protocol.RPKI.public_ProConID"]){
                        ASNumber customer = isec_node.first.as<ASNumber>();
                        for(const auto& provider : isec_node.second){
                            security_protocol.RPKI.public_ProConID[customer].push_back(provider.as<ASNumber>());
                        }
                    }
                }

            }catch(YAML::ParserException &e){
                std::cout << "\033[33m[WARN] The file \"" << file_path << "\" is INVALID as a yaml file.\033[00m" << std::endl;
                std::cerr << "\033[33m       " << e.what() << "\033[00m\n";
            }
        }else{
            std::cout << "\033[33m[WARN] The file \"" << file_path << "\" does NOT exist.\033[00m" << std::endl;
        }
        return;
    }

    void file_export(string file_path_string){
        YAML::Node export_data;

        filesystem::path file_path(file_path_string);

        // if (filesystem::exists(file_path)) {
        //     std::cout << "\033[33m[WARN] The file \"" << file_path_string << "\" is already exist.\033[00m\n";
        //     std::cout << "Are you sure to overwrite? (y/n) ";
        //     string user_input;
        //     cin >> user_input;
        //     std::transform(user_input.begin(), user_input.end(), user_input.begin(), ::tolower);
        //     if(!(user_input == "y" || user_input == "yes")){
        //         std::cerr << "\033[32m[INFO] Canceled. The file was not overwritten.\033[00m\n";
        //         return;
        //     }
        // }

        /* AS LIST */
        for(const auto& it : YAML::convert<ASClassList>::encode(as_class_list)){
            export_data[it.first] = it.second;
        }
        /* CONNECTION LIST */
        export_data["connection"] = connection_list;

        /* MESSAGES LIST */
        export_data["message"] = message_queue;

        /* SECURITY OBJECTS */
        export_data["ASPA"] = security_protocol.RPKI.public_aspa_list;
        export_data["RPKI.isec_adopted_as_list"] = security_protocol.RPKI.isec_adopted_as_list;
        export_data["RPKI.public_ProConID"] = security_protocol.RPKI.public_ProConID;

        std::ofstream fout(file_path_string);
        if (!fout) {
            std::cerr << "\033[33m[WARN] Failed to open the file \"" << file_path_string << "\" for writing.\033[00m\n";
            return;
        }

        YAML::Emitter out;
        out.SetIndent(1);
        out << export_data;
        fout << out.c_str();
        fout.close();

        return;
    }

    void gen_attack(ASNumber src, ASNumber target){
        if(as_class_list.class_list.find(src) == as_class_list.class_list.end()){
            std::cout << "\033[33m[WARN] Since AS " << src << " has NOT been registered, no attack has been generated.\033[00m" << std::endl;
            return;
        }
        ASClass* target_as_class = get_AS(target);
        if(target_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << target << " has NOT been registered, no attack has been generated.\033[00m" << std::endl;
            return;
        }

        vector<Connection> src_connection_list = get_connection_with(src);
        vector<ASNumber> adj_as_list;
        for(const auto& c : src_connection_list){
            if(src == c.src){
                adj_as_list.push_back(c.dst);
            }else if(src == c.dst){
                adj_as_list.push_back(c.src);
            }
        }

        IPAddress target_address = target_as_class->network_address;
        Path attack_path = Path{target, src};

        for(const ASNumber& adj_as : adj_as_list){
            add_messages(MessageType::Update, src, adj_as, target_address, attack_path);
        }
        return;
    }


    // SECURITY OBJECTS
    void add_ASPA(ASNumber customer, vector<ASNumber> provider_list){
        security_protocol.RPKI.public_aspa_list[customer] = provider_list;
    }

    void auto_ASPA(ASNumber origin_customer, int hop_num){
        ASClass* customer_as_class = get_AS(origin_customer);
        if(customer_as_class == nullptr){
            std::cout << "\033[33m[WARN] Since AS " << origin_customer << " has NOT been registered, the ASPA CANNOT be added.\033[00m" << std::endl;
            return;
        }
        vector<ASNumber> customer_as_list = {origin_customer};

        while(hop_num != 0 && customer_as_list.size() != 0){
            vector<ASNumber> next_customer_as_list = {};
            for(ASNumber& customer : customer_as_list){
                vector<Connection> c_list = get_connection_with(customer);
                vector<ASNumber> provider_list = {};
                for(const Connection& connection : c_list){
                    if(as_a_is_what_on_c(customer, connection) == ComeFrom::Customer){
                        provider_list.push_back(connection.src);
                    }
                }
                next_customer_as_list.insert(next_customer_as_list.end(), provider_list.begin(), provider_list.end());
                if(provider_list.size() == 0){
                    security_protocol.RPKI.public_aspa_list[customer] = {0};
                }else{
                    security_protocol.RPKI.public_aspa_list[customer] = provider_list;
                }
            }
            hop_num -= 1;
            sort(customer_as_list.begin(), customer_as_list.end());
            sort(next_customer_as_list.begin(), next_customer_as_list.end());
            set_union(customer_as_list.begin(), customer_as_list.end(), next_customer_as_list.begin(), next_customer_as_list.end(), back_inserter(customer_as_list));
        }
    }

    void set_ASPV(ASNumber as_number, bool onoff, int priority){
        get_AS(as_number)->change_policy(onoff, Policy::Aspa, priority);
    }

    void show_ASPA_list(void){
        std::cout << "--------------------" << "\n";
        std::cout << "ASPA" << '\n';
        for(auto it = security_protocol.RPKI.public_aspa_list.begin(); it != security_protocol.RPKI.public_aspa_list.end(); it++){
            std::cout << "  - \033[1mcustomer\033[0m : " << it->first;
            std::cout << ", \033[1mASPA\033[0m : " << it->second << "\n";
        }
        std::cout << "--------------------" << "\n";
    }

    void switch_adoption_iSec(ASNumber as_number, bool onoff, int priority){
        if(onoff){
            if(find(security_protocol.RPKI.isec_adopted_as_list.begin(), security_protocol.RPKI.isec_adopted_as_list.end(), as_number) == security_protocol.RPKI.isec_adopted_as_list.end()){
                security_protocol.RPKI.isec_adopted_as_list.push_back(as_number);
                get_AS(as_number)->change_policy(onoff, Policy::Isec, priority);
            }else{
                std::cout << "\033[33m[WARN] The AS " << as_number << " has already published its adoption of BGP-iSec.\033[00m" << std::endl;
            }
        }else{
            auto it = remove(security_protocol.RPKI.isec_adopted_as_list.begin(), security_protocol.RPKI.isec_adopted_as_list.end(), as_number);
            if (it != security_protocol.RPKI.isec_adopted_as_list.end()) {
                security_protocol.RPKI.isec_adopted_as_list.erase(it, security_protocol.RPKI.isec_adopted_as_list.end());
                get_AS(as_number)->change_policy(onoff, Policy::Isec, priority);
            } else {
                std::cout << "\033[33m[WARN] The AS " << as_number << " has not adopted BGP-iSec.\033[00m" << std::endl;
            }
        }
        return;
    }

    void add_ProConID_all(void){
        for(const ASNumber as_number : security_protocol.RPKI.isec_adopted_as_list){
            vector<ASNumber> ProConID_list = {};
            vector<ASNumber> provider_list = {as_number};
            vector<ASNumber> checked_list = {};

            while(0 < provider_list.size()){
                ASNumber customer = provider_list.front();
                provider_list.erase(provider_list.begin());
                if(contains(checked_list, customer)){
                    continue;
                }
                checked_list.push_back(customer);

                vector<ASNumber> next_provider_list = {};

                vector<Connection> c_list = get_connection_with(customer);
                for(const Connection& connection : c_list){
                    if(as_a_is_what_on_c(customer, connection) == ComeFrom::Customer){
                        next_provider_list.push_back(connection.src);
                    }
                }

                for(auto it = next_provider_list.begin(); it != next_provider_list.end();){
                    if(contains(ProConID_list, *it) || contains(checked_list, *it)){
                        it = next_provider_list.erase(it);
                        continue;
                    }
                    if(find(security_protocol.RPKI.isec_adopted_as_list.begin(), security_protocol.RPKI.isec_adopted_as_list.end(), *it) != security_protocol.RPKI.isec_adopted_as_list.end()){
                        ProConID_list.push_back(*it);
                        it = next_provider_list.erase(it);
                    }else{
                        ++it;
                    }
                }

                // here, next_provider_list is the list of NON adopting provider as.
                sort(next_provider_list.begin(), next_provider_list.end());
                sort(provider_list.begin(), provider_list.end());
                set_union(provider_list.begin(), provider_list.end(), next_provider_list.begin(), next_provider_list.end(), back_inserter(provider_list));
                provider_list.erase(unique(provider_list.begin(), provider_list.end()), provider_list.end());
            }
            security_protocol.RPKI.public_ProConID[as_number] = ProConID_list;
        }
        return;
    }

    void show_isec_adopting(void){
        std::cout << "--------------------" << "\n";
        std::cout << "BGP-iSec Adopting AS" << "\n  ";
        sort(security_protocol.RPKI.isec_adopted_as_list.begin(), security_protocol.RPKI.isec_adopted_as_list.end());
        for(const ASNumber as_number : security_protocol.RPKI.isec_adopted_as_list){
            std::cout << as_number << ", ";
        }
        std::cout << "\b\b\x1b[K\n--------------------\n";
        return;
    }

    void show_ProConID_list(void){
        std::cout << "--------------------" << "\n";
        std::cout << "ProConID" << '\n';
        for(auto it = security_protocol.RPKI.public_ProConID.begin(); it != security_protocol.RPKI.public_ProConID.end(); it++){
            std::cout << "  - \033[1mcustomer\033[0m : " << it->first;
            std::cout << ", \033[1mProConID\033[0m : " << it->second << "\n";
        }
        std::cout << "--------------------" << "\n";
        return;
    }
};
