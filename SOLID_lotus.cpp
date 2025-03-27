#include "SOLID_lotus.hpp"

/* LOTUS (main) */
void NEW_LOTUS::run(void){
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
