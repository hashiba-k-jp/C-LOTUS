#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

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

    bool operator==(const Connection& other) const{
        if(type == ConnectionType::Peer && other.type == ConnectionType::Peer){
            return ((src == other.src && dst == other.dst) || (src == other.dst && dst == other.src));
        }else{
            return type == other.type && src == other.src && dst == other.dst;
        }
    }
};

struct Route{
    Path path;
    ComeFrom come_from;
    int LocPrf;
    bool best_path;
    optional<ASPV> aspv;
    optional<Isec> isec_v;
};

struct RouteDiff{
    ComeFrom come_from;
    Path path;
    IPAddress address;
};

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
struct convert<Route*>{
    static Node encode(const Route* r){
        Node node;
        node["path"]      = string_path(r->path);
        node["come_from"] = r->come_from;
        node["LocPrf"]    = r->LocPrf;
        node["best_path"] = r->best_path;
        node["aspv"]      = r->aspv;
        node["isec_v"]    = r->isec_v;
        return node;
    };
    static bool decode(const Node& node, Route*& r){
        if(!node.IsMap()){
            return false;
        }
        optional<ASPV> aspv = nullopt;
        optional<Isec> isec_v;
        if(node["aspv"] && !node["aspv"].IsNull()){
            aspv = node["aspv"].as<ASPV>();
        }
        if(node["isec_v"] && !node["isec_v"].IsNull()){
            isec_v = node["isec_v"].as<Isec>();
        }
        r = new Route{
            parse_path(node["path"].as<string>()),
            node["come_from"].as<ComeFrom>(),
            node["LocPrf"].as<int>(),
            node["best_path"].as<bool>(),
            aspv,
            isec_v
        };
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

namespace YAML{
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

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
}

template <typename T>
bool contains(const std::vector<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const optional<T>& val) {
    if(val == nullopt){
        os << "Null";
    }else{
        os << val;
    }
    return os;
}

#endif
