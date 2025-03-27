#ifndef UTIL_CONVERT_H
#define UTIL_CONVERT_H

#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <variant>

#include <yaml-cpp/yaml.h>
using namespace std;
using namespace YAML;

#include "util.hpp"
#include "data_struct.hpp"
#include "routing_table.hpp"
#include "as_class.hpp"

namespace YAML{
    template<>
    struct convert<MessageType>{
        static Node encode(const MessageType& msg_type);
        static bool decode(const Node& node, MessageType& value);
    };

    template<>
    struct convert<ConnectionType>{
        static Node encode(const ConnectionType& c_type);
        static bool decode(const Node& node, ConnectionType& value);
    };

    template<>
    struct convert<ComeFrom>{
        static Node encode(const ComeFrom& come_from);
        static bool decode(const Node& node, ComeFrom& value);
    };

    template<>
    struct convert<ASPV>{
        static Node encode(const ASPV& aspv);
        static bool decode(const Node& node, ASPV& value);
    };

    template<>
    struct convert<Isec>{
        static Node encode(const Isec& isec_v);
        static bool decode(const Node& node, Isec& value);
    };

    template<>
    struct convert<Policy>{
        static Node encode(const Policy& policy);
        static bool decode(const Node& node, Policy& value);
    };

    template<>
    struct convert<Message>{
        static Node encode(const Message& msg);
        static bool decode(const Node& node, Message& msg);
    };

    template<>
    struct convert<Connection>{
        static Node encode(const Connection& c);
        static bool decode(const Node& node, Connection& c);
    };

    template<>
    struct convert<Route*>{
        static Node encode(const Route* r);
        static bool decode(const Node& node, Route*& r);
    };

    template<typename Type>
    struct convert<optional<Type>>{
        static Node encode(const optional<Type>& value) {
            Node node;
            if(!value){
                node = Node();
            } else {
                node = *value;
            }
            return node;
        }
        static bool decode(const Node& node, optional<Type>& value) {
            if (!node || node.IsNull()){
                value = nullopt;
                return true;
            }
            try {
                value = node.as<Type>();
                return true;
            } catch (const YAML::BadConversion&) {
                return false;
            }
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
        static Node encode(const ASClass& as_class);
        static bool decode(const Node& node, ASClass& as_class);
    };

    template<>
    struct convert<RoutingTable>{
        static Node encode(const RoutingTable& routing_table);
        static bool decode(const Node& node, RoutingTable& routing_table);
    };
}

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const optional<T>& val) {
    if(val == nullopt){
        os << "Null";
    }else{
        os << val;
    }
    return os;
}

#endif
