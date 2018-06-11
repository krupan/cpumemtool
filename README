# CPU/MEM Tool

`cpumemd` is a simple daemon that listens on a TCP port and responds to two commands:

`cpu`

`mem`

When it receives `cpu` it reports cpu usage as a percentage.  When it receives `mem` it reports memory usage.

There is a client, `cpumemtool`, that can connect and send either of the above commands, but you could just as easily use netcat too :-)

## Building

You will need to have boost C++ libraries installed, and then simply run `make`.  The resulting binaries will be in the `bin` directory.

This has been tested with g++ 8.1.0 on a reasonably up-to-date (as of 2018-06-09) Archlinux system.

## Running

Run the daemon like so:

```
cpumemd <port>
```

There are several ways to daemonize it, for example, using nohup, daemonize, systemd, etc.

Run the client like so:

```
cpumemtool <hostname> <port> <cmd>
```

Hostname and port should be the hostname and port where `cpumemd` is running, cmd should be one of `cpu` or `mem`.  Any other command will just be ignored.