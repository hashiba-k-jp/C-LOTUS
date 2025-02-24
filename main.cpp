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

int main() {
    LOTUS LOTUS;

    /* IMPORT DATA FROM FILE */
    // LOTUS.file_import("FILE_PATH");

    /* GENERATE AS */
    vector<ASNumber> as_number_list = {1, 10, 11, 12, 100, 200};
    for(ASNumber& asn : as_number_list){
        LOTUS.add_AS(ASNumber(asn));
    }

    /* CONNECT AS */
    vector<tuple<ConnectionType, ASNumber, ASNumber>> connection_list = {
        {ConnectionType::Down,   1,  10},
        {ConnectionType::Peer,  10,  11},
        {ConnectionType::Peer,  11,  12},
        {ConnectionType::Down,  10, 100},
        {ConnectionType::Down,  10, 200},
        {ConnectionType::Down,   1, 200},
    };
    for(const auto& c : connection_list){
        LOTUS.add_connection(get<0>(c), get<1>(c), get<2>(c));
    }

    /* ADD INIT MESSAGE TO ALL AS */
    LOTUS.add_all_init();

    /* RUN LOTUS (before attack) */
    LOTUS.run();

    /* GENERATE ATTACK */
    LOTUS.gen_attack(100, 1);

    /* RUN LOTUS (with attack) */
    LOTUS.run();

    /* SHOW ALL AS INFO */
    LOTUS.show_AS_list();

    /* EXPORT DATA TO FILE */
    // LOTUS.file_export("FILE_PATH");

    return 0;
}
