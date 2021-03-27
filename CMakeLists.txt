#
# Examples and utilities related to Linux namespaces
#


cmake_minimum_required(VERSION 2.6)

PROJECT(ns C)

set(CMAKE_C_FLAGS "-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror -fPIC")

#
# pause
#
ADD_EXECUTABLE(pause pause.c)

#
# sem
#
ADD_EXECUTABLE(sem sem.c)

#
# cmpns
#
ADD_EXECUTABLE(cmpns cmpns.c util.c)

#
# clonens
#
ADD_EXECUTABLE(clonens clonens.c)
TARGET_COMPILE_DEFINITIONS(clonens PRIVATE -D_GNU_SOURCE)

#
# execns
#
ADD_EXECUTABLE(execns execns.c util.c)
TARGET_COMPILE_DEFINITIONS(execns PRIVATE -D_GNU_SOURCE)

#
# shns
#
ADD_EXECUTABLE(shns shns.c)
TARGET_COMPILE_DEFINITIONS(shns PRIVATE -D_GNU_SOURCE)

#
# idns
#
ADD_EXECUTABLE(idns idns.c util.c)
TARGET_COMPILE_DEFINITIONS(idns PRIVATE -D_GNU_SOURCE)

#
# ownerns
#
ADD_EXECUTABLE(ownerns ownerns.c util.c)
TARGET_COMPILE_DEFINITIONS(ownerns PRIVATE -D_GNU_SOURCE)

#
# parentns
#
ADD_EXECUTABLE(parentns parentns.c util.c)
TARGET_COMPILE_DEFINITIONS(parentns PRIVATE -D_GNU_SOURCE)

#
# userns
#
ADD_EXECUTABLE(userns userns.c util.c)
TARGET_COMPILE_DEFINITIONS(userns PRIVATE -D_GNU_SOURCE)

#
# setnshost
#
ADD_EXECUTABLE(setnshost setnshost.c util.c)
TARGET_COMPILE_DEFINITIONS(setnshost PRIVATE -D_GNU_SOURCE)

#
#  udpsrv/udpcli
#
ADD_EXECUTABLE(udpsrv udpsrv.c)
ADD_EXECUTABLE(udpcli udpcli.c)

#
# pid
#
ADD_EXECUTABLE(pid pid.c)

#
# enter_pidns_up
#
ADD_EXECUTABLE(enter_pidns_up enter_pidns_up.c)
TARGET_COMPILE_DEFINITIONS(enter_pidns_up PRIVATE -D_GNU_SOURCE)

#
# getnspid
#
ADD_EXECUTABLE(getnsids getnsids.c util.c)
TARGET_COMPILE_DEFINITIONS(getnsids PRIVATE -D_GNU_SOURCE)

#
# sigwait
#
ADD_EXECUTABLE(sigwait sigwait.c)

#
# rebootns
#
ADD_EXECUTABLE(rebootns rebootns.c)
TARGET_COMPILE_DEFINITIONS(rebootns PRIVATE -D_GNU_SOURCE)

#
# userns_child_exec
#
ADD_EXECUTABLE(userns_child_exec userns_child_exec.c)
TARGET_COMPILE_DEFINITIONS(userns_child_exec PRIVATE -D_GNU_SOURCE)

#
# shns2
#
ADD_EXECUTABLE(shns2 shns2.c util.c)
TARGET_COMPILE_DEFINITIONS(shns2 PRIVATE -D_GNU_SOURCE)

#
# shns3
#
ADD_EXECUTABLE(shns3 shns3.c util.c)
TARGET_COMPILE_DEFINITIONS(shns3 PRIVATE -D_GNU_SOURCE)

#
# semns
#
ADD_EXECUTABLE(semns semns.c util.c)
TARGET_COMPILE_DEFINITIONS(semns PRIVATE -D_GNU_SOURCE)

#
# shmns
#
ADD_EXECUTABLE(shmns shmns.c util.c)
TARGET_COMPILE_DEFINITIONS(shmns PRIVATE -D_GNU_SOURCE)

#
# multi-th
#
ADD_EXECUTABLE(multi-th multi-th.c util.c)
TARGET_LINK_LIBRARIES(multi-th pthread)

#
# producer/consumer
#
ADD_EXECUTABLE(producer prodcons.c util.c)
TARGET_COMPILE_DEFINITIONS(producer PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(producer pthread)

ADD_EXECUTABLE(consumer prodcons.c util.c)
TARGET_COMPILE_DEFINITIONS(consumer PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(consumer pthread)

#
# producer2/consumer2
#
ADD_EXECUTABLE(producer2 prodcons2.c util.c)
TARGET_COMPILE_DEFINITIONS(producer2 PRIVATE -D_GNU_SOURCE)

ADD_EXECUTABLE(consumer2 prodcons2.c util.c)
TARGET_COMPILE_DEFINITIONS(consumer2 PRIVATE -D_GNU_SOURCE)

#
# producer3/consumer3
#
ADD_EXECUTABLE(consumer3 prodcons3.c util.c)
TARGET_COMPILE_DEFINITIONS(consumer3 PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(consumer3 rt)

ADD_EXECUTABLE(producer3 prodcons3.c util.c)
TARGET_COMPILE_DEFINITIONS(producer3 PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(producer3 rt)

ADD_EXECUTABLE(pmsg pmsg.c util.c)
TARGET_COMPILE_DEFINITIONS(pmsg PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(pmsg rt)

ADD_EXECUTABLE(producer4 prodcons4.c util.c)
TARGET_COMPILE_DEFINITIONS(producer4 PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(producer4 rt pthread)

ADD_EXECUTABLE(consumer4 prodcons4.c util.c)
TARGET_COMPILE_DEFINITIONS(consumer4 PRIVATE -D_GNU_SOURCE)
TARGET_LINK_LIBRARIES(consumer4 rt pthread)

ADD_EXECUTABLE(linfo linfo.c)

ADD_EXECUTABLE(fsinfo fsinfo.c)

ADD_EXECUTABLE(reaper reaper.c)

ADD_EXECUTABLE(orphan orphan.c util.c)

ADD_EXECUTABLE(socket socket.c)
