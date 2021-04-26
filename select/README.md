# 编译运行：
```g++ server.cpp -o server && ./server```
```g++ client.cpp -o client && ./client```

# 项目简介：

+ 客户端是很简单的 socket->bind->connect->write->read->write（显示屏）；
+ 服务器端是用很简单的 select 的网络I/O实现的
    - socket->bind->listen->fd_set->FD_ZERO->FD_SET->select->遍历文件描述符集->FD_ISSET->accept->FD_SET->read/write/recv/send

# 理解关键点：
+ bind 的文件描述符只是用于监听使用，如果监听到客户端后使用 accept 成功后返回的一个新文件描述符，这个新文件描述符才是用来和客户端读写的；也就是说监听和传输数据的文件描述符是分开的；
+ 新增了一个地址结构体数组用来保存正在通信的客户端，用来输出记录目前通信的是哪个客户端，但是存在两个问题：
    - 用 vector 构造数组使用时失败了；
    - 在客户端结束断开联系时如何把已经保存的地址给置为初始化状态；