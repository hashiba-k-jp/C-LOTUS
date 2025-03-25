namespace YAML{
    template<>
    struct convert<MessageType>{
        static Node encode(const MessageType& msg_type){
            Node node;
            switch(msg_type) {
                #define X(name) case MessageType::name: node = #name; break;
                MESSAGE_TYPE
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, MessageType& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = MessageType::name; return true; }
            MESSAGE_TYPE
            #undef X
            return false;
        }
    };

    template<>
    struct convert<ConnectionType>{
        static Node encode(const ConnectionType& c_type){
            Node node;
            switch(c_type) {
                #define X(name) case ConnectionType::name: node = #name; break;
                CONNECTION_TYPE
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, ConnectionType& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ConnectionType::name; return true; }
            CONNECTION_TYPE
            #undef X
            return false;
        }
    };

    template<>
    struct convert<ComeFrom>{
        static Node encode(const ComeFrom& come_from){
            Node node;
            switch(come_from) {
                #define X(name) case ComeFrom::name: node = #name; break;
                COMEFROM
                #undef X
            }
            return node;
        }
        static bool decode(const Node& node, ComeFrom& value){
            if(!node.IsScalar()){ return false; }
            string s = node.as<string>();
            #define X(name) if(caseInsensitiveCompare(s, #name)){ value = ComeFrom::name; return true; }
            COMEFROM
            #undef X
            return true;
        }
    };

    template<typename T>
    struct convert<optional<T>> {
        static Node encode(const optional<T>& value) {
            Node node;
            if(!value){
                node = Node();
            } else {
                node = *value;
            }
            return node;
        }
        static bool decode(const Node& node, optional<T>& value) {
            if (!node || node.IsNull()){
                value = nullopt;
                return true;
            }
            try {
                value = node.as<T>();
                return true;
            } catch (const YAML::BadConversion&) {
                return false;
            }
        }
    };

    template<>
    struct convert<Message>{
        static Node encode(const Message& msg){
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
        static bool decode(const Node& node, Message& msg){
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
    };

    template<>
    struct convert<Connection>{
        static Node encode(const Connection& c){
            Node node;
            node["dst"]  = c.dst;
            node["src"]  = c.src;
            node["type"] = c.type;
            return node;
        }
        static bool decode(const Node& node, Connection& c){
            if(!node.IsScalar()){
                return false;
            }
            c.src  = node["src"].as<ASNumber>();
            c.dst  = node["dst"].as<ASNumber>();
            c.type = node["type"].as<ConnectionType>();
            return true;
        }
    };

    template<>
    struct convert<Route*>{
        static Node encode(const Route* r){
            Node node;
            node["path"]      = string_path(r->path);
            node["come_from"] = r->come_from;
            node["LocPrf"]    = r->LocPrf;
            node["best_path"] = r->best_path;
            node["sec_valid"] = r->security_valid;
            return node;
        };
        static bool decode(const Node& node, Route*& r){
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
                node["sec_valid"].as<SecurityValid*>()
            };
            return true;
        }
    };

    template<typename Type>
    struct convert<vector<Type>>{
        static Node encode(const vector<Type>& vector_t){
            Node node(NodeType::Sequence);
            for(const auto& entry : vector_t){
                node.push_back(entry);
            }
            return node;
        }
        static bool decode(const Node& node, vector<Type>& vector_t){
            if(!node.IsMap()){
                return false;
            }
            for(const auto& entry : node){
                vector_t.push_back(entry.as<Type>());
            }
            return true;
        }
    };

    template<typename Type>
    struct convert<queue<Type>>{
        static Node encode(const queue<Type>& queue_t){
            Node node(NodeType::Sequence);
            queue<Type> tmp_queue = queue_t;
            while(!tmp_queue.empty()){
                node.push_back(tmp_queue.front());
                tmp_queue.pop();
            }
            return node;
        }
        static bool decode(const Node& node, queue<Type>& queue_t){
            if(!node.IsSequence()){
                return false;
            }
            for(const auto& entry : node){
                queue_t.push(entry.as<Type>());
            }
            return true;
        }
    };

    template<typename Key, typename Value>
    struct convert<map<Key, Value>>{
        static Node encode(const map<Key, Value>& map){
            Node node(NodeType::Map);
            for (const auto& entry : map){
                node[entry.first] = entry.second;
            }
            return node;
        }
        static bool decode(const Node& node, map<Key, Value>& map){
            if(!node.IsMap()){
                return false;
            }
            for(const auto& entry : node){
                map[entry.first.as<Key>()] = entry.second.as<Value>();
            }
            return true;
        }
    };

    template<>
    struct convert<ASClass> {
        static Node encode(const ASClass& as_class) {
            Node node;
            node["AS"]              = as_class.as_number;
            node["network_address"] = as_class.network_address;
            node["policy"]          = as_class.policy;
            node["routing_table"]   = as_class.routing_table;
            return node;
        }
        static bool decode(const Node& node, ASClass& as_class) {
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
    };

    template<>
    struct convert<ASClassList> {
        static Node encode(const ASClassList& as_class_list) {
            Node node;
            for(auto it = as_class_list.class_list.begin(); it != as_class_list.class_list.end(); it++){
                node["AS_list"].push_back(it->second);
            }
            node["IP_gen_seed"] = as_class_list.ip_gen.index;
            return node;
        }
        static bool decode(const Node& node, ASClassList& as_class_list) {
            if(!node.IsSequence()){
                return false;
            }
            for(const auto& as_class : node["AS_list"]){
                as_class_list.class_list[as_class["AS"].as<ASNumber>()] = as_class.as<ASClass>();
            }
            as_class_list.ip_gen = IPAddressGenerator{node["IP_gen_seed"].as<int>()};
            return true;
        }
    };

    template<>
    struct convert<RoutingTable>{
        static Node encode(const RoutingTable& routing_table){
            Node node;
            for(const auto& it : routing_table.table){
                node[it.first] = it.second;
            }
            return node;
        };
        static bool decode(const Node& node, RoutingTable& routing_table){
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
    };

    template<>
    struct convert<SecurityValid*>{
        static Node encode(const SecurityValid* security_valid){
            Node node;
            node["aspv"] = security_valid->aspv;
            node["isec_v"] = security_valid->isec_v;
            return node;
        }
        static bool decode(const Node& node, SecurityValid*& security_valid){
            if(!node.IsSequence()){
                return false;
            }
            security_valid = new SecurityValid{
                node["aspv"].as<ASPV>(),
                node["aspv"].as<Isec>(),
            };
            return true;
        }
    };
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const optional<T>& val) {
    if(val == nullopt){
        os << "Null";
    }else{
        os << val;
    }
    return os;
}
