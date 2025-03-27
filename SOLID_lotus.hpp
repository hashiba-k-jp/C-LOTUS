#ifndef SOLID_LOTUS_H
#define SOLID_LOTUS_H

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
#include "util_convert.hpp"


class ILogger{
public:
    virtual void info(const string& msg) = 0;
    virtual void warn(const string& msg) = 0;
    virtual void error(const string& msg) = 0;
    virtual ~ILogger() = default;
};
class Logger : public ILogger{
public:
    void info(const string& msg) override;
    void warn(const string& msg) override;
    void error(const string& msg) override;
};


class IASManager{
public:
    virtual void add_AS(ASNumber asn) = 0;
    virtual void import_AS(ASNumber asn, IPAddress address, vector<Policy> policy, RoutingTable table) = 0;
    virtual ASClass* get_AS(ASNumber asn) = 0;
    virtual bool has_AS(ASNumber asn) = 0;
    virtual void show_AS(ASNumber asn) = 0;
    virtual void show_AS_list() = 0;
    virtual const map<ASNumber, ASClass> get_AS_list() = 0;
    virtual ~IASManager() = default;
};
class ASManager : public IASManager{
private:
    map<ASNumber, ASClass> as_class_list;
    ILogger* logger;
    int index = 0;
public:
    ASManager(ILogger* logger) : logger(logger){}
    void add_AS(ASNumber asn) override;
    void import_AS(ASNumber asn, IPAddress address, vector<Policy> policy, RoutingTable table) override;
    ASClass* get_AS(ASNumber asn) override;
    bool has_AS(ASNumber asn) override;
    void show_AS(ASNumber asn) override;
    void show_AS_list(void) override;
    const map<ASNumber, ASClass> get_AS_list() override;
};


class IConnectionManager{
public:
    virtual void add_connection(ConnectionType type, ASNumber src, ASNumber dst) = 0;
    virtual void show_connections() = 0;
    virtual vector<Connection> get_connections_with(ASNumber asn) = 0;
    virtual ComeFrom as_a_is_what_on_c(ASNumber asn, Connection c) = 0;
    virtual vector<Connection> get_all_connections() = 0;
    virtual ~IConnectionManager() = default;
};
class ConnectionManager : public IConnectionManager{
    vector<Connection> connection_list;
    IASManager* as_manager;
    ILogger* logger;
public:
    ConnectionManager(ILogger* logger, IASManager* as_manager) : as_manager(as_manager), logger(logger){}
    void add_connection(ConnectionType type, ASNumber src, ASNumber dst) override;
    void show_connections(void) override;
    vector<Connection> get_connections_with(ASNumber asn) override;
    ComeFrom as_a_is_what_on_c(ASNumber asn, Connection c) override;
    vector<Connection> get_all_connections() override;
};


class IMessageProcessor{
public:
    virtual void add_message(MessageType msgtype, ASNumber src, optional<ASNumber> dst=nullopt, optional<IPAddress> address=nullopt, optional<Path> path=nullopt) = 0;
    virtual void push_message(Message msg) = 0;
    virtual void add_all_init() = 0;
    virtual void show_messages() = 0;
    virtual optional<Message> pop_message() = 0;
    virtual queue<Message> get_messages() = 0;
    virtual ~IMessageProcessor() = default;
};
class MessageProcessor : public IMessageProcessor{
    queue<Message> message_queue;
    IASManager* as_manager;
    ILogger* logger;
public:
    MessageProcessor(IASManager* as_manager, ILogger* logger) : as_manager(as_manager), logger(logger) {}
    void add_message(MessageType msgtype, ASNumber src, optional<ASNumber> dst=nullopt, optional<IPAddress> address=nullopt, optional<Path> path=nullopt)override;
    void push_message(Message msg) override;
    void add_all_init(void) override;
    void show_messages(void) override;
    optional<Message> pop_message(void) override;
    queue<Message> get_messages(void) override;
};


class IFileManager{
public:
    virtual int file_import(const string& file_path_string) = 0;
    virtual int file_export(const string& file_path_string) = 0;
    virtual ~IFileManager() = default;
};
class FileManager : public IFileManager{
    IASManager* as_manager;
    IConnectionManager* conn_manager;
    IMessageProcessor* msg_manager;
    ILogger* logger;
public:
    FileManager(
        IASManager* as_manager,
        IConnectionManager* conn_manager,
        IMessageProcessor* msg_manager,
        ILogger* logger
    ) : as_manager(as_manager), conn_manager(conn_manager), msg_manager(msg_manager), logger(logger){}
    int file_import(const string& file_path_string) override;
    int file_export(const string& file_path_string) override;
};


class NEW_LOTUS{
    IASManager* as_manager;
    IConnectionManager* conn_manager;
    IMessageProcessor* msg_manager;
    // ISecurityPolicyManager* secPolicyManager;
    ILogger* logger;
public:
    NEW_LOTUS(
        IASManager* as_manager,
        IConnectionManager* conn_manager,
        IMessageProcessor* msg_manager,
        ILogger* logger
    )
    : as_manager(as_manager), conn_manager(conn_manager), msg_manager(msg_manager), logger(logger){}
    void run(void);
};


#endif
