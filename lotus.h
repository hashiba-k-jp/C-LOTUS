#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "util.h"
#include "as_class.h"

using namespace std;

class LOTUS{
protected:
    ASCLassList as_class_list;
    queue<Message> message_queue;
    vector<Connection> connection_list;
    map<ASNumber, vector<ASNumber>> public_aspa_list;
};
