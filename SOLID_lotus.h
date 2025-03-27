/* Logger */
class ILogger{
public:
    virtual void info(const string& msg) = 0;
    virtual void warn(const string& msg) = 0;
    virtual void error(const string& msg) = 0;
    virtual ~ILogger() = default;
};

class Logger : public ILogger{
public:
    void info(const string& msg) override{
        cout << "\033[32m[INFO ] " << msg << "\033[39m\n";
    };
    void warn(const string& msg) override{
        cout << "\033[33m[WARN ] " << msg << "\033[39m\n";
    };
    void error(const string& msg) override{
        cout << "\033[31m[ERROR] " << msg << "\033[39m\n";
    };
};

/* ASClass */
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

    void add_AS(ASNumber asn) override{
        if(asn == 0){
            logger->warn("AS number 0 is special and cannot be added.");
        }else if(as_class_list.find(asn) != as_class_list.end()){
            logger->warn("AS " + to_string(asn) + " already exists.");
        }else{
            index += 1;
            ASClass new_as_class = ASClass {asn, "10." + to_string(index/256) + "." + to_string(index%256) + ".0/24"};
            as_class_list[asn] = new_as_class;
            logger->info("AS " + to_string(asn) + " successfully added.");
        }
    }

    virtual void import_AS(ASNumber asn, IPAddress address, vector<Policy> policy, RoutingTable table) override{
        as_class_list[asn] = ASClass{asn, address, policy, table};
    }

    ASClass* get_AS(ASNumber asn) override{
        if(as_class_list.find(asn) != as_class_list.end()){
            return &as_class_list[asn];
        }
        return nullptr;
    }

    bool has_AS(ASNumber asn) override{
        return as_class_list.find(asn) != as_class_list.end();
    }

    void show_AS(ASNumber asn) override{
        if(ASClass* as_class = get_AS(asn)){
            as_class->show_AS();
        }else{
            logger->warn("AS " + to_string(asn) + " not found.");
        }
    }

    void show_AS_list(void) override{
        for(pair<ASNumber, ASClass> it : as_class_list){
            it.second.show_AS();
        }
    }

    const map<ASNumber, ASClass> get_AS_list() override{
        return as_class_list;
    }
};

/* Connections */
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
    ConnectionManager(ILogger* logger, IASManager* as_manager)
        : as_manager(as_manager), logger(logger){}

    void add_connection(ConnectionType type, ASNumber src, ASNumber dst) override{
        if(!as_manager->has_AS(src)){
            logger->warn("The AS number " + to_string(src) + " has NOT been registered. (Aborted)");
            return;
        }
        if(!as_manager->has_AS(dst)){
            logger->warn("The AS number " + to_string(dst) + " has NOT been registered. (Aborted)");
            return;
        }
        const Connection new_connection = Connection{type, src, dst};
        if(contains(connection_list, new_connection)){
            logger->warn("The " + to_string(type) + " connection from" + to_string(src) + " to " + to_string(dst) + " already exists. (Aborted)");
            return;
        }
        connection_list.push_back(new_connection);
        return;
    }

    void show_connections(void) override{
        cout << "********************" << "\n";
        cout << "CONNECTIONS" << "\n";
        for(const Connection& c : connection_list){
            cout << "  * \033[1msrc\033[0m: " << c.src << ", \033[1mdst\033[0m: " << c.dst << ", \033[1mtype\033[0m: " << c.type << "\n";
        }
        cout << "********************" << "\n";
        return;
    }

    vector<Connection> get_connections_with(ASNumber asn) override{
        vector<Connection> connected_with;
        for(const Connection& c : connection_list){
            if(c.src == asn || c.dst == asn){
                connected_with.push_back(c);
                continue;
            }
        }
        return connected_with;
    }

    ComeFrom as_a_is_what_on_c(ASNumber asn, Connection c) override{
        if(c.type == ConnectionType::Peer){
            return ComeFrom::Peer;
        }else if(c.type == ConnectionType::Down){
            if(asn == c.src){
                return ComeFrom::Provider;
            }else if(asn == c.dst){
                return ComeFrom::Customer;
            }
        }
        throw logic_error("\n\033[31m[ERROR] Unreachable code reached in function: " + string(__func__) + " at " + string(__FILE__) + ":" + to_string(__LINE__) + "\033[0m");
    };

    vector<Connection> get_all_connections() override{
        return connection_list;
    }
};

