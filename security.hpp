#ifndef SECURITY_H
#define SECURITY_H

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
class Route;

class ISecurityPolicy{
public:
    // virtual bool validation(void) = 0;
    virtual void before_run(void) = 0;
    virtual ~ISecurityPolicy(void) = default;
};

class ISecurityManager{
public:
    virtual bool validation_all_valid(const vector<Policy> policy, const Route& route) const = 0;
    virtual void validation(vector<Policy> policies, Route* route, Message update_msg) = 0;
    virtual ~ISecurityManager() = default;
};

#endif
