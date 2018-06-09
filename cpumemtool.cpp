//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: cpumemtool <host> <port> <command>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints =
            resolver.resolve(tcp::v4(), argv[1], argv[2]);

        tcp::socket s(io_context);
        boost::asio::connect(s, endpoints);

        using namespace std; // For strlen.
        char command[max_length];
        // TODO: will argv[3] always be null terminated?
        strcpy(command, argv[3]);
        size_t command_length = strlen(command);
        // add the needed \n:
        command[command_length] = '\n';
        command[command_length + 1] = 0;
        command_length++;
        boost::asio::write(s, boost::asio::buffer(command, command_length));
        char reply[max_length];
        size_t reply_length = s.read_some(boost::asio::buffer(reply, max_length));
        std::cout.write(reply, reply_length);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