/* Messages */
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

    void add_message(
        MessageType msgtype,
        ASNumber src,
        optional<ASNumber> dst=nullopt,
        optional<IPAddress> address=nullopt,
        optional<Path> path=nullopt
    ) override{
        if(!as_manager->has_AS(src)){
            logger->warn("The AS number " + to_string(src) + " has NOT been registered. (Aborted)");
            return;
        }
        if(msgtype == MessageType::Update){
            if(!as_manager->has_AS(*dst)){
                logger->warn("The AS number " + to_string(*dst) + " has NOT been registered. (Aborted)");
                return;
            }
        }
        message_queue.push(Message{msgtype, src, dst, address, path, nullopt});
        return;
    }

    void push_message(Message msg) override{
        message_queue.push(msg);
        return;
    }

    void add_all_init(void) override{
        for(pair<ASNumber, ASClass> it : as_manager->get_AS_list()){
            add_message(MessageType::Init, it.first);
        }
        return;
    }

    void show_messages(void) override{
        if(message_queue.size() == 0){
            logger->info("No messages in the queue.");
            return;
        }
        cout << "++++++++++++++++++++" << "\n";
        cout << "MESSAGES" << "\n";
        queue<Message> tmp_msg_queue = message_queue;
        while(!tmp_msg_queue.empty()){
            const Message& msg = tmp_msg_queue.front();
            if(msg.type == MessageType::Init){
                cout << "  + \033[1m[" << msg.type << "]\033[0m   \033[1msrc\033[0m: " << msg.src << '\n';
            }else if(msg.type == MessageType::Update){
                cout << "  + \033[1m[" << msg.type << "]\033[0m \033[1msrc\033[0m: " << msg.src << ", \033[1mdst\033[0m: " << *msg.dst << ", \033[1mnetwork\033[0m: " << *msg.address << ", \033[1mpath\033[0m: " << string_path(*msg.path) << "\n";
            }
            tmp_msg_queue.pop();
        }
        cout << "++++++++++++++++++++" << "\n";
        return;
    }

    optional<Message> pop_message(void) override{
        if(!message_queue.empty()){
            Message msg = message_queue.front();
            message_queue.pop();
            return msg;
        }else{
            return nullopt;
        }
    }

    queue<Message> get_messages(void) override{
        return message_queue;
    }
};

