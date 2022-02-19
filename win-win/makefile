CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2
endif
server: main.cpp  ./CGImysql/sql_connection_pool.cpp\
				  ./config/config.cpp\
				  ./log/log.cpp\
				  ./tcpclient/tcpclient.cpp\
				  ./timer/lst_timer.cpp\
				  ./webserver/webserver.cpp 
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -L /usr/lib64/mysql/ -lmysqlclient
clean:
	rm  -r server
