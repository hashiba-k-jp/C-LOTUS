#include "security_manager.hpp"

/* SecurityManager */
bool SecurityManager::validation_all_valid(const vector<Policy> policy, const Route& route) const{
    if(contains(policy, Policy::Aspa) && route.route_sec.aspv == ASPV::Invalid){
        return false;
    }
    if(contains(policy, Policy::Isec) && route.route_sec.isec_v == Isec::Invalid){
        return false;
    }
    return true;
}

void SecurityManager::validation(vector<Policy> policies, Route* route, Message update_msg){
    // to be inplemented.
    return;
}

void SecurityManager::adopt_security_policy(ASNumber asn, Policy policy, int priority){
    // [ For ASPA/ASPV ]
    // This function only enables ASPV for given ASNumber. Use aspa->set_spas() to publish SPAS (ASPA).

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