/* File I/O */
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

    int file_import(const string& file_path_string) override{
        ifstream file(file_path_string);
        string line;
        if(file){
            try{
                logger->info("Parsing " + file_path_string + " .");

                YAML::Node imported = YAML::Load(file);
                vector<ASNumber> imported_as;

                /* AS LIST */
                for(const auto& as_node : imported["AS_list"]){
                    RoutingTable routing_table = as_node["routing_table"].as<RoutingTable>();
                    vector<Policy> policy = as_node["policy"].as<vector<Policy>>();
                    routing_table.policy = policy;
                    as_manager->import_AS(
                        as_node["AS"].as<ASNumber>(),
                        as_node["network_address"].as<IPAddress>(),
                        policy,
                        routing_table
                    );
                }

                /* CONNECTION LIST */

                for(const auto& c_node : imported["connection"]){
                    conn_manager->add_connection(
                        c_node["type"].as<ConnectionType>(),
                        c_node["src"].as<ASNumber>(),
                        c_node["dst"].as<ASNumber>()
                    );
                }

                /* MESSAGES LIST */
                for(const auto& m_node : imported["message"]){
                    MessageType msgtype = m_node["type"].as<MessageType>();
                    if(msgtype == MessageType::Init){
                        msg_manager->add_message(
                            msgtype,
                            m_node["src"].as<ASNumber>()
                        );
                    }else if(msgtype == MessageType::Update){
                        msg_manager->add_message(
                            msgtype,
                            m_node["src"].as<ASNumber>(),
                            m_node["dst"].as<ASNumber>(),
                            m_node["network"].as<IPAddress>(),
                            parse_path(m_node["path"].as<string>())
                        );
                    }
                }

                return 0;
            }catch(YAML::ParserException &e){
                logger->error("The input file " + file_path_string + " is an INVALID YAML file. (Aborted)\n        " + e.what());
            }
        }else{
            logger->error("The file " + file_path_string + " does NOT exist. (Aborted)");
        }
        return -1;
    }

    int file_export(const string& file_path_string) override{
        YAML::Node export_data;

        filesystem::path file_path(file_path_string);

        /* AS LIST */
        for(const auto& it : YAML::convert<map<ASNumber, ASClass>>::encode(as_manager->get_AS_list())){
            export_data[it.first] = it.second;
        }
        /* CONNECTION LIST */
        export_data["connection"] = conn_manager->get_all_connections();

        /* MESSAGES LIST */
        export_data["message"] = msg_manager->get_messages();
        //
        // /* SECURITY OBJECTS */
        // export_data["ASPA"] = public_aspa_list;
        // export_data["isec_adopted_as_list"] = isec_adopted_as_list;
        // export_data["public_ProConID"] = public_ProConID;
        //
        std::ofstream fout(file_path_string);
        if (!fout) {
            logger->error("Failed to open the file " + file_path_string + "for writing. (Aborted)");
            return -1;
        }
        //
        YAML::Emitter out;
        out.SetIndent(1);
        out << export_data;
        fout << out.c_str();
        fout.close();

        return 0;
    }
};

/* LOTUS (main) */
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

    void run(void){
        while(auto msg = msg_manager->pop_message()){
            if(msg->type == MessageType::Init){
                for(const Connection& c : conn_manager->get_connections_with(msg->src)){
                    msg->come_from = conn_manager->as_a_is_what_on_c(msg->src, c);
                    ASNumber receive_as;
                    if(msg->src == c.src){
                        receive_as = c.dst;
                    }else /* msg.src == c.dst */{
                        receive_as = c.src;
                    }
                    vector<Message> new_update_message_list = as_manager->get_AS(receive_as)->receive_init(*msg);
                    for(const Message& new_update_msg : new_update_message_list){
                        msg_manager->push_message(new_update_msg);
                    }
                }
            }else if(msg->type == MessageType::Update){
                ASClass* as_class = as_manager->get_AS(*(msg->dst));
                vector<Connection> connection_with_dst = conn_manager->get_connections_with(*(msg->dst));
                optional<Connection> connection = nullopt;
                for(const Connection& c : connection_with_dst){
                    if(msg->src == c.src || msg->src == c.dst){
                        connection = c;
                        break;
                    }
                }
                if(connection == nullopt){
                    logger->error("Fatal logic error: The connection does not exists. (Aborted running)");
                    exit(0);
                }

                msg->come_from = conn_manager->as_a_is_what_on_c(msg->src, *connection);
                optional<RouteDiff> route_diff = as_class->update(*msg);
                if(route_diff == nullopt){
                    // continue;
                }else if(route_diff->come_from == ComeFrom::Customer){
                    for(const Connection& c : connection_with_dst){
                        msg_manager->add_message(
                            MessageType::Update,
                            *(msg->dst),
                            (c.src == *(msg->dst)) ? c.dst : c.src,
                            route_diff->address,
                            route_diff->path
                        );
                    }
                }else if(route_diff->come_from == ComeFrom::Peer || route_diff->come_from == ComeFrom::Provider){
                    for(const Connection& c : connection_with_dst){
                        if(c.type == ConnectionType::Down && c.src == *(msg->dst)){
                            msg_manager->add_message(
                                MessageType::Update,
                                *(msg->dst),
                                c.dst,
                                route_diff->address,
                                route_diff->path
                            );
                        }
                    }
                }
            }
        }
        return;
    }
};
