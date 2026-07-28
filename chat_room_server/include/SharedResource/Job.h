#ifndef JOB_H
#define JOB_H

 #include "./models/Messages.h"
 #include "./models/User.h"


enum class JobType {
    INSERT_MESSAGE,
    INSERT_FIRST_MESSAGE
   
};

struct Job {
    JobType type;
    Message message;  
    User user;        
  
};

#endif
