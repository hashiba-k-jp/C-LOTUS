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
#include "routing_table.hpp"
// #include "as_class.hpp"
// #include "util_convert.hpp"

class ISecurityPolicy{
public:
    // virtual bool validation(void) = 0;
    virtual void before_run(void) = 0;
    virtual ~ISecurityPolicy(void) = default;
};

#endif
