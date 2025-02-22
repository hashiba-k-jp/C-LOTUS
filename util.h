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
struct IPAddress{
    string address;
    bool operator<(const IPAddress& other) const{
        return address < other.address;
    }
};

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


/***
 *** data structures
 ***/
enum class Itself{ I };
struct Path{
    vector<variant<int, Itself>> path;
};

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
