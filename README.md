## 客户端安装说明

###  1. 拉取项目

```bash
git clone https://github.com/zbchi/WeeeChatroom.git
```

------

### 2. 安装 JSON 依赖

> 客户端依赖 [`nlohmann/json`](https://github.com/nlohmann/json) 用于解析与构建 JSON 数据。

####  Ubuntu / Debian：

```bash
sudo apt update
sudo apt install nlohmann-json3-dev
```

#### Arch / Manjaro：

```bash
sudo pacman -S nlohmann-json
```

------

###  3. 编译项目

```
mkdir build
cd build
cmake ..
make -j
```

------

###  4. 运行客户端

假设服务端 IP 为 `43.130.227.236`，可以使用以下命令连接服务端：

```
./client/chat_client 43.130.227.236
```
