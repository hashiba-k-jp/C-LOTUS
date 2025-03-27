#ifndef UTIL_H
#define UTIL_H

inline bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
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
// #define VALIDATION X(Valid) X(Invalid)
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
inline std::ostream& operator<<(std::ostream& os, ClassName value) {\
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

#define TO_STRING(ClassName, EnumValues)\
inline std::string to_string(ClassName value) {\
    switch (value) {\
        EnumValues\
    }\
}

#define X(name) case MessageType::name: return #name;
TO_STRING(MessageType, MESSAGE_TYPE)
#define X(name) case ConnectionType::name: return #name;
TO_STRING(ConnectionType, CONNECTION_TYPE)
#define X(name) case ComeFrom::name: return #name;
TO_STRING(ComeFrom, COMEFROM)
#define X(name) case Policy::name: return #name;
TO_STRING(Policy, POLICY)
#define X(name) case ASPV::name: return #name;
TO_STRING(ASPV, ASPV_TYPE)
#define X(name) case Isec::name: return #name;
TO_STRING(Isec, ISEC_TYPE)
#undef X

#endif
