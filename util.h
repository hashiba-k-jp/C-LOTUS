#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

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
struct Path{
    vector<variant<ASNumber, Itself>> path;
};
std::ostream& operator<<(std::ostream& os, Itself itself) {
    os << "I";
    return os;
}

bool operator==(const variant<ASNumber, Itself>& lhs, const variant<ASNumber, Itself>& rhs) {
    return visit([](auto&& lhs_val, auto&& rhs_val) -> bool {
        using T1 = std::decay_t<decltype(lhs_val)>;
        using T2 = std::decay_t<decltype(rhs_val)>;
        if constexpr (std::is_same_v<T1, T2>) {
            return lhs_val == rhs_val;
        } else {
            return false;
        }
    }, lhs, rhs);
}
void print_path(const variant<ASNumber, Itself>& v) {
    std::visit([](auto&& arg) {
        std::cout << arg;
    }, v);
}

struct Message{
    MessageType type;
    ASNumber src, dst;
    IPAddress address;
    vector<Path> path;
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

#endif
