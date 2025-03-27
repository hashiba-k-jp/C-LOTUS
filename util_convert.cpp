#include "util_convert.hpp"

namespace YAML{
    Node convert<MessageType>::encode(const MessageType& msg_type){
        Node node;
        switch(msg_type) {
            #define X(name) case MessageType::name: node = #name; break;
            MESSAGE_TYPE
            #undef X
        }
        return node;
    }
    bool convert<MessageType>::decode(const Node& node, MessageType& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(caseInsensitiveCompare(s, #name)){ value = MessageType::name; return true; }
        MESSAGE_TYPE
        #undef X
        return false;
    }

    Node convert<ConnectionType>::encode(const ConnectionType& c_type){
        Node node;
        switch(c_type) {
            #define X(name) case ConnectionType::name: node = #name; break;
            CONNECTION_TYPE
            #undef X
        }
        return node;
    }
    bool convert<ConnectionType>::decode(const Node& node, ConnectionType& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ConnectionType::name; return true; }
        CONNECTION_TYPE
        #undef X
        return false;
    }

    Node convert<ComeFrom>::encode(const ComeFrom& come_from){
        Node node;
        switch(come_from) {
            #define X(name) case ComeFrom::name: node = #name; break;
            COMEFROM
            #undef X
        }
        return node;
    }
    bool convert<ComeFrom>::decode(const Node& node, ComeFrom& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ComeFrom::name; return true; }
        COMEFROM
        #undef X
        return true;
    }

    Node convert<ASPV>::encode(const ASPV& aspv){
        Node node;
        switch(aspv) {
            #define X(name) case ASPV::name: node = #name; break;
            ASPV_TYPE
            #undef X
        }
        return node;
    }
    bool convert<ASPV>::decode(const Node& node, ASPV& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(s == #name){ value = ASPV::name; return true; }
        ASPV_TYPE
        #undef X
        return true;
    }

    Node convert<Isec>::encode(const Isec& isec_v){
        Node node;
        switch(isec_v) {
            #define X(name) case Isec::name: node = #name; break;
            ISEC_TYPE
            #undef X
        }
        return node;
    }
    bool convert<Isec>::decode(const Node& node, Isec& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(s == #name){ value = Isec::name; return true; }
        ISEC_TYPE
        #undef X
        return true;
    }

    Node convert<Policy>::encode(const Policy& policy){
        Node node;
        switch(policy) {
            #define X(name) case Policy::name: node = #name; break;
            POLICY
            #undef X
        }
        return node;
    }
    bool convert<Policy>::decode(const Node& node, Policy& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(caseInsensitiveCompare(s, #name)){ value = Policy::name; return true; }
        POLICY
        #undef X
        return false;
    }

    Node convert<Message>::encode(const Message& msg){
        Node node;
        node["type"] = msg.type;
        node["src"]  = msg.src;
        if(msg.type == MessageType::Update){
            node["dst"]       = *msg.dst;
            node["network"]   = *msg.address;
            node["path"]      = string_path(*msg.path);
            node["come_from"] = *msg.come_from;
        }
        return node;
    }
    bool convert<Message>::decode(const Node& node, Message& msg){
        if(!node.IsScalar()){
            return false;
        }
        msg.type = node["type"].as<MessageType>();
        msg.src  = node["src"].as<ASNumber>();
        if(node["type"].as<MessageType>() == MessageType::Update){
            msg.dst       = node["dst"].as<ASNumber>();
            msg.address   = node["network"].as<IPAddress>();
            msg.path      = parse_path(node["path"].as<string>());
            msg.come_from = node["come_from"].as<ComeFrom>();
        }
        return true;
    }

    Node convert<Connection>::encode(const Connection& c){
        Node node;
        node["dst"]  = c.dst;
        node["src"]  = c.src;
        node["type"] = c.type;
        return node;
    }
    bool convert<Connection>::decode(const Node& node, Connection& c){
        if(!node.IsScalar()){
            return false;
        }
        c.src  = node["src"].as<ASNumber>();
        c.dst  = node["dst"].as<ASNumber>();
        c.type = node["type"].as<ConnectionType>();
        return true;
    }

    Node convert<Route*>::encode(const Route* r){
        Node node;
        node["path"]      = string_path(r->path);
        node["come_from"] = r->come_from;
        node["LocPrf"]    = r->LocPrf;
        node["best_path"] = r->best_path;
        node["aspv"]      = r->aspv;
        node["isec_v"]    = r->isec_v;
        return node;
    };
    bool convert<Route*>::decode(const Node& node, Route*& r){
        if(!node.IsMap()){
            return false;
        }
        optional<ASPV> aspv = nullopt;
        optional<Isec> isec_v;
        if(node["aspv"] && !node["aspv"].IsNull()){
            aspv = node["aspv"].as<ASPV>();
        }
        if(node["isec_v"] && !node["isec_v"].IsNull()){
            isec_v = node["isec_v"].as<Isec>();
        }
        r = new Route{
            parse_path(node["path"].as<string>()),
            node["come_from"].as<ComeFrom>(),
            node["LocPrf"].as<int>(),
            node["best_path"].as<bool>(),
            aspv,
            isec_v
        };
        return true;
    }

    Node convert<ASClass>::encode(const ASClass& as_class) {
        Node node;
        node["AS"]              = as_class.as_number;
        node["network_address"] = as_class.network_address;
        node["policy"]          = as_class.policy;
        node["routing_table"]   = as_class.routing_table;
        return node;
    }
    bool convert<ASClass>::decode(const Node& node, ASClass& as_class) {
        if(!node.IsSequence()){
            return false;
        }
        as_class.as_number       = node["AS"].as<ASNumber>();
        as_class.network_address = node["network_address"].as<IPAddress>();
        as_class.policy          = node["policy"].as<vector<Policy>>();
        as_class.routing_table   = node["routing_table"].as<RoutingTable>();
        as_class.routing_table.policy = as_class.policy;
        return true;
    }

    Node convert<RoutingTable>::encode(const RoutingTable& routing_table){
        Node node;
        for(const auto& it : routing_table.table){
            node[it.first] = it.second;
        }
        return node;
    };
    bool convert<RoutingTable>::decode(const Node& node, RoutingTable& routing_table){
        if(!node.IsMap()){
            return false;
        }
        map<IPAddress, vector<Route*>> table;
        for(const auto& r : node){
            IPAddress route_address = r.first.as<IPAddress>();
            for(const auto& route : r.second){
                Route* new_route = route.as<Route*>();
                table[route_address].push_back(new_route);
            }
        }
        routing_table.table = table;
        return true;
    }
}
