#include "isec_proconid.hpp"

void Isec_ProconID::publish_isec(ASNumber asn){
    if(!as_manager->has_AS(asn)){
        logger->warn("The AS number " + to_string(asn) + " has NOT been registered. (Aborted)");
    }
    if(contains(adoption_list, asn)){
        logger->warn("The AS number " + to_string(asn) + " has already adopted BGP-iSec(ProConID). (Overwrite)");
    }
    adoption_list.push_back(asn);
    return;
}

optional<Isec> Isec_ProconID::verification(const Route r, const Message update_msg){
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
    if(!contains(adoption_list, *update_msg.dst)){
        return nullopt;
    }

    // If the origin AS does not adopted, iSec should not evaluated.
    if(!contains(adoption_list, get<ASNumber>((*update_msg.path).front()))){
        return nullopt;
    }

    if(update_msg.come_from == ComeFrom::Provider){
        return Isec::Valid;
    }else{
        vector<ASNumber> adopted_path_as = {};
        for(const auto as_number : *update_msg.path){
            // The type of as_number MUST be ASNumber
            if(contains(adoption_list, get<ASNumber>(as_number))){
                adopted_path_as.push_back(get<ASNumber>(as_number));
            }
        }
        int i = 0;
        while(i < static_cast<int>(size(adopted_path_as)) - 1){
            if(!contains(proconid_list[adopted_path_as[i]], adopted_path_as[i+1])){
                return Isec::Invalid;
            }
            ++i;
        }
        if(update_msg.come_from == ComeFrom::Peer){
            return Isec::Valid;
        }else if(update_msg.come_from == ComeFrom::Customer){
            if(contains(proconid_list[adopted_path_as.back()], *update_msg.dst)){
                return Isec::Valid;
            }else{
                return Isec::Invalid;
            }
        }
    }
    throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
}

void Isec_ProconID::before_run(){
    //make ProConID for all adopting AS.
    return;
}
