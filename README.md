# HttpClientCurl
目标：使用libcurl写一个能每秒实现2-5万GET或者POST请求的httpClient库。

libcurl有三种使用模式：

1）简单同步模式，内部是每个curl* 内置了一个单一使用的multi*实例，效率比较低；

2）multi_poll或者multi_wait，内部是同一个东西，windows内部使用wsaevent实现，linux使用poll实现；

效率还凑合，但是也没有特别强大；

3）使用外部的事件驱动引擎，驱动内部socket进行处理，号称效率高；

目前版本的完成度：

1）目前使用Multi_poll模式已经实现了；还是不是特别满意；能实现2万左右的并发，但是CPU占用比较多；

2）试着使用epoll模式写了一下，但是并没有调通，接口设计的莫名其妙，调了3天还是无法稳定运行，真是崩溃，所以放弃了使用epoll模式；
