#ifndef USE_CASE_H
#define USE_CASE_H

#include <chrono>
#include <string>

class use_case_t {

public :

   enum completion_t {TO_BE_CONTINUED,
                      COMPLETED,
                      ERROR,
                      STOPPED,   // external
                      TIMEOUT};  // external

   enum stage_t      {NEW,
                      INITIALIZING,
                      INITIALIZED,
                      WORKING,
                      WORKED,
                      FINISHING,
                      FINISHED};

   static std::string completion_str (completion_t complet);
   static std::string stage_str      (stage_t      stage);

   use_case_t (std::string          ucase,
               std::chrono::seconds init_max,
               std::chrono::seconds work_max,
               std::chrono::seconds finish_max);

   std::string          get_name                      ();
   stage_t              get_stage                     ();
   completion_t         get_completion                ();
   std::chrono::seconds get_duration_max              ();
   std::chrono::seconds get_init_stage_duration_max   ();
   std::chrono::seconds get_work_stage_duration_max   ();
   std::chrono::seconds get_finish_stage_duration_max ();
   completion_t         init_stage                    ();
   completion_t         work_iteration                ();
   completion_t         finish_stage                  ();

   void                 set_too_slow                  ();

private :

   std::string          name;
   stage_t              current_stage;
   completion_t         current_completion;
   int                  current_work_iter;

   std::chrono::seconds init_duration_max;
   std::chrono::seconds work_duration_max;
   std::chrono::seconds finish_duration_max;
   std::chrono::seconds iter_duration;
   int                  nb_iterations;

   bool                 work_slowly;

   bool                 enter_stage (stage_t);
   void                 quit_stage  ();

};

#endif

