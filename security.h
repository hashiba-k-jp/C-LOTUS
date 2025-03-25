#ifndef SECURITY_H
#define SECURITY_H

#define POLICY X(LocPrf) X(PathLength) X(Aspa) X(Isec)

enum class Policy{
    #define X(name) name,
    POLICY
    #undef X
};

ostream& operator<<(std::ostream& os, Policy value) {
    switch (value) {
        #define X(name) case Policy::name: os << #name; break;
        POLICY
        #undef X
    }
    return os;
}

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

#include "policies/aspa.h"
#include "policies/isec_procon_id.h"

class RPKI{
public:
    map<ASNumber, vector<ASNumber>> public_aspa_list;
    vector<ASNumber> isec_adopted_as_list;
    map<ASNumber, vector<ASNumber>> public_ProConID;
};

#endif
