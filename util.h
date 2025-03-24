#ifndef UTIL_H
#define UTIL_H

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    return std::equal(str1.begin(), str1.end(), str2.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

/***
 *** For safety typing
 ***/
using ASNumber = int;
using IPAddress = string;

/***
 *** enum definitions
 ***/
#define MESSAGE_TYPE X(Init) X(Update)
#define CONNECTION_TYPE X(Peer) X(Down)
#define COMEFROM X(Customer) X(Peer) X(Provider)
#define POLICY X(LocPrf) X(PathLength) X(Aspa) X(Isec)
#define ASPV_TYPE X(Valid) X(Invalid) X(Unknown)
#define ISEC_TYPE X(Valid) X(Invalid) X(Debug)

#define CREATE_ENUM_CLASS(ClassName, EnumValues) \
enum class ClassName{ \
    EnumValues\
};
#define X(name) name,
CREATE_ENUM_CLASS(MessageType, MESSAGE_TYPE)
CREATE_ENUM_CLASS(ConnectionType, CONNECTION_TYPE)
CREATE_ENUM_CLASS(ComeFrom, COMEFROM)
CREATE_ENUM_CLASS(Policy, POLICY)
CREATE_ENUM_CLASS(ASPV, ASPV_TYPE)
CREATE_ENUM_CLASS(Isec, ISEC_TYPE)
#undef X

#define OPERATOR_COUT(ClassName, EnumValues)\
std::ostream& operator<<(std::ostream& os, ClassName value) {\
    switch (value) {\
        EnumValues\
    }\
    return os;\
}

#define X(name) case MessageType::name: os << #name; break;
OPERATOR_COUT(MessageType, MESSAGE_TYPE)
#define X(name) case ConnectionType::name: os << #name; break;
OPERATOR_COUT(ConnectionType, CONNECTION_TYPE)
#define X(name) case ComeFrom::name: os << #name; break;
OPERATOR_COUT(ComeFrom, COMEFROM)
#define X(name) case Policy::name: os << #name; break;
OPERATOR_COUT(Policy, POLICY)
#define X(name) case ASPV::name: os << #name; break;
OPERATOR_COUT(ASPV, ASPV_TYPE)
#define X(name) case Isec::name: os << #name; break;
OPERATOR_COUT(Isec, ISEC_TYPE)
#undef X

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

template<>
struct convert<ASPV>{
    static Node encode(const ASPV& aspv){
        Node node;
        switch(aspv) {
            #define X(name) case ASPV::name: node = #name; break;
            ASPV_TYPE
            #undef X
        }
        return node;
    }
    static bool decode(const Node& node, ASPV& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(s == #name){ value = ASPV::name; return true; }
        ASPV_TYPE
        #undef X
        return true;
    }
};

template<>
struct convert<Isec>{
    static Node encode(const Isec& isec_v){
        Node node;
        switch(isec_v) {
            #define X(name) case Isec::name: node = #name; break;
            ISEC_TYPE
            #undef X
        }
        return node;
    }
    static bool decode(const Node& node, Isec& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(s == #name){ value = Isec::name; return true; }
        ISEC_TYPE
        #undef X
        return true;
    }
};

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

template<>
struct convert<vector<Policy>> {
    static Node encode(const vector<Policy>& policies) {
        Node node;
        for (const auto& p : policies) {
            node.push_back(p);
        }
        return node;
    }
    static bool decode(const Node& node, std::vector<Policy>& policy_list) {
        if(!node.IsSequence()){ return false; }
        for (const auto& n : node) {
            policy_list.push_back(n.as<Policy>());
        }
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
        if (!node || node.IsNull()) { // YAMLのnullをチェック
            value = nullopt;
            return true;
        }

        try {
            value = node.as<T>(); // 型Tに変換
            return true;
        } catch (const YAML::BadConversion&) {
            return false; // 変換失敗時はfalseを返す
        }
    }
};

#endif
