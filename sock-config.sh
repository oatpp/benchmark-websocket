#!/bin/bash

sysctl -w fs.file-max=11000000
sysctl -w fs.nr_open=11000000
ulimit -n 11000000
sysctl -w net.ipv4.tcp_mem="100000000 100000000 100000000"
sysctl -w net.core.somaxconn=65535
sysctl -w net.ipv4.tcp_max_syn_backlog=65535

sysctl -w net.ipv4.ip_local_port_range="1025 65535"