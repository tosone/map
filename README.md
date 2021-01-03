# map

一个类似于 Redis 客户端的命令格式，包含各种简单工具的集合。

## 命令列表

### Help

- `help` 获取当前的帮助信息。

### Exit

- `exit` 结束程序。

### Version

- `version` 获取当前版本。

### Base64

- `base64 enc <string>` 编码 string 为 base64 字符串。
- `base64 dec <string>` 解码 base64 字符串。

### Hash

- `hash <method> <filename/string>` 对字符串或者文件进行 Hash，支持的 Hash 算法包括：md5、sha1、sha256、sha12。

### Editor

- `vi <filename>` 编辑一个文件。

### Network

- `tcp <ip> <port>` 测试 host 的端口是否通。

### System

- `uname` 获取系统架构信息。
- `uptime` 获取系统开机多久。
- `hostname` 获取 hostname。

### Game

- `game <name>` 开始某个游戏。

### Pi

- `pi <length>` 计算 pi 值。
