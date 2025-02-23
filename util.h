#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

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
