#ifndef __MONGO_DRIVER_HPP__
#define __MONGO_DRIVER_HPP__

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <chrono>

#include"graph.hpp"
#include <mongocxx/pool.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#define BATCH_SIZE 100000

extern mongocxx::instance inst;///{};
extern mongocxx::pool pool;
//{mongocxx::uri{"mongodb://labostrex132.iccluster.epfl.ch:27017"}};
#include "../graph.hpp"

//

//void
//extern struct edge_ts;
extern void printDBTIME();
extern void init();
extern void initClientCollection();
 extern uint32_t queryCollection(uint32_t src_id, edge_ts* result, uint64_t offsets, uint32_t degree,int ts_search, mongocxx::client&c, int tid);

/*{

    mongocxx::cursor curr =
            collection.find(make_document(kvp("_id",int(src_id)))); //<<finalize);


    for(auto&& doc: curr){
        if(!doc["neighbours"]) continue;
        bsoncxx::array::view subarray{doc["neighbours"].get_array().value};
        if(subarray.length()  == 0 ) continue;

        result = (edge_ts*) calloc(subarray.length(), sizeof(edge_ts));
        uint64_t idx = offsets + degree;
        int i = 0;
        for (bsoncxx::array::element msg : subarray) {

            bsoncxx::document::view subdoc{msg.get_document()};
            bsoncxx::document::element el =  (*subdoc.begin());



            bsoncxx::document::view val_wrap {el.get_document()};
            uint32_t ts =  val_wrap["ts"].get_int32().value;
            if(ts > ts_search) return;
            result[idx].ts = ts;
            std::string keystr(el.key());
            result[idx].src = src_id;
            result[idx].dst = atol(keystr.c_str());

            idx++;
            degree++;

        }



    }

}*/

#endif