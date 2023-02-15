#include <vector>
#include <map>
#include <algorithm>

#ifndef COMMON_H
#define COMMON_H

namespace ns3 {

    enum Messages
    {
        REQUEST_TRANS,  //0
        RESPONSE_TRANS,    //1
        REQUEST_BLOCK,          //2
        RESULT_TRANS,               //3    
    };
}

#endif