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
mongocxx::client conn{mongocxx::uri{"mongodb://labostrex132.iccluster.epfl.ch:27017"}};
struct edge{
   uint32_t src;
   uint32_t dst;
};

auto collection = conn["tesseract"]["myTest2"];


void insertArray(){

    collection.insert_one( make_document(kvp("_id", 5),
                                         kvp("neighbours",
                                             make_array (
                                                     make_document(kvp("3", [] (bsoncxx::builder::basic::sub_document child) {
                                                         child.append(kvp("ts", "2"));
                                                     } ) )

                                             )
                                         )
    ) );
}

void UpdateArrayStreamTest(){
    auto elements = {1,2,3};

    bsoncxx::builder::stream::document docs;
    auto arr = array{};

    for(int i = 0; i < 3;i++) {
        bsoncxx::builder::stream::document doc{};
        doc << std::to_string(i) << open_document << "ts" << 4 << close_document;

        arr << doc;

    }

        docs << "$push" << open_document << "neighbours" <<   open_document << "$each" << arr.view() << close_document <<close_document;

        document filter;
        filter <<"_id" << 5 << finalize;

        collection.update_one(filter.view(), docs.view());




}
void updateArray(){

 auto elements = {1,2,3};


//   collection.update_one(

     auto doc =
         make_document(kvp("_id", 5) ) ;

//        auto doc3 = make_document(kvp("$push",
//                make_array(kvp("neighbours",  [&elements](bsoncxx::builder::basic::document child){
//                        for(auto &element: elements){
//                            child.append(kvp(std::to_string(element), 4));
//                        }
//
//                        }
//
//                        )) ) );

//         auto doc2 = make_document(kvp("$push",
//               make_document(kvp("neighbours" , make_document(kvp("$each",  [&elements](bsoncxx::builder::basic::sub_array sub_arr) {
//                    for(auto& element : elements){
//
//                        sub_arr.append(kvp(std::to_string(element) , 3));
//                    }
//
//               }
//               ) ) ) ) ) );
//                     make_document(kvp("4", [] (bsoncxx::builder::basic::sub_document child){
//
//                           child.append(kvp("ts", "6"));
//
//                           } ) )

//  collection.update_one(doc.view(),doc3.view());
}

