#ifndef MESSAGEREACTION_H
#define MESSAGEREACTION_H

#include <string>

struct MessageReaction {
    int messageId = 0;
    int userId = 0;
    std::string reaction;
};

#endif
