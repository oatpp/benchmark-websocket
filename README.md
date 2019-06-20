# benchmark-websocket [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.benchmark-websocket?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=19&branchName=master)

About recent results read here: [5 Million WebSockets](https://oatpp.io/benchmark/websocket/5-million/)

See also:
- [oatpp repo](https://github.com/oatpp/oatpp)  
- [oatpp-websocket repo](https://github.com/oatpp/oatpp-websocket)

## Overview

This repo contains both server and client used in oatpp-websocket-benchmarks. 
Both server and client are implemented based on oatpp Async API and [oatpp-coroutines](https://oatpp.io/docs/oatpp-coroutines/)

### Repo structure

```
|- client/
|    |- src/                    // client source code
|    |- CMakeLists.txt          // client CMakeLists.txt
|
|- server/
|    |- src/                    // server source code
|    |- CMakeLists.txt          // server CMakeLists.txt
|
|- prepare.sh                   // prepare script - will clone oatpp and oatpp-websocket. build and install.
|- sock-config.sh               // configure required sysctl(s)
```

## Reproduce latest benchmark

Create two `n1-highmem-16 (16 vCPUs, 104 GB memory) - Debian GNU/Linux 9` instances in same VPC on Google Cloud.

### Execute the following commands for both instances (SSH).

- Install git

```bash
$ sudo su
$ apt-get update
...
$ apt-get install -y git
...
```

- Clone [benchmark-websocket repo](https://github.com/oatpp/benchmark-websocket) and `cd` to repo folder 

```bash
$ git clone https://github.com/oatpp/benchmark-websocket
...
$ cd benchmark-websocket
```

- Install `oatpp` and `oatpp-websocket` modules (run ./prepare.sh script).

```bash
$ ./prepare.sh
```

- Configure environment (run ./sock-config-5m.sh script)

```bash
$ ./sock-config-5m.sh
$ ulimit -n 6000000
```

### Build and Run Server

Commands for server instance only:

- Build server

```bash
$ cd server/build/
$ cmake ..
$ make
```

- Run server

```bash
$ ./wsb-server-exe --tp 16 --tio 8 --pc 500
```
where:  
`--tp` - number of data-processing threads.  
`--tio` - number of I/O workers.  
`--pc` - number of ports to listen to.

### Build and Run Client

Commands for client instance only:

- Build client

```bash
$ cd client/build/
$ cmake ..
$ make
```

- Run client

```bash
$ ./wsb-client-exe --tp 16 --tio 8 -h <server-private-ip> --socks-max 5000000 --socks-port 10000 --si 1000 --sf 30 --pc 500
```
where:  
`--tp` - number of data-processing threads.  
`--tio` - number of I/O workers.  
`-h <server-private-ip>` - substitute **private-ip** of server instance here.  
`--socks-max` - how many client connections to establish.  
`--socks-port` - how many client connections per port.  
`--si 1000 --sf 30` - control how fast clients will connect to server. Here - each `1000` iterations sleep for `30` milliseconds.  
`--pc` - number of available server ports to connect to. 

**Note** - clients will not start load until all clients are connected.  
**Note** - client app will fail with assertion if any of clients has failed.

## Links

- [Latest benchmark](https://oatpp.io/benchmark/websocket/5-million/)
- [oatpp-websocket repo](https://github.com/oatpp/oatpp-websocket)
- [oatpp repo](https://github.com/oatpp/oatpp)
