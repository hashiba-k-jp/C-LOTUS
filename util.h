#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

#include <yaml-cpp/yaml.h>

using namespace std;

/***
 *** For safety typing
 ***/
// struct ASNumber{ int asn; };
using ASNumber = int;
// struct IPAddress{
//     string address;
//     bool operator<(const IPAddress& other) const{
//         return address < other.address;
//     }
// };
using IPAddress = string;

/***
 *** enum definitions
 ***/
enum class MessageType{ Init, Update };
enum class ConnectionType{ Peer, Down };
enum class ComeFrom{ Customer, Peer, Provider };
enum class DefaultPolicy{ LocPrf, PathLength };
using Policy = DefaultPolicy;
/* // For future extensions:
 * enum class ExtendedPolicyASPA { ASPA };
 * enum class ExtendedPolicyISEC { ISEC };
 * using Policy = std::variant<Policy, ASPA, ISEC>;
 */

namespace YAML{
    template<>
    struct convert<MessageType>{
        static Node encode(const MessageType& msg_type){
            Node node;
            switch(msg_type) {
                case MessageType::Init:   node = "init"; break;
                case MessageType::Update: node = "update"; break;
            }
            return node;
        }
        static bool decode(const Node& node, MessageType& msg_type){
            if(!node.IsScalar()){
                return false;
            }
            string s = node.as<string>();
            if(s == "init"){
                msg_type = MessageType::Init;
            }else if(s == "update"){
                msg_type = MessageType::Update;
            }else{
                return false;
            }
            return true;
        }
    };

    template<>
    struct convert<ConnectionType>{
        static Node encode(const ConnectionType& c_type){
            Node node;
            switch(c_type) {
                case ConnectionType::Peer: node = "peer"; break;
                case ConnectionType::Down: node = "down"; break;
            }
            return node;
        }
        static bool decode(const Node& node, ConnectionType& c_type){
            if(!node.IsScalar()){
                return false;
            }
            string s = node.as<string>();
            if(s == "peer"){
                c_type = ConnectionType::Peer;
            }else if(s == "down"){
                c_type = ConnectionType::Down;
            }else{
                return false;
            }
            return true;
        }
    };

    template<>
    struct convert<ComeFrom>{
        static Node encode(const ComeFrom& come_from){
            Node node;
            switch(come_from) {
                case ComeFrom::Provider: node = "provider"; break;
                case ComeFrom::Peer:     node = "peer"; break;
                case ComeFrom::Customer: node = "customer"; break;
            }
            return node;
        }
        static bool decode(const Node& node, ComeFrom& come_from){
            if(!node.IsScalar()){
                return false;
            }
            string s = node.as<string>();
            if(s == "provider"){
                come_from = ComeFrom::Provider;
            }else if(s == "peer"){
                come_from = ComeFrom::Peer;
            }else if(s == "customer"){
                come_from = ComeFrom::Customer;
            }else{
                return false;
            }
            return true;
        }
    };

    template<>
    struct convert<Policy>{
        static Node encode(const Policy& policy){
            Node node;
            switch(policy) {
                case Policy::LocPrf:     node = "LocPrf"; break;
                case Policy::PathLength: node = "PathLength"; break;
            }
            return node;
        }
        static bool decode(const Node& node, Policy& policy){
            if(!node.IsScalar()){
                return false;
            }
            string s = node.as<string>();
            if(s == "LocPrf"){
                policy = Policy::LocPrf;
            }else if(s == "PathLength"){
                policy = Policy::PathLength;
            }else{
                return false;
            }
            return true;
        }
    };

    template<>
    struct convert<vector<Policy>> {
        static Node encode(const vector<Policy>& policies) {
            Node node;
            for (const auto& p : policies) {
                node.push_back(p);
            }
            return node;
        }
        static bool decode(const Node& node, std::vector<Policy>& policy_list) {
            if(!node.IsSequence())
                return false;
            for (const auto& n : node) {
                policy_list.push_back(n.as<Policy>());
            }
            return true;
        }
    };

}

std::ostream& operator<<(std::ostream& os, MessageType mt) {
    switch (mt) {
        case MessageType::Init: os << "Init"; break;
        case MessageType::Update: os << "Update"; break;
        default: os << "Unknown"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ConnectionType ct) {
    switch (ct) {
        case ConnectionType::Peer: os << "Peer"; break;
        case ConnectionType::Down: os << "Down"; break;
        default: os << "Unknown"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ComeFrom cf) {
    switch (cf) {
        case ComeFrom::Customer: os << "Customer"; break;
        case ComeFrom::Peer    : os << "Peer";     break;
        case ComeFrom::Provider: os << "Provider"; break;
        default: os << "Unknown"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, DefaultPolicy policy) {
    switch (policy) {
        case DefaultPolicy::LocPrf:
            os << "LocPrf";
            break;
        case DefaultPolicy::PathLength:
            os << "PathLength";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}


/***
 *** data structures
 ***/
enum class Itself{ I };
using Path = vector<variant<ASNumber, Itself>>;
std::ostream& operator<<(std::ostream& os, Itself itself) {
    os << "I";
    return os;
}

ostream& operator<<(ostream& os, const variant<ASNumber, Itself>& v) {
    visit([&os](auto&& arg){
        using T = decay_t<decltype(arg)>;
        if constexpr (is_same_v<T, ASNumber>) { os << arg; }
        else if constexpr (is_same_v<T, Itself>) { os << "I"; }
    }, v);
    return os;
}

vector<variant<ASNumber, Itself>> ITSELF_VEC = {Itself::I};
bool operator==(const variant<ASNumber, Itself>& lhs, const variant<ASNumber, Itself>& rhs) {
    return visit([](const auto& lhs_val, const auto& rhs_val) -> bool {
        using T1 = decay_t<decltype(lhs_val)>;
        using T2 = decay_t<decltype(rhs_val)>;
        if constexpr (is_same_v<T1, T2>) {
            return lhs_val == rhs_val;
        } else {
            return false;
        }
    }, lhs, rhs);
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
    // The order of the displayed path and the path in the internal data structure are REVERSED.
    reverse(path.begin(), path.end());
    return path;
}

string string_path(Path path){
    ostringstream oss;
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        oss << *it << "-";
    }
    string string_path = oss.str();
    if(!string_path.empty()){
        string_path.pop_back();
    }

    return string_path;
}

struct Message{
    MessageType type;
    ASNumber src;
    optional<ASNumber> dst;
    optional<IPAddress> address;
    optional<Path> path;
    optional<ComeFrom> come_from;
};

struct Connection{
    ConnectionType type;
    ASNumber src, dst;
};

struct Route{
    Path path;
    ComeFrom come_from;
    int LocPrf;
    bool best_path;
};

struct RouteDiff{
    ComeFrom come_from;
    Path path;
    IPAddress address;
};

#endif
