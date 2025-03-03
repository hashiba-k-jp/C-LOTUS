#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <variant>

#include <yaml-cpp/yaml.h>

using namespace std;

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    return std::equal(str1.begin(), str1.end(), str2.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

/***
 *** For safety typing
 ***/
using ASNumber = int;
using IPAddress = string;

/***
 *** enum definitions
 ***/
#define MESSAGE_TYPE X(Init) X(Update)
#define CONNECTION_TYPE X(Peer) X(Down)
#define COMEFROM X(Customer) X(Peer) X(Provider)
#define POLICY X(LocPrf) X(PathLength) X(Aspa)

#define CREATE_ENUM_CLASS(ClassName, EnumValues) \
enum class ClassName{ \
    EnumValues\
};
#define X(name) name,
CREATE_ENUM_CLASS(MessageType, MESSAGE_TYPE)
CREATE_ENUM_CLASS(ConnectionType, CONNECTION_TYPE)
CREATE_ENUM_CLASS(ComeFrom, COMEFROM)
CREATE_ENUM_CLASS(Policy, POLICY)
#undef X

#define OPERATOR_COUT(ClassName, EnumValues)\
std::ostream& operator<<(std::ostream& os, ClassName value) {\
    switch (value) {\
        EnumValues\
    }\
    return os;\
}

#define X(name) case MessageType::name: os << #name; break;
OPERATOR_COUT(MessageType, MESSAGE_TYPE)
#define X(name) case ConnectionType::name: os << #name; break;
OPERATOR_COUT(ConnectionType, CONNECTION_TYPE)
#define X(name) case ComeFrom::name: os << #name; break;
OPERATOR_COUT(ComeFrom, COMEFROM)
#define X(name) case Policy::name: os << #name; break;
OPERATOR_COUT(Policy, POLICY)
#undef X

namespace YAML{
    template<>
    struct convert<MessageType>{
        static Node encode(const MessageType& msg_type){
            Node node;
            switch(msg_type) {
                #define X(name) case MessageType::name: node = #name; break;
                MESSAGE_TYPE
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, MessageType& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = MessageType::name; return true; }
            MESSAGE_TYPE
            #undef X
            return false;
        }
    };

    template<>
    struct convert<ConnectionType>{
        static Node encode(const ConnectionType& c_type){
            Node node;
            switch(c_type) {
                #define X(name) case ConnectionType::name: node = #name; break;
                CONNECTION_TYPE
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, ConnectionType& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ConnectionType::name; return true; }
            CONNECTION_TYPE
            #undef X
            return false;
        }
    };

    template<>
    struct convert<ComeFrom>{
        static Node encode(const ComeFrom& come_from){
            Node node;
            switch(come_from) {
                #define X(name) case ComeFrom::name: node = #name; break;
                COMEFROM
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, ComeFrom& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ComeFrom::name; return true; }
            COMEFROM
            #undef X
            return true;
        }
    };

    template<>
    struct convert<Policy>{
        static Node encode(const Policy& policy){
            Node node;
            switch(policy) {
                #define X(name) case Policy::name: node = #name; break;
                POLICY
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, Policy& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = Policy::name; return true; }
            POLICY
            #undef X
            return false;
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
            if(!node.IsSequence()){ return false; }
            for (const auto& n : node) {
                policy_list.push_back(n.as<Policy>());
            }
            return true;
        }
    };
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
        if(caseInsensitiveCompare(as_string, "I")){
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

namespace YAML{

    /* STRUCTS */
    template<>
    struct convert<Message>{
        static Node encode(const Message& msg){
            Node node;
            node["type"] = msg.type;
            node["src"]  = msg.src;
            if(msg.type == MessageType::Update){
                node["dst"]       = *msg.dst;
                node["network"]   = *msg.address;
                node["path"]      = string_path(*msg.path);
                node["come_from"] = *msg.come_from;
            }
            return node;
        }
        static bool decode(const Node& node, Message& msg){
            if(!node.IsScalar()){
                return false;
            }
            msg.type = node["type"].as<MessageType>();
            msg.src  = node["src"].as<ASNumber>();
            if(node["type"].as<MessageType>() == MessageType::Update){
                msg.dst       = node["dst"].as<ASNumber>();
                msg.address   = node["network"].as<IPAddress>();
                msg.path      = parse_path(node["path"].as<string>());
                msg.come_from = node["come_from"].as<ComeFrom>();
            }
            return true;
        }
    };

    template<>
    struct convert<Connection>{
        static Node encode(const Connection& c){
            Node node;
            node["dst"]  = c.dst;
            node["src"]  = c.src;
            node["type"] = c.type;
            return node;
        }
        static bool decode(const Node& node, Connection& c){
            if(!node.IsScalar()){
                return false;
            }
            c.src  = node["src"].as<ASNumber>();
            c.dst  = node["dst"].as<ASNumber>();
            c.type = node["type"].as<ConnectionType>();
            return true;
        }
    };

    template<>
    struct convert<Route>{
        static Node encode(const Route& r){
            Node node;
            node["path"]      = string_path(r.path);
            node["come_from"] = r.come_from;
            node["LocPrf"]    = r.LocPrf;
            node["best_path"] = r.best_path;
            return node;
        };
        static bool decode(const Node& node, Route& r){
            if(!node.IsScalar()){
                return false;
            }
            r.path      = parse_path(node["path"].as<string>());
            r.come_from = node["come_from"].as<ComeFrom>();
            r.LocPrf    = node["LocPrf"].as<int>();
            r.best_path = node["best_path"].as<bool>();
            return true;
        }
    };

    /* VECTOR, MAP,  */

    template<typename Type>
    struct convert<vector<Type>>{
        static Node encode(const vector<Type>& vector_t){
            Node node(NodeType::Sequence);
            for(const auto& entry : vector_t){
                node.push_back(entry);
            }
            return node;
        }
        static bool decode(const Node& node, vector<Type>& vector_t){
            if(!node.IsMap()){
                return false;
            }
            for(const auto& entry : node){
                vector_t.push_back(entry.as<Type>());
            }
            return true;
        }
    };

    template<typename Type>
    struct convert<queue<Type>>{
        static Node encode(const queue<Type>& queue_t){
            Node node(NodeType::Sequence);
            queue<Type> tmp_queue = queue_t;
            while(!tmp_queue.empty()){
                node.push_back(tmp_queue.front());
                tmp_queue.pop();
            }
            return node;
        }
        static bool decode(const Node& node, queue<Type>& queue_t){
            if(!node.IsSequence()){
                return false;
            }
            for(const auto& entry : node){
                queue_t.push(entry.as<Type>());
            }
            return true;
        }
    };

    template<typename Key, typename Value>
    struct convert<map<Key, Value>>{
        static Node encode(const map<Key, Value>& map){
            Node node(NodeType::Map);
            for (const auto& entry : map){
                node[entry.first] = entry.second;
            }
            return node;
        }
        static bool decode(const Node& node, map<Key, Value>& map){
            if(!node.IsMap()){
                return false;
            }
            for(const auto& entry : node){
                map[entry.first.as<Key>()] = entry.second.as<Value>();
            }
            return true;
        }
    };
}

template <typename T, typename... Ts>
bool contains(const std::vector<std::variant<Ts...>>& vec, const T& value) {
    return std::find_if(vec.begin(), vec.end(), [&value](const std::variant<Ts...>& v) {
        return std::visit([&value](auto&& arg) {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, T>) {
                return arg == value;
            }
            return false;
        }, v);
    }) != vec.end();
}

#endif
