#define POLICY_NAME ASPV
#define NEW_POLICY X(Valid) X(Invalid) X(Unknown)

enum class POLICY_NAME{
    #define X(name) name,
    NEW_POLICY
    #undef x
};

ostream& operator<<(std::ostream& os, POLICY_NAME value) {
    switch (value) {
        #define X(name) case POLICY_NAME::name: os << #name; break;
        NEW_POLICY
        #undef X
    }
    return os;
}

template<>
struct convert<POLICY_NAME>{
    static Node encode(const POLICY_NAME& value){
        Node node;
        switch(value) {
            #define X(name) case POLICY_NAME::name: node = #name; break;
            NEW_POLICY
            #undef X
        }
        return node;
    }
    static bool decode(const Node& node, POLICY_NAME& value){
        if(!node.IsScalar()){ return false; }
        string s = node.as<string>();
        #define X(name) if(s == #name){ value = POLICY_NAME::name; return true; }
        NEW_POLICY
        #undef X
        return true;
    }
};
