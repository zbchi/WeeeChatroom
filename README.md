# 客户端安装
## docker安装
<!-- 
> docker run 最后的参数是服务端的Ip地址。
如用容器启动并传文件:  
1.把文件托进/tmp/chatclient中。
2.使用/tmp/chatclient为父目录传输文件，如:/tmp/chatclient/文件名

```bash
docker pull zbchi/chatroomcli
docker run -it -v /tmp/chatclient:/tmp/chatclient zbchi/chatroomcli 10.30.1.235
```

------ -->

## 编译安装

###  1. 拉取仓库

```bash
git clone https://github.com/zbchi/WeeeChatroom.git
```

### 2. 安装依赖

> 客户端依赖 [`nlohmann/json`](https://github.com/nlohmann/json) 用于解析与构建 JSON 数据。

####  Ubuntu / Debian：

```bash
sudo apt update
sudo apt install nlohmann-json3-dev

sudo apt install libreadline-dev

```

#### Arch / Manjaro：

```bash
sudo pacman -S nlohmann-json

sudo pacman -S readline
```

###  3. 编译项目

```bash
cd WeeeChatroom
cmake -Bbuild -S. -DO=ON
cd build
make -j
```

###  4. 运行客户端

假设服务端 IP 为 `10.30.1.235`，可以使用以下命令连接服务端：

```bash
./client/chat_client 10.30.1.235
```

> 下载的文件存储在目录:/tmp/chatclient/chat_files 中


# 服务端安装

## docker安装

```bash
git clone https://github.com/zbchi/WeeeChatroom.git
cd Weeechatroom
docker compose up
```

------

## 编译安装

### 1.安装依赖

```bash
#安装Json
sudo apt update
sudo apt install nlohmann-json3-dev

#安装并启动Redisls
sudo apt update
sudo apt install redis -y
sudo systemctl start redis

#安装MySQL
sudo apt install -y mysql-server mysql-client

#安装hiredis库
git clone https://github.com/redis/hiredis.git
cd hiredis
make -j
sudo make install

#安装redis++库
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
sudo make install

#安装MySQL开发库
sudo apt install libmysqlclient-dev

#安装libcurl和libcurl-dev
sudo apt install -y libcurl4-openssl-dev

#安装libuuid
sudo apt-get install uuid-dev

```

### 2.拉取仓库

``` bash
git clone https://github.com/zbchi/WeeeChatroom.git
```

### 3.初始化数据库

```bash
cd WeeeChatroom
sudo mysql < init.sql
```

### 4.编译并运行

``` bash
mkdir build 
cd build 
cmake ..
make -j
./server/chat_server
```
