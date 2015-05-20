LocalSearch
===========

本项目用于GIF库中，索引本地数据库并提供检索服务,性能稳定。

包括数据抓取模块、分词模块、倒排索引、索引存储(创新)、相关度排序模块、HttpServer、Redis值存储 等模块。


#Build

依赖库：

scws-1.2.2 分词组件

hiredis   提供Key值存储
