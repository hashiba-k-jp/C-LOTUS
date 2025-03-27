#include "SOLID_lotus.hpp"

/*
 * This main file is for test.
 */

int main() {
    Logger logger;
    ASManager asManager(&logger);
    ConnectionManager connManager(&logger, &asManager);
    MessageProcessor msgProcessor(&asManager, &logger);
    FileManager fileManager(&asManager, &connManager, &msgProcessor, &logger);
    NEW_LOTUS NEW_LOTUS(&asManager, &connManager, &msgProcessor, &logger);

    asManager.add_AS(1);
    asManager.add_AS(10);
    asManager.add_AS(11);
    asManager.add_AS(12);
    asManager.add_AS(100);
    asManager.add_AS(200);

    connManager.add_connection(ConnectionType::Down, 1, 10);
    connManager.add_connection(ConnectionType::Peer, 10, 11);
    connManager.add_connection(ConnectionType::Peer, 11, 12);
    connManager.add_connection(ConnectionType::Down, 10, 100);
    connManager.add_connection(ConnectionType::Down, 10, 200);

    msgProcessor.add_all_init();

    NEW_LOTUS.run();
    asManager.show_AS_list();

    return 0;
}
