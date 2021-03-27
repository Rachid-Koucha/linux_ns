#!/bin/sh


if [ $# -ne 1 ]
then
  echo Usage: $0 container_name >&2
  exit 1
fi

for cg in blkio cpu,cpuacct cpuset devices freezer hugetlb memory net_cls,net_prio perf_event pids rdma
do
  if [ $cg = "cpuset" ]
  then
    echo 1 > /sys/fs/cgroup/${cg}/cgroup.clone_children
  fi

  if [ ! -d /sys/fs/cgroup/${cg}/lxc ]
  then
    mkdir /sys/fs/cgroup/${cg}/lxc
  fi
  chown -R rachid /sys/fs/cgroup/${cg}/lxc
  chgrp -R rachid /sys/fs/cgroup/${cg}/lxc

  if [ ! -d /sys/fs/cgroup/${cg}/lxc/$1 ]
  then
    mkdir /sys/fs/cgroup/${cg}/lxc/$1
  fi

  chown -R rachid /sys/fs/cgroup/${cg}/lxc/$1
  chgrp -R rachid /sys/fs/cgroup/${cg}/lxc/$1

  #
  #
  #
  if [ ! -d /sys/fs/cgroup/devices/user.slice/lxc ]
  then
    mkdir /sys/fs/cgroup/devices/user.slice/lxc
  fi
  chown -R rachid /sys/fs/cgroup/devices/user.slice/lxc
  chgrp -R rachid /sys/fs/cgroup/devices/user.slice/lxc

  if [ ! -d /sys/fs/cgroup/devices/user.slice/lxc/$1 ]
  then
    mkdir /sys/fs/cgroup/devices/user.slice/lxc/$1
  fi
  chown -R rachid /sys/fs/cgroup/devices/user.slice/lxc/$1
  chgrp -R rachid /sys/fs/cgroup/devices/user.slice/lxc/$1


  if [ ! -d /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc ]
  then
    mkdir /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc
  fi
  chown -R rachid /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc
  chgrp -R rachid /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc

  if [ ! -d /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc/$1 ]
  then
    mkdir /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc/$1
  fi
  chown -R rachid /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc/$1
  chgrp -R rachid /sys/fs/cgroup/pids/user.slice/user-1000.slice/user@1000.service/lxc/$1

  if [ ! -d /sys/fs/cgroup/memory/user.slice/user-1000.slice/user@1000.service/lxc ]
  then
    mkdir /sys/fs/cgroup/memory/user.slice/user-1000.slice/user@1000.service/lxc
  fi
  chown -R rachid /sys/fs/cgroup/memory/user.slice/user-1000.slice/user@1000.service/lxc
  chgrp -R rachid /sys/fs/cgroup/memory/user.slice/user-1000.slice/user@1000.service/lxc


done
