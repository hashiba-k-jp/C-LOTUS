#include "aspa.hpp"

void ASPA::publish_aspa(ASNumber asn){
    if(!as_manager->has_AS(asn)){
        logger->error("The AS number " + to_string(asn) + " has NOT been registered. (Aborted)");
        return;
    }
    if(contains(adoption_list, asn)){
        logger->warn("The AS number " + to_string(asn) + " has already adopted ASPA. (Aborted)");
        return;
    }
    adoption_list.push_back(asn);
    return;
}

void ASPA::set_spas(ASNumber asn, vector<ASNumber> provider_list){
    if(!as_manager->has_AS(asn)){
        logger->error("The AS number " + to_string(asn) + " has NOT been registered. (Aborted)");
        return;
    }
    if(!contains(adoption_list, asn)){
        logger->warn("The AS number " + to_string(asn) + " does not adpot ASPA. (Continue)");
    }
    spas_list[asn] = provider_list;
    return;
}

void ASPA::show_ASPV_list(void){
    cout << "--------------------" << "\n";
    cout << "ASPV enabled AS" << '\n';
    cout << adoption_list << "\n";
    cout << "--------------------" << "\n";
}

void ASPA::show_SPAS_list(void){
    cout << "--------------------" << "\n";
    cout << "ASPA" << '\n';
    for(const auto& it : spas_list){
        cout << "  - \033[1mcustomer\033[0m : " << it.first;
        cout << ", \033[1mSPAS\033[0m : " << it.second << "\n";
    }
    cout << "--------------------" << "\n";
}

ASPV ASPA::verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider){
    if(!contains(adoption_list, get<ASNumber>(customer))){
        return ASPV::Unknown;
    }else{
        const vector<ASNumber>& saps = spas_list[get<ASNumber>(customer)];
        if(contains(saps, get<ASNumber>(provider))){
            return ASPV::Valid;
        }else{
            return ASPV::Invalid;
        }
    }
    throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
}

ASPV ASPA::aspv(const Route r, ASNumber neighbor_as){
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

void ASPA::before_run(){
    //some process
    return;
}
