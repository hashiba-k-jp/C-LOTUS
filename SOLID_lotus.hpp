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
#include "manager.hpp"

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
