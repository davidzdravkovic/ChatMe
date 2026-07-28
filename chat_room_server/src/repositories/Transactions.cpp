#include "../include/repositories/Transactions.h"



void Transaction::commit() {
 

    if (!finished) {
        tx.commit();
        finished = true;
    }
 

}
// void Transaction::commit()
// {
//     if (!finished) {
//         tx.commit();
//         finished = true;
//     }
// }

// void Transaction::rollback()
// {
//     if (!finished) {
//         tx.abort();
//         finished = true;
//     }
// }

// Transaction::~Transaction()
// {
  
//     if (!finished) {
//         try {
//             tx.abort();
//         } catch (...) {
           
//         }
//     }
// }
