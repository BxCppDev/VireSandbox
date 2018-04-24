exec=mut_cond_1 thread_timeout_tst time_tst use_case_tst session_tst

h_lib=time_tools.h use_case.h session.h
cpp_lib=$(h_lib:%.h=%.cpp)

all: ${exec}

thread_timeout_tst : thread_timeout_tst.cpp ${cpp_lib} ${h_lib}
	g++ -std=c++11 $@.cpp ${cpp_lib} -o $@ -lpthread

time_tst : time_tst.cpp ${cpp_lib} ${h_lib}
	g++ -std=c++11 $@.cpp ${cpp_lib} -o $@ -lpthread

mut_cond_1 : mut_cond_1.cpp ${cpp_lib} ${h_lib}
	g++ -std=c++11 $@.cpp ${cpp_lib} -o $@ -lpthread

use_case_tst : use_case_tst.cpp ${cpp_lib} ${h_lib}
	g++ -std=c++11 $@.cpp ${cpp_lib} -o $@ -lpthread

session_tst : session_tst.cpp ${cpp_lib} ${h_lib}
	g++ -std=c++11 $@.cpp ${cpp_lib} -o $@ -lpthread

clean:
	rm -f ${exec}