void UpdateArrayStream(int ts, edge*buffer, uint32_t buf_size){



    mongocxx::model::update_one* update_ops = ( mongocxx::model::update_one*) malloc(100000 * sizeof( mongocxx::model::update_one));

    for(uint64_t i = 0; i < buf_size;i+=BATCH_SIZE) {

        auto arr = array{};
        uint32_t prev_src = buffer[i].src;
        uint32_t no_ops = 0;


        auto start = std::chrono::steady_clock::now();
        for(uint64_t j = 0; j < BATCH_SIZE && j + i < buf_size; j++) {
            edge e= buffer[j+i];
            bsoncxx::builder::stream::document doc{};
            doc << std::to_string(e.dst) << open_document << "ts" << ts << close_document;

            arr << doc;
            if(e.src != prev_src){
                bsoncxx::builder::stream::document docs;
                docs << "$push" << open_document << "neighbours" <<   open_document << "$each" << arr.view() << close_document <<close_document;
                document filter;
                filter <<"_id" << (int)prev_src;
//                collection.update_one(filter.view(), docs.view());
                update_ops[no_ops++] =  mongocxx::model::update_one(make_document(kvp("_id", (int)prev_src)),docs.extract());
//                docs =  bsoncxx::builder::stream::document{};
                prev_src = e.src;
                arr.clear();
            }
        }
        bsoncxx::builder::stream::document docs;
        docs << "$push" << open_document << "neighbours" <<   open_document << "$each" << arr.view() << close_document <<close_document;
        document filter;
        filter <<"_id" << (int)prev_src;
//                collection.update_one(filter.view(), docs.view());
        update_ops[no_ops++] =  mongocxx::model::update_one(make_document(kvp("_id", (int)prev_src)),docs.extract());
//                docs =  bsoncxx::builder::stream::document{};
        
        arr.clear();
        auto bulk = collection.create_bulk_write();
        for(uint32_t j = 0 ; j < no_ops;j++){
            bulk.append(update_ops[j]);
        }

        auto result = bulk.execute();

        if (!result) {
            return exit(1);
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << "Batch process time: " <<  std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "\n";

    }



//    collection.update_one(filter.view(), docs.view());




}
void updateDoc(int ts, edge* buffer, uint32_t buf_size){
   auto elements = {1,2,3};
   ts = 0;
mongocxx::model::update_one* update_ops = ( mongocxx::model::update_one*) malloc(100000 * sizeof( mongocxx::model::update_one));
   for(uint64_t j = 0; j < buf_size; j+= 100000) {
      ts++;

      auto start = std::chrono::steady_clock::now();
      
      for(uint64_t i = 0; i < 100000;i++){ 

         edge e = buffer[j+ i];
         //collection.update_one(
         auto doc = // make_document(kvp("_id",(int)e.src),

              make_document(kvp("$set", make_document(kvp("neighbours."+std::to_string(e.dst), [&ts] (bsoncxx::builder::basic::sub_document child){
                             child.append(kvp("ts",ts) );
                             } 
                             )
                          )
                       )
                   // ) 
                 ); 

         //mongocxx::model::update_one upsert_op(make_document(kvp("_id", (int)e.src )).view(), doc.view());
         update_ops[i] = mongocxx::model::update_one(make_document(kvp("_id", (int)e.src )), std::move(doc));

      }

      auto bulk = collection.create_bulk_write();
      for(uint32_t i = 0 ; i < 100000;i++){
         bulk.append(update_ops[i]);
      }

      auto result = bulk.execute();

      if (!result) {
         return exit(1);
      }
      auto end = std::chrono::steady_clock::now();
      std::cout << "Batch process time: " <<  std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "\n";
   }

   /* 
      collection.update_one(
      make_document(kvp("_id", 11)),
      make_document(kvp("$set", [&elements,&ts] (){

      for(auto &element: elements) {
      make_document(kvp("neighbours."+std::to_string(element) ,[&ts] (bsoncxx::builder::basic::sub_document child){
      child.append(kvp("ts",ts) );

      } ) );
      } 
      }
      )
      )
      );*/

}

void insertDoc(int ts, edge*buffer, uint32_t buf_size){
   for(int i = 0; i < buf_size; i++){
      bsoncxx::builder::basic::document document{};

      document.append(kvp("_id", (int) buffer[i].src));
      collection.insert_one(document.view());
   }


}

void insertDoc(uint32_t NB_NODES){

   std::vector<bsoncxx::document::value> documents;
   for(int i = 0; i < NB_NODES; i++) {
      documents.push_back(
            bsoncxx::builder::stream::document{} << "_id" << i << bsoncxx::builder::stream::finalize);
   }

   collection.insert_many(documents);

   /*
      for(int i = 0; i < NB_NODES; i++) {


      bsoncxx::builder::basic::document document{};

      document.append(kvp("_id", (int) i));
      documents.push_back(document.) ;  
      } collection.insert_many(documents);
      */
}
void insertDoc(){
   auto elements = {1,2,3};
   int ts = 1;
   std::string field_name("ts");

   bsoncxx::builder::basic::document document{};
   document.append(kvp("_id",11));
   document.append(kvp("neighbours", [&elements](bsoncxx::builder::basic::sub_document child){
            for(auto &element : elements ){
            child.append(kvp(std::to_string(element), [](bsoncxx::builder::basic::sub_document child2){
                     child2.append(kvp("ts", 3));
                     }  ));
            }
            } ));


   collection.insert_one(document.view());



}
int main(int, char**argv) { 

   uint32_t NB_NODES = atol(argv[1]);
   char* fname = argv[2];
    struct stat sb;


   int fd = open(fname, O_RDWR|O_CREAT|O_NOATIME|O_LARGEFILE);
    fstat(fd, &sb);
    uint64_t NB_EDGES =sb.st_size/sizeof(edge);// NB_NODES * 16;
   edge* edges = (struct edge*) mmap(NULL, NB_EDGES* sizeof(edge), PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
   if(edges == MAP_FAILED){
      std::cout << "Failed to mmap \n" ;
      exit(1);
   }
   /*
   //Insert as array of documents 
   collection.insert_one( make_document(kvp("_id", 5), 
   kvp("neighbours",
   make_array ( 
   make_document(kvp("3", [] (bsoncxx::builder::basic::sub_document child) {
   child.append(kvp("ts", "2"));
   } ) )

   ) 
   ) 
   ) );                        
   */

   /* collection.update_one(
      make_document(
      kvp("_id", 0) ),
      make_document(
      kvp("$push", make_document(
      kvp("neighbours" ,  make_document( kvp("ts",3) ) )
      )
      )
      )
      );*/

//   insertArray();
//    insertDoc(NB_NODES);
    UpdateArrayStream(1,edges,NB_EDGES);
 //insertDoc();
   edge* buffer = new edge[2];
   buffer[0].src = 2;
   buffer[0].dst = 3;
   buffer[1].src = 4;
   buffer[1].dst = 5;

//   updateDoc(1,edges,NB_EDGES);

   /*auto cursor = collection.find({});

     for (auto&& doc : cursor) {
     std::cout << bsoncxx::to_json(doc) << std::endl;
     ;
     }*/
}
