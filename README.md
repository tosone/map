# map

一个类似于 Redis 客户端的命令格式，包含各种简单的数据结构的实现。

## 命令列表

HashMap 数据结构。包含的命令有：

- `hmap set <key> <value>` 设置一个 key 到 hashmap 中。
- `hmap get <key>` 获取一个 key 的值。
- `hmap del <key>` 删除一个 key 的值。
- `hmap cap` 获取 hashmap 的 cap。
- `hmap len` 获取 hashmap 的 len。
- `hmap print` 打印所有 hashmap 的数据。

LRU 数据结构。包含的命令有：

- `lru set <key> <value>` 设置一个 key 到 LRU 中。
- `lru get <key>` 获取一个 key 的值。
- `lru cap set <len>` 设置 LRU 的 cap。
- `lru cap get` 获取 LRU 的 cap。
- `lru len` 获取当前 LRU 中元素的个数。
- `lru print` 打印所有 lru 的数据。

AVL-Tree 数据结构。包含的命令：

- `avl set <key>` 将 key 插入到 avl tree 中。
- `avl get <key>` 判断 key 是否存在于 avl tree 中。
- `avl print <pre/in/post>` 以前序、中序、后序方式遍历 avl tree。
- `avl dump <filename>` 将 avl tree 中的数据导出至 dot。

Base64 包含的命令：

- `base64 enc <string>` 编码 string 为 base64 字符串。
- `base64 dec <string>` 解码 base64 字符串。

## 示例

|HashMap|LRU|AVL-Tree|
|:---:|:---:|:---:|
|![hashmap](./image/hashmap.png)|![lru](./image/lru.png)|![avl](./image/avl.png)|

## 参考

- `dot -Tpng filename.dot -o filename.png` 转换 dot 文件到 png。
