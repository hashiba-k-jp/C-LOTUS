#include "security_manager.hpp"

/* SecurityManager */
bool SecurityManager::validation(vector<Policy> policies, Route* route, Message update_msg){
    for(Policy p : policies){
        switch(p){
            case Policy::Aspa:
                if(aspa->aspv(*route, update_msg.src) == ASPV::Invalid){
                    return false;
                }
                break;
            case Policy::Isec:
                if(isec->verification(*route, update_msg) == Isec::Invalid){
                    return false;
                }
            default:
                continue;
        }
    }
    return true;
}

void SecurityManager::adopt_security_policy(ASNumber asn, Policy policy, int priority){

    if(ASClass* as_class = as_manager->get_AS(asn)){
        as_class->add_policy(policy, priority);
    }else{
        logger->warn("AS " + to_string(asn) + " not found. (Aborted)");
        return;
    }

    switch(policy){
        case Policy::Aspa:
            aspa->publish_aspa(asn);
            break;
        case Policy::Isec:
            isec->publish_isec(asn);
            break;
        default:
            break;
    }
    return;
}
