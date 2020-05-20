#include "mongoDriver.hpp"
#include <iostream>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/sub_document.hpp>
#include <bsoncxx/builder/basic/impl.hpp>

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>


#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <chrono>


#define BATCH_SIZE 100000
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::sub_array;


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::array;

mongocxx::instance inst{};
mongocxx::pool pool{mongocxx::uri{"mongodb://labostrex132.iccluster.epfl.ch:27017"}};

//struct edge_full{
//   uint32_t src;
//   uint32_t dst;
//};
//
//struct edge_ts{
//    uint32_t src;
//    uint32_t dst;
//    uint32_t ts;
//};
//auto collection = conn["tesseract"]["lj-dir"];
//mongocxx::client* clients[56];

//void initClientCollection(){
//    for(int i = 0; i < 56; i++) {
//        auto c = pool.acquire();
//        clients[i] = (c);
//    }
//
//}
//uint64_t db_time = 0;

//uint64_t db_time = 0;
double db_time[56];
void init(){
    for(int i =0 ;i  < 56;i++)
        db_time[i] = 0;
}
uint32_t queryCollection(uint32_t src_id, edge_ts* result, uint64_t v_offset, uint32_t degree,int ts_search ,mongocxx::client& c, int tid){
    auto collection = c["tesseract"]["lj-dir"];
    auto s = std::chrono::steady_clock::now();
    mongocxx::cursor curr =
            collection.find(make_document(kvp("_id",int(src_id)))); //<<finalize);

//    if( curr.begin() == curr.end()) return degree;
    for(auto&& doc: curr){
        if(!doc["neighbours"]) continue;
        bsoncxx::array::view subarray{doc["neighbours"].get_array().value};
        if(subarray.length()  == 0 ) continue;

//        result = (edge_ts*) calloc(subarray.length(), sizeof(edge_ts));
        uint64_t idx = v_offset + degree;
        int i = 0;

        for(uint32_t _idx = degree; _idx < subarray.length(); _idx++){
            bsoncxx::array::element msg = subarray[_idx];
//        for (bsoncxx::array::element msg : subarray) {
            if(!msg) {
                auto e = std::chrono::steady_clock::now();
                std::chrono::duration<double> diff = e - s;

               db_time[tid] += diff.count();
                return degree;
            }
            bsoncxx::document::view subdoc{msg.get_document()};
            bsoncxx::document::element el =  (*subdoc.begin());



            bsoncxx::document::view val_wrap {el.get_document()};
            uint32_t ts =  val_wrap["ts"].get_int32().value;
            if(ts > ts_search ) {
                auto e = std::chrono::steady_clock::now();

                std::chrono::duration<double> diff = e - s;

                db_time[tid] += diff.count();
                return degree;
            }
            result[idx].ts = ts;
            std::string keystr(el.key());
            result[idx].src = src_id;
            result[idx].dst = atol(keystr.c_str());

            idx++;
            degree++;

        }


        auto e = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = e - s;

        db_time[tid] += diff.count();
    }
    return degree;


}

void printDBTIME(){
    double x = 0;
    for(int i = 0; i < 56;i++){
        printf(" %.3f ", db_time[i]);
        x+= db_time[i];
    }
    printf("\n[TIME] DB TIME %.3f\n",x);
}
