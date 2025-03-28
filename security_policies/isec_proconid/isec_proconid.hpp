#ifndef ISEC_PROCONID_H
#define ISEC_PROCONID_H

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

#include "../../manager.hpp"
#include "../../security.hpp"

class IASManager;
class ILogger;

class I_Isec_ProconID : public ISecurityPolicy{
public:
    virtual void publish_isec(ASNumber asn) = 0;
    virtual optional<Isec> verification(const Route r, const Message update_msg) = 0;
    virtual ~I_Isec_ProconID() = default;
};

class Isec_ProconID : public I_Isec_ProconID{
    vector<ASNumber> adoption_list;
    map<ASNumber, vector<ASNumber>> proconid_list;
    IASManager* as_manager;
    ILogger* logger;
public:
    Isec_ProconID(IASManager* as_manager, ILogger* logger)
        : as_manager(as_manager), logger(logger){}
    void publish_isec(ASNumber asn) override;
    optional<Isec> verification(const Route r, const Message update_msg) override;
    void before_run(void) override;
    virtual ~Isec_ProconID() override = default;
};

#endif
