LazyCat 用户手册

运行前请确认系统中安装有 Python 2.7
shell> python -V

修改客户机头文件内的服务器 IP 地址。
位于 Chinchilla.h 第 53 行。

编译
进入 Source 文件夹后执行
shell> make

若无错误，将看到生成的可执行文件 server 和 client。
分别为服务器和客户机

分别运行 server 和 client 即可
shell> ./server
shell> ./client

查看命令帮助可通过 /help 或 /? 来完成。
LazyCat 所有的命令均以 `/' 开始。操作时请按照屏幕提示进行。

若有问题，请参阅设计手册。

Enjoy!