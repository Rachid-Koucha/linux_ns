# Articles and accompanying example programs about linux namespaces

[1 Introduction](#1_Introduction)  
[2 Maintainers](#2_Maintainers)  
[3 Download ](#3_Download)  
[4 Build ](#4_Build)  
[5 Covers ](#5_Covers)  
[6 Description of the utilities](#6_Utils)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.1 lxc-pid](#6_1_lxc_pid)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.2 lxc-start2](#6_2_lxc_start2)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.3 cmpns](#6_3_cmpns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.4 clonens](#6_4_clonens)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.5 execns](#6_5_execns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.6 shns](#6_6_shns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.7 shns2](#6_7_shns2)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.8 shns3](#6_8_shns3)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.9 idns](#6_9_idns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.10 ownerns](#6_10_ownerns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.11 parentns](#6_11_parentns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.12 userns](#6_12_userns)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.13 linfo](#6_13_linfo)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.14 setnshost](#6_14_setnshost)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.15 fsinfo](#6_15_fsinfo)  

## <a name="1_Introduction"></a>1 Introduction

The examples programs available here illustrate the articles serie **published in french** in [GNU Linux Magazine France](https://boutique.ed-diamond.com/) presenting the Linux namespaces in details from user to kernel space. They are written with a minimum number of lines to make them easier to understand. Hence, the source code may appear not robust (no checks on the return codes, basic algorithmic...).

The articles are first published in the magazine. The list is located [here](https://connect.ed-diamond.com/auteur/Koucha-Rachid). They become available for free after 4 to 6 months and a **pdf** copy is stored in the _articles_ sub-directory:

- [Les namespaces ou l'art de se d&eacute;multiplier](https://connect.ed-diamond.com/GNU-Linux-Magazine/GLMF-239/Les-namespaces-ou-l-art-de-se-demultiplier);
- [Les utilitaires relatifs aux namespaces](https://connect.ed-diamond.com/GNU-Linux-Magazine/GLMF-240/Les-utilitaires-relatifs-aux-namespaces);
- [Les structures de donn&eacute;es des namespaces dans le noyau](https://connect.ed-diamond.com/GNU-Linux-Magazine/GLMF-243/les-structures-de-donnees-des-namespaces-dans-le-noyau);
- [Le fonctionnement des namespaces dans le noyau](https://connect.ed-diamond.com/GNU-Linux-Magazine/glmf-245/le-fonctionnement-des-namespaces-dans-le-noyau);
- [Identit� multiple avec le namespace user](https://connect.ed-diamond.com/GNU-Linux-Magazine/glmf-246/identite-multiple-avec-le-namespace-user);
- [A la d�couverte des namespaces mount et uts](https://connect.ed-diamond.com/GNU-Linux-Magazine/glmf-247/a-la-decouverte-des-namespaces-mount-et-uts);
- [Les bizarreries de l'isolation des IPC](https://connect.ed-diamond.com/GNU-Linux-Magazine/glmf-250/les-bizarreries-de-l-isolation-des-ipc);
- [Les namespaces network et pid](https://connect.ed-diamond.com/GNU-Linux-Magazine/glmf-256/les-namespaces-network-et-pid);
- [Le namespace cgroup ne sera pas le dernier de la lign&eacute;e](https://connect.ed-diamond.com/gnu-linux-magazine/glmf-258/le-namespace-cgroup-ne-sera-pas-le-dernier-de-la-lignee).

## <a name="2_Maintainers"></a>2 Maintainers

For any question or remark, please contact [me](mailto:rachid.koucha@gmail.com)

## <a name="3_Download"></a>3 Download

The source code is available on github. Use the following command to clone it:
```
$ git clone https://github.com/Rachid-Koucha/linux_ns.git
```
The example programs are at the top level. The articles available for free are in the _articles_ sub-directory in **pdf** format.

## <a name="4_Build"></a>4 Build

The build is done with cmake:
```
$ cmake .
-- The C compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
[...]
$ make
Scanning dependencies of target reaper
[  1%] Building C object CMakeFiles/reaper.dir/reaper.c.o
[  2%] Linking C executable reaper
[  2%] Built target reaper
[...]
```

To clean the built files:
```
$ make clean
```

## <a name="5_Covers"></a>5 Covers

The covers of the published articles are:

- Issue#239 of July/August 2020:
<p align="left"><a href="articles/linux_namespaces_01.pdf"><img src="covers/article_cover_01.png"></a></p>

- Issue#240 of September 2020:
<p align="left"><a href="articles/linux_namespaces_02.pdf"><img src="covers/article_cover_02.png"></a></p>

- Issue#243 of December 2020:
<p align="left"><a href="articles/linux_namespaces_03.pdf"><img src="covers/article_cover_03.png"></a></p>

- Issue#245 of February 2021:
<p align="left"><a href="articles/linux_namespaces_04.pdf"><img src="covers/article_cover_04.png"></a></p>

- Issue#246 of March 2021:
<p align="left"><a href="articles/linux_namespaces_05.pdf"><img src="covers/article_cover_05.png"></a></p>

- Issue#247 of April 2021:
<p align="left"><a href="articles/linux_namespaces_06.pdf"><img src="covers/article_cover_06.png"></a></p>

- Issue#250 of July-August 2021:
<p align="left"><a href="articles/linux_namespaces_07.pdf"><img src="covers/article_cover_07.png"></a></p>

- Issue#256 of March-April 2022:
<p align="left"><img src="covers/article_cover_08.png"></p>

- Issue#258 of July-August 2022:
<p align="left"><img src="covers/article_cover_09.png"></p>

## <a name="6_Utils"></a>6 Description of the utilities

The following simple utilities are used to illustrate the articles serie concerning the Linux namespaces. They are not designed to be used in a larger scope although they may provide some hints to write some more elaborated tools around the Linux namespaces.

### <a name="6_1_lxc_pid"></a>6.1 lxc-pid

`lxc-pid` is a shell script which retrieves and displays the pid of the _init_ process of a LXC container.

For example:
```none
$ sudo lxc-start -n bbox
$ sudo lxc-ls -f
$ sudo lxc-ls -f
NAME    STATE   AUTOSTART GROUPS IPV4       IPV6 UNPRIVILEGED 
bbox    RUNNING 0         -      10.0.3.230 -    false     
$ sudo ./lxc-pid bbox
14642
$ sudo lxc-stop -n bbox
$ $ sudo ./lxc-pid bbox
Unknown container bbox
```
### <a name="6_2_lxc_start2"></a>6.2 lxc-start2

`lxc-start2` is a shell script to start a LXC container which displays its name in its shell prompt.

For example, we start a container named _bbox_ (of course it is supposed to have been created):
```none
$ sudo ./lxc-start2 bbox
$ sudo lxc-attach -n bbox
$ sudo lxc-console -n bbox -t 0

Connected to tty 0
Type <Ctrl+a q> to exit the console, <Ctrl+a Ctrl+a> to enter Ctrl+a itself

BusyBox v1.30.1 (Ubuntu 1:1.30.1-4ubuntu4) built-in shell (ash)
Enter 'help' for a list of built-in commands.

bbox# <CTRL>+<a>+<q>
$
```

### <a name="6_3_cmpns"></a>6.3 cmpns

`cmpns` compares the namespaces of two processes which identifiers are passed as parameters.

In this example, we compare the namespaces of the current shell with the namespaces of the _init_ process of the host:
```none
$ sudo ./cmpns $$ 1
cgroup is equal
ipc is equal
mnt is equal
net is equal
pid is equal
user is equal
uts is equal
```

In this example, we compare the namespaces of the current shell with the namespaces of the _init_ process of a LXC container:
```none
$ sudo lxc-start -n bbox
$ sudo ./lxc-pid bbox
14857
$ sudo ./cmpns $$ 14857
cgroup is different
ipc is different
mnt is different
net is different
pid is different
user is equal
uts is different
```
Privileged containers run in the same user namespaces as the host. Hence, the equality in the above display.

### <a name="6_4_clonens"></a>6.4 clonens

`clonens` creates a new process in new namespaces. The latters are passed as
parameters with their symbolic names (_cgroup_, _ipc_, _mnt_, _net_, _pid_, _user_ and _uts_).

For example, in one terminal, we launch a process in new pid and uts namespaces. The launched process is suspended on `pause()`:
```none
$ sudo ./clonens pid uts
Created process 14950 in requested namespaces
```
In another terminal, we compare the namespaces of the new process with the namespaces of the
current shell to verify that the _uts_ and _pid_ namespaces are different:
```none
$ sudo ./cmpns 14950 $$
cgroup is equal
ipc is equal
mnt is equal
net is equal
pid is different
user is equal
uts is different
```
### <a name="6_5_execns"></a>6.5 execns

`execns` executes a program in one or more namespaces of a running process.

In this example, we run a shell in the namespaces of the _init_ process of a running LXC container:
```none
$ sudo ./lxc-pid bbox
14857
$ PS1="\h#\x20" sudo -E ./execns 14857 /bin/sh
Moved into namespace ipc
Moved into namespace pid
Moved into namespace net
Moved into namespace uts
Moved into namespace cgroup
Moved into namespace mnt


BusyBox v1.30.1 (Ubuntu 1:1.30.1-4ubuntu4) built-in shell (ash)
Enter 'help' for a list of built-in commands.

bbox# exit
program's status: 0 (0x0)
```
### <a name="6_6_shns"></a>6.6 shns

`shns` creates a shell in the namespaces specified as parameters.

For example:
```none
$ PS1="SHNS# " sudo -E ./shns
New namespace 'ipc'
New namespace 'pid'
New namespace 'net'
New namespace 'user'
New namespace 'uts'
New namespace 'cgroup'
New namespace 'mnt'
SHNS# date
dim. 28 mars 2021 18:57:37 CEST
SHNS# exit
/bin/sh: 5: Cannot set tty process group (No such process)
program's status: 0 (0x0)
```
### <a name="6_7_shns2"></a>6.7 shns2

Like `shns`, `shns2` creates a shell in the namespaces specified as parameters. But it prompts the user just before the execution of the shell.

For example:
```none
$ PS1='SHNS$ ' ./shns2 ipc uts mnt user
New namespace 'ipc'
New namespace 'user'
New namespace 'uts'
New namespace 'mnt'
Process#15706 go forward ([Y]/N)? 
```
### <a name="6_8_shns3"></a>6.8 shns3

`shns3` is the synthesis of `shns` and `shns2`. The usage is:
```none
$ ./shns3 -h
Usage: shns3 [-h] [-d level] [-n nsname] [-u uidmap] [-g gidmap] [-s path]
  -h|--help            : This help
  -h|--debug level     : Debug level
  -n|--nspc nsname     : Create namespace
                         'nsname' is: cgroup|ipc|net|mnt|pid|user|uts
  -u|--umap uidmap     : User id mapping
                         'uidmap' is 'uid loweruid count'
  -g|--gmap gidmap     : Group id mapping
                         'gidmap' is 'gid lowergid count'
  -s|--shell path      : Execute shell
                         'path' is '/bin/sh' by default
```
The namespaces to create are passed with the `--nspc` option. The user ids and group ids mappings are respectively passed through the `--umap` and `--gmap` options. For example:
```none
$ ./shns3 -u '0 1000 1' -u '1 100000 100' -g '0 1000 1' -g '1 100000 100' -d 1
DEBUG_1 (main#358): New namespace 'ipc'
DEBUG_1 (main#358): New namespace 'pid'
DEBUG_1 (main#358): New namespace 'net'
DEBUG_1 (main#358): New namespace 'user'
DEBUG_1 (main#358): New namespace 'uts'
DEBUG_1 (main#358): New namespace 'cgroup'
DEBUG_1 (main#358): New namespace 'mnt'
DEBUG_1 (main#409): Running 'newuidmap 7565 0 1000 1 1 100000 100'...
DEBUG_1 (main#424): Running 'newgidmap 7565 0 1000 1 1 100000 100'...
# id
uid=0(root) gid=0(root) groups=0(root),65534(nogroup)
#
```
### <a name="6_9_idns"></a>6.9 idns

`idns` displays the identifiers of one or more namespaces of a given process. By default, all
the namespaces are considered.

For example, to display the identifiers of the namespaces of the current shell:
```none
$ sudo ./idns $$
/proc/7622/ns/cgroup [Device,Inode]: [4,4026531835]
/proc/7622/ns/ipc [Device,Inode]: [4,4026531839]
/proc/7622/ns/mnt [Device,Inode]: [4,4026531840]
/proc/7622/ns/net [Device,Inode]: [4,4026531992]
/proc/7622/ns/pid [Device,Inode]: [4,4026531836]
/proc/7622/ns/uts [Device,Inode]: [4,4026531838]
/proc/7622/ns/user [Device,Inode]: [4,4026531837]
$ sudo ./idns $$ uts ipc
/proc/7622/ns/uts [Device,Inode]: [4,4026531838]
/proc/7622/ns/ipc [Device,Inode]: [4,4026531839]
```
### <a name="6_10_ownerns"></a>6.10 ownerns

`ownerns` displays the user_ns owning a the namespaces of a given process.

For example, we display the user_ns owning the namespaces of the current shell:
```none
$ sudo ./ownerns $$
/proc/7622/ns/cgroup belongs to [Device,Inode]: [4,4026531837]
/proc/7622/ns/ipc belongs to [Device,Inode]: [4,4026531837]
/proc/7622/ns/mnt belongs to [Device,Inode]: [4,4026531837]
/proc/7622/ns/net belongs to [Device,Inode]: [4,4026531837]
/proc/7622/ns/pid belongs to [Device,Inode]: [4,4026531837]
/proc/7622/ns/uts belongs to [Device,Inode]: [4,4026531837]
ERROR@main#87: ioctl(/proc/7622/ns/user, NS_GET_USERNS): 'Operation not permitted' (1)
```

### <a name="6_11_parentns"></a>6.11 parentns

`parentns` displays the parent namespace of a the namespaces of a given process. This works only for
hierarchical namespaces (i.e. _pid_ns_ and _user_ns_).

For example, we display the parent user_ns of the _init_ process of a running LXC container:
```none
$ sudo ./lxc-pid bbox
14857
$ sudo ./parentns 14857 pid user
/proc/14857/ns/pid is child of [Device,Inode]: [4,4026531836]
ERROR@main#89: ioctl(/proc/14857/ns/user, NS_GET_PARENT): 'Operation not permitted' (1)
```
In the above display, there is an error for the user_ns as a privileged container runs in the user_ns as the host. And it is not possible to get the parent user namespace of the initial user_ns.

### <a name="6_12_userns"></a>6.12 userns

`userns` displays the name and the user identifier of the user which created the user namespaces of a given process.

For example, we display the user identifier of the creator of the user namespace of the current shell:
```none
$ echo $$
7622
$ sudo ./userns $$
/proc/7622/ns/user belongs to user: 'root' (0)
```
### <a name="6_13_linfo"></a>6.13 linfo

`linfo` displays information about a symbolink link.

For example:
```none
$ ./linfo /proc/$$/ns/mnt
Symbolic link:
	Name: /proc/3098/ns/mnt
	Rights: 0777
	Device (major/minor): 0x0/0x5
	Inode: 0x1a284 (107140)
Target:
	Name: mnt:[4026531840]
	Rights: 0444
	Device (major/minor): 0x0/0x4
	Inode: 0xf0000000 (4026531840)
```


### <a name="6_14_setnshost"></a>6.14 setnshost

`setnshost` sets the hostname in the uts namespace of a given process.

For example, we set the hostname in a LXC container using the pid of its _init_ process:
```none
$ sudo lxc-start -n bbox
$ sudo lxc-console -n bbox -t 0
Connected to tty 0
Type <Ctrl+a q> to exit the console, <Ctrl+a Ctrl+a> to enter Ctrl+a itself

BusyBox v1.30.1 (Ubuntu 1:1.30.1-4ubuntu4) built-in shell (ash)
Enter 'help' for a list of built-in commands.

/ # hostname
bbox
/ # <CTRL>+<a>+<q>
$ sudo ./lxc-pid bbox
5865
$ sudo ./setnshost 5865 qwerty
$ sudo lxc-console -n bbox -t 0

Connected to tty 0
Type <Ctrl+a q> to exit the console, <Ctrl+a Ctrl+a> to enter Ctrl+a itself

/ # hostname
qwerty
/ # <CTRL>+<a>+<q>
$
```

### <a name="6_15_fsinfo"></a>6.15 fsinfo

`fsinfo` displays the information about a file system into which a file/directory resides.

To display the information about the root file system on my machine:

```none
$ ./fsinfo /
Type             : 0xef53 (EXT2/3/4FS)
Block size       : 4096
Total blocks     : 119916557
Total free blocks: 117922759
Total files      : 30531584
Max name length  : 255
Mount flags      : 0x1020 (REMOUNT BIND)
```

The following displays the information about the PROCFS file system into which the `ns` directory resides:

```none
$ ./fsinfo /proc/$$/ns    
Type             : 0x9fa0 (PROCFS)
Block size       : 4096
Total blocks     : 0
Total free blocks: 0
Total files      : 0
Max name length  : 255
Mount flags      : 0x102e (NODEV NOEXEC NOSUID REMOUNT BIND)
```

The following displays the information about the NSFS file system into which reside the targets of the symbolic links in the latter directory:

```none
$ ./fsinfo /proc/$$/ns/mnt
Type             : 0x6e736673 (NSFS)
Block size       : 4096
Total blocks     : 0
Total free blocks: 0
Total files      : 0
Max name length  : 255
Mount flags      : 0x20 (REMOUNT)
```
