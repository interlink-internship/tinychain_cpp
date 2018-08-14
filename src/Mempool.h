#include <map>
#include "Transaction.h"
`
using namespace str;
class Mempool{
 public:
  map<string, shared_ptr<Transaction> mempool;
  void add_txn_to_mempool(shared_ptr<Transaction> transaction);
};
