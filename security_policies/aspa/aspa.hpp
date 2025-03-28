#ifndef ASPA_H
#define ASPA_H

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

class I_ASPA : public ISecurityPolicy{
public:
    virtual void publish_aspa(ASNumber asn) = 0;
    virtual void set_spas(ASNumber asn, vector<ASNumber> provider_list) = 0;
    virtual void show_ASPA_list() = 0;
    virtual ASPV aspv(const Route r, ASNumber neighbor_as) = 0;
    virtual ~I_ASPA() = default;
};

class ASPA : public I_ASPA{
    vector<ASNumber> adoption_list;
    map<ASNumber, vector<ASNumber>> spas_list;
    IASManager* as_manager;
    ILogger* logger;
public:
    ASPA(IASManager* as_manager, ILogger* logger)
        : as_manager(as_manager), logger(logger){}
    void publish_aspa(ASNumber asn) override;
    void set_spas(ASNumber asn, vector<ASNumber> provider_list) override;
    void show_ASPA_list(void) override;
    ASPV aspv(const Route r, ASNumber neighbor_as) override;
    void before_run(void) override;
    ASPV verify_pair(variant<ASNumber, Itself> customer, variant<ASNumber, Itself> provider);
    virtual ~ASPA() override = default;
};

#endif
