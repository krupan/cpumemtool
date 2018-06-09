//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// CPU measurements
struct cpu_stat
{
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int soft_irq;
    int steal;
};

class session
{
public:
    session(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
        // hand parsed for now:
        prev_cpu_stat.user = 728425;
        prev_cpu_stat.nice = 117947;
        prev_cpu_stat.system = 286913;
        prev_cpu_stat.idle = 14333965;
        prev_cpu_stat.iowait = 3232;
        prev_cpu_stat.irq = 66665;
        prev_cpu_stat.soft_irq = 31772;
        prev_cpu_stat.steal = 0;
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        std::cout << "start\n";
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

private:
    // adds a null terminator so we can use string functions:
    int input_to_string(char *input)
    {
        for(size_t i = 0; i < max_length; i++)
        {
            if(input[i] == '\n')
            {
                input[i] = 0;
                return 0;
            }
        }
        // if we got here there was no '\n' in input :-(
        return 1;
    }

    // populates everything except non_idle and total, and only
    void get_cpu_stat(struct cpu_stat &cpu_stat)
    {
        std::ifstream stat("/proc/stat");
        // throwaway field:
        std::string cpu;
        // just read the first line, which is all cpus together:
        stat >> cpu >> cpu_stat.user >> cpu_stat.nice >> cpu_stat.system >>
            cpu_stat.idle >> cpu_stat.iowait >> cpu_stat.irq >>
            cpu_stat.soft_irq >> cpu_stat.steal;
    }

    double calc_cpu_percentage(struct cpu_stat &cpu_stat)
    {
        // This is the formula provided here:
        // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux/
        // The author of that clarifies in a comment that the load
        // should be "Percentage of usage from the previous
        // measurement," so that's what this does.
        std::cout << "calc_cpu_percentage\n";
        int actual_prev_idle = prev_cpu_stat.idle + prev_cpu_stat.iowait;
        int actual_idle = cpu_stat.idle + cpu_stat.iowait;

        int prev_non_idle = prev_cpu_stat.user + prev_cpu_stat.nice +
            prev_cpu_stat.system + prev_cpu_stat.irq + prev_cpu_stat.soft_irq +
            prev_cpu_stat.steal;
        int non_idle = cpu_stat.user + cpu_stat.nice + cpu_stat.system +
            cpu_stat.irq + cpu_stat.soft_irq + cpu_stat.steal;

        int prev_total = actual_prev_idle + prev_non_idle;
        int total = actual_idle + non_idle;

        // differentiate: actual value minus the previous one
        double totald = total - prev_total;
        double idled = actual_idle - actual_prev_idle;

        return ((totald - idled) / totald) * 100;
    }

    // returns the new message length
    int get_cpu_load(char *data)
    {
        struct cpu_stat cpu_stat;
        get_cpu_stat(cpu_stat);
        double cpu_percentage = calc_cpu_percentage(cpu_stat);
        // save cpu stat as prev_cpu_stat
        prev_cpu_stat = cpu_stat;
        // TODO: round to some number of decimal places?
        snprintf(data, max_length, "%f\n", cpu_percentage);
        return strlen(data);
    }

    // returns the new message length
    int get_mem(char *data)
    {
        snprintf(data, max_length, "mem usage will go here\n");
        return strlen(data);
    }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            size_t response_len;
            std::cout << "handle_read, bytes_transferred: " << bytes_transferred << "\n";
            int err = input_to_string(data_);
            if(err)
            {
                // TODO: define tcp response for malformed input
                std::cerr << "input needs to end in \\n\n";
                snprintf(data_, empty_length + 1, "\n");
                assert(empty_length == strlen(data_));
                response_len = empty_length;
            }
            std::cout << "data_: " << data_ << "\n";
            if(strncmp("cpu", data_, max_length) == 0)
            {
                response_len = get_cpu_load(data_);
            }
            else if(strncmp("mem", data_, max_length) == 0)
            {
                response_len = get_mem(data_);
            }
            else
            {
                // TODO: define tcp response for invalid command?
                std::cerr << "got command I don't recognize: " << data_ << "\n";
                snprintf(data_, empty_length + 1, "\n");
                assert(empty_length == strlen(data_));
                response_len = empty_length;
            }
            std::cout << "data_ is now: " << data_ << "\n";
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(data_, response_len),
                                     boost::bind(&session::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "handle_write\n";
            std::cout << "data_: " << data_ << "\n";
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cout << "handle_write with error\n";
            delete this;
        }
    }

    tcp::socket socket_;
    enum { max_length = 1024,
           empty_length = 1};
    char data_[max_length];
    struct cpu_stat prev_cpu_stat;
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "server\n";
        start_accept();
    }

private:
    void start_accept()
    {
        std::cout << "start_accept\n";
        session* new_session = new session(io_context_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "handle_accept\n";
            new_session->start();
        }
        else
        {
            std::cout << "handle_accept error\n";
            delete new_session;
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        using namespace std; // For atoi.
        server s(io_context, atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
