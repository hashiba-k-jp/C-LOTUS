#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <omp.h>

#include "util.h"
#include "lotus.h"
#include "routing_table.h"
#include "as_class.h"

/*
 * This main file is for test.
 */

void test_normal(LOTUS LOTUS){
    std::cout << "/*-----------------------------------------------*/\n";
    std::cout << "/*           attack test without security        */\n";
    std::cout << "/*-----------------------------------------------*/\n";

    /* RUN LOTUS (before attack) */
    LOTUS.run();

    /* GENERATE ATTACK */
    LOTUS.gen_attack(6, 1);

    /* RUN LOTUS (with attack) */
    LOTUS.run();

    /* SHOW ALL AS INFO */
    LOTUS.show_AS_list();

    /* EXPORT DATA TO FILE */
    LOTUS.file_export("./NORMAL_attack.yml");

    return;
}

void test_aspa(LOTUS LOTUS){
    std::cout << "/*-----------------------------------------------*/\n";
    std::cout << "/*           attack test with ASPA/ASPV          */\n";
    std::cout << "/*-----------------------------------------------*/\n";

    /* ADD ASPA DATA */
    LOTUS.add_ASPA(1, {});
    LOTUS.add_ASPA(3, {});
    LOTUS.add_ASPA(4, {});

    /* ENABLE ASPV */
    LOTUS.set_ASPV(1, true, 1);
    LOTUS.set_ASPV(3, true, 1);
    LOTUS.set_ASPV(4, true, 1);

    /* RUN LOTUS (before attack) */
    LOTUS.run();

    /* GENERATE ATTACK */
    // LOTUS.gen_attack(src, tar);
    IPAddress target_address = LOTUS.get_AS(1)->network_address;
    LOTUS.add_messages(MessageType::Update, 6, 3, target_address, Path{1, 2, 6});

    /* RUN LOTUS (with attack) */
    LOTUS.run();

    /* SHOW ALL AS INFO */
    LOTUS.show_AS_list();

    /* EXPORT DATA TO FILE */
    LOTUS.file_export("./ASPA_attack.yml");

    return;
}

void test_isec(LOTUS LOTUS){
    std::cout << "/*-----------------------------------------------*/\n";
    std::cout << "/*           attack test with BGP-iSec           */\n";
    std::cout << "/*-----------------------------------------------*/\n";

    /* ENABLE BGP-ISEC */
    LOTUS.switch_adoption_iSec(1, true, 1);
    LOTUS.switch_adoption_iSec(3, true, 1);
    LOTUS.switch_adoption_iSec(4, true, 1);

    /* ADD PROCONID LIST */
    LOTUS.add_ProConID_all();

    /* RUN LOTUS (before attack) */
    LOTUS.run();

    /* GENERATE ATTACK */
    // LOTUS.gen_attack(src, tar);
    IPAddress target_address = LOTUS.get_AS(1)->network_address;
    LOTUS.add_messages(MessageType::Update, 6, 3, target_address, Path{1, 2, 6});

    /* RUN LOTUS (with attack) */
    LOTUS.run();

    /* SHOW ALL AS INFO */
    LOTUS.show_AS_list();

    /* EXPORT DATA TO FILE */
    LOTUS.file_export("./ISEC_attack.yml");

    return;
}

int main() {
    LOTUS LOTUS;

    /* IMPORT DATA FROM FILE */
    // LOTUS.file_import("FILE_PATH");

    /* GENERATE AS */
    LOTUS.add_AS(ASNumber{1});
    LOTUS.add_AS(ASNumber{2});
    LOTUS.add_AS(ASNumber{3});
    LOTUS.add_AS(ASNumber{4});
    LOTUS.add_AS(ASNumber{5});
    LOTUS.add_AS(ASNumber{6});

    /* CONNECT AS */
    LOTUS.add_connection(ConnectionType::Down, ASNumber{4}, ASNumber{2});
    LOTUS.add_connection(ConnectionType::Down, ASNumber{4}, ASNumber{6});
    LOTUS.add_connection(ConnectionType::Down, ASNumber{3}, ASNumber{5});
    LOTUS.add_connection(ConnectionType::Down, ASNumber{3}, ASNumber{6});
    LOTUS.add_connection(ConnectionType::Down, ASNumber{2}, ASNumber{1});
    LOTUS.add_connection(ConnectionType::Peer, ASNumber{3}, ASNumber{4});

    /* ADD INIT MESSAGE TO ALL AS */
    LOTUS.add_all_init();

    test_normal(LOTUS);
    test_aspa(LOTUS);
    test_isec(LOTUS);

    return 0;
}
