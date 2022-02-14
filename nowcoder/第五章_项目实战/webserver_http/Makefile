CXX ?= g++
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -02
endif
server: main.cpp ./timer/lst_timer.cpp ./http/http_conn.cpp ./lock/locker.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp ./threadpool/threadpool.cpp webserver.cpp config.cpp
	$(CXX) -o server $^ $(CXXFLAGS) -lpthread -L /usr/lib64/mysql/ -lmysqlclient  #-L /usr/lib64/mysql/
clean:
	rm -r server 