#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

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

#include "security_policies/aspa/aspa.hpp"
#include "security_policies/isec_proconid/isec_proconid.hpp"

// This class has both RPKI data and security verification processes.
class SecurityManager{
    IASManager* as_manager;
    ILogger* logger;
public:
    I_ASPA* aspa;
    I_Isec_ProconID* isec;
public:
    SecurityManager(IASManager* as_manager, ILogger* logger)
    : as_manager(as_manager), logger(logger),
      aspa(new ASPA(as_manager, logger)),
      isec(new Isec_ProconID(as_manager, logger)) {}

    bool validation(vector<Policy> policies, Route* route, Message update_msg);
    void before_run(void);
    void adopt_security_policy(ASNumber asn, Policy policy, int priority);
};

#endif
