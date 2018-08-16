#include <map>
#include <vector>
#include <sstream>
#include <memory>
#include <iostream>
#include <exception>

#include "Transaction.h"

using namespace str;
class Mempool{
 public:
  //mempool[txid]
  map<string, shared_ptr<Transaction> mempool;
  vector<shared_ptr<Transaction>> orphan_transactions;

  Mempool(){
    map<string, shared_ptr<Transaction> mempool;
    vector<string, shared_ptr<Transaction>> orphan_transactions;
  };


  string toString() {
    std::stringstream ss;
    map<string, int>::iterator it;

    ss << "["

    //auto it でもいいかも
    for ( it = mempool.begin(); it != mempool.end(); it++ )
    {
        ss  << "{"
            << "\"txid\":["
            << it->first  // string (key)
            << "],"
            << "\"Transaction\":["
            << it->second->toString() // string's value
            << "]"
            << "}"
            << (it != mempool.end()) ? "," : "";
    }

    ss << "]";

    return ss.str();
}


  void addTxntoMempool(shared_ptr<Transaction> transaction,
                          shared_ptr<vector<string>> peer_hostnames);
  {
    if (mempool.find(transaction.id) == mempool.end()){

    }else{
      /*try {
        transaction = transaction.validateTransaction();
      }
      catch(transactionValidationError& e)
      {
       if(e.to_orphan == nullptr){
       cout << transaction rejected << endl;
     }else{
     cout << transaction << e.to_orphan.id << submitted as orphan << endl;
     orphan_transactions.push_back(transaction);
     }*/

     cout << transaction << transaction.id << added to mempool << endl;

     /*TO ADD

     for (i=0; i << peer_hostnames.size(); i++){

     send_to_peer(transaction, peer_hostnames.at(i));

   }*/

     }
    }

    shared_ptr<UnspentTxOut> findUtxoinMempool(shared_ptr<TxIn>&& txin){
      string txid = txin.toSpend.txid;
      int txoutidx = txin.toSpend.txouIdx;

      try{
        txout = this->mempool[txid]->txouts[idx];
      }
      catch(const exception& e)
      {
      cout << "Could not find utxo in mempool for" << txid << endl;
      return None;
      }

      //isCoinbase = false, height = -1
      return UnspentTxOut(txout.value, txout.toAddr, txid,
                          txoutidx, false, -1);

    }

    }
  }
};
