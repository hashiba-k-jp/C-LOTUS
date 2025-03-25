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
bool contains(const std::vector<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

#endif
