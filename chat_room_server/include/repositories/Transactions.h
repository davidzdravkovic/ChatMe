#ifndef TRANSACTION_H
#define TRANSACTION_H
#include "../DataBase/DataBase.h"

#include <pqxx/pqxx>

class Transaction {
    DataBasePool& pool;
    pqxx::connection& connection;
    pqxx::work tx;
    bool finished = false;

public:
    explicit Transaction(DataBasePool& p)
       :  pool(p),
          connection(pool.giveConn()),
          tx(connection) {}

    void commit();
    pqxx::work& get() { return tx; }

        
    ~Transaction() {
    try {
        if (!finished) {
            tx.abort();
        }
    } catch (...) {
    } 
    pool.releaseConn(connection);
}
};

#endif
