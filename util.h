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

#define CREATE_ENUM_CLASS(ClassName, EnumValues) \
enum class ClassName{ \
    EnumValues\
};
#define X(name) name,
CREATE_ENUM_CLASS(MessageType, MESSAGE_TYPE)
CREATE_ENUM_CLASS(ConnectionType, CONNECTION_TYPE)
CREATE_ENUM_CLASS(ComeFrom, COMEFROM)
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
#undef X

#endif
