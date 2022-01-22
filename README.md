# HttpClientCurl
目标：使用libcurl写一个能每秒实现2-5万GET或者POST请求的httpClient库。

1）目前使用Multi_poll模式已经实现了；还是不是特别满意；能实现2万左右的并发，但是CPU占用比较多；

2）试着使用epoll模式写了一下，但是并没有调通，接口设计的莫名其妙，调了3天还是无法稳定运行，真是崩溃，所以放弃了使用epoll模式；
