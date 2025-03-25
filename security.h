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

struct SecurityValid{
    optional<ASPV> aspv;
    optional<Isec> isec_v;
};

SecurityValid* empty_sec_valid(void){
    return new SecurityValid{nullopt, nullopt};
}

class RPKI{
public:
    map<ASNumber, vector<ASNumber>> public_aspa_list;
    vector<ASNumber> isec_adopted_as_list;
    map<ASNumber, vector<ASNumber>> public_ProConID;

    bool check_best_path(vector<Policy> policy, SecurityValid* security_valid){
        for(const Policy& p : policy){
            switch(p){
                case Policy::Aspa:
                    if(security_valid->aspv == ASPV::Invalid){
                        return false;
                    }
                    continue;
                case Policy::Isec:
                    if(security_valid->isec_v == Isec::Invalid){
                        return false;
                    }
                    continue;
                default:
                    continue;
            }
        }
        return true;
    }

    ASPV verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider){
        auto it = public_aspa_list.find(get<ASNumber>(customer));

        if(it == public_aspa_list.end()){
            return ASPV::Unknown;
        }else{
            const vector<ASNumber>& provider_list = it->second;
            if(find(provider_list.begin(), provider_list.end(), get<ASNumber>(provider)) != provider_list.end()){
                return ASPV::Valid;
            }else{
                return ASPV::Invalid;
            }
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    ASPV aspv(const Route r, ASNumber neighbor_as){
        // The last node of the path of the route from another AS MUST NOT be Itself::I,
        // thus comparing only to ASNumber is enough.

        // Note: (I-D [https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/])
        // If there are no hops or just one hop between the apexes of the up-ramp and the down-ramp, then the AS_PATH is valid (valley free).

        if(const ASNumber* p = get_if<ASNumber>(&r.path.back()); p && *p != neighbor_as){
            return ASPV::Invalid;
        }
        ASPV semi_state = ASPV::Valid;
        ASPV pair_check;
        switch(r.come_from){
            case ComeFrom::Customer:
            case ComeFrom::Peer:
                for(size_t i = 0; i < r.path.size() - 1; ++i){
                    pair_check = verify_pair(r.path[i], r.path[i+1]);
                    if(pair_check == ASPV::Invalid){
                        return ASPV::Invalid;
                    }else if(pair_check == ASPV::Unknown){
                        semi_state = ASPV::Unknown;
                    }
                }
                return semi_state;
            case ComeFrom::Provider:
                bool upflow_fragment = true;
                for(size_t i = 0; (upflow_fragment&&i<r.path.size()-1)||(!upflow_fragment&&i<r.path.size()); ++i){
                    if(upflow_fragment){
                        // r.path.size() <= i+1, IndexError
                        pair_check = verify_pair(r.path[i], r.path[i+1]);
                        if(pair_check == ASPV::Invalid){
                            upflow_fragment = false;
                        }else if(pair_check == ASPV::Unknown){
                            semi_state = ASPV::Unknown;
                        }
                    }else if(upflow_fragment == false){
                        // if r.path.size() <= i, IndexError.
                        pair_check = verify_pair(r.path[i], r.path[i-1]);
                        if(pair_check == ASPV::Invalid){
                            return ASPV::Invalid;
                        }else if(pair_check == ASPV::Unknown){
                            semi_state = ASPV::Unknown;
                        }
                    }
                }
                return semi_state;
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    optional<Isec> isec_v(const Route r, const Message update_msg){
        // REFERENCE
        // C. Morris, A. Herzberg, B. Wang, and S. Secondo,
        // "BGP-iSec: Improved Security of Internet Routing Against Post-ROV Attacks",
        // in USENIX Network and Distributed System Security (NDSS) Symposium, 2024.
        // https://dx.doi.org/10.14722/ndss.2024.241035

        // Origin = X0 -> X1 -> ... -> Xl -> Y = *update_msg.dst
        // update_msg.path = {X0, X1, ..., Xl}, and Y is not included.

        if(update_msg.type == MessageType::Init){
            return nullopt;
        }

        // if the AS Y is not adopted AS, iSec should not evaluated.
        if(!contains(isec_adopted_as_list, *update_msg.dst)){
            return nullopt;
        }

        // If the origin AS does not adopted, iSec should not evaluated.
        if(!contains(isec_adopted_as_list, get<ASNumber>((*update_msg.path).front()))){
            return nullopt;
        }

        if(update_msg.come_from == ComeFrom::Provider){
            return Isec::Valid;
        }else{
            vector<ASNumber> adopted_path_as = {};
            for(const auto as_number : *update_msg.path){
                // The type of as_number MUST be ASNumber
                if(contains(isec_adopted_as_list, get<ASNumber>(as_number))){
                    adopted_path_as.push_back(get<ASNumber>(as_number));
                }
            }
            int i = 0;
            while(i < static_cast<int>(size(adopted_path_as)) - 1){
                if(!contains(public_ProConID[adopted_path_as[i]], adopted_path_as[i+1])){
                    return Isec::Invalid;
                }
                ++i;
            }
            if(update_msg.come_from == ComeFrom::Peer){
                return Isec::Valid;
            }else if(update_msg.come_from == ComeFrom::Customer){
                if(contains(public_ProConID[adopted_path_as.back()], *update_msg.dst)){
                    return Isec::Valid;
                }else{
                    return Isec::Invalid;
                }
            }
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    }

    void new_route_security_validation(Route* route, Message update_msg){
        route->security_valid->aspv = aspv(*route, update_msg.src);
        route->security_valid->isec_v = isec_v(*route, update_msg);
        // other security function should be added here.
    }

};

#endif
