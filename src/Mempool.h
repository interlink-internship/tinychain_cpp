#include <map>
#include <vector>
#include <sstream>
#include <memory>
#include <iostream>
#include <

#include "Transaction.h"

using namespace str;
class Mempool{
 public:
  map<string, shared_ptr<Transaction> mempool;
<<<<<<< HEAD
  void add_txn_to_mempool(shared_ptr<Transaction> transaction);
=======
  vector<shared_ptr<Transaction>> orphan_transactions;

  Mempool(){
    map<string, shared_ptr<Transaction> mempool;
    vector<string, shared_ptr<Transaction>> orphan_transactions;
  };

  /*
  string toString() {
    std::stringstream ss;
  }*/

  void add_txn_to_mempool(shared_ptr<Transaction> transaction,
                          shared_ptr<vector<string>> peer_hostnames);
  {
    if (mempool.find(transaction.id) == mempool.end()){

    }else{
      /*try {
        transaction = transaction.validateTransaction();
      }
      catch(const transactionValidationError& e)
      {
       if(e.to_orphan == nullptr){
       cout << transaction rejected << endl;
     }else{
     cout << transaction << e.to_orphan.id << submitted as orphan << endl;
     orphan_transactions.push_back(transaction);
     }*/

     cout << transaction << transaction.id << added to mempool << endl;

     /*TO ADD

     for (i=0; i << sizeof(peer_hostnames); i++){

     send_to_peer(transaction, peer_hostnames.at(i))

   }




     }
    }











    }
  }
>>>>>>> develop
};
