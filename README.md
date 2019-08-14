[中文](https://github.com/Cocos-BCX/EOS-1808/blob/master/README_cn.md)

# Cocos-BCX 1808 概述
[COCOS 1808 非同质数字资产标准和世界观系统](https://github.com/Cocos-BCX/1808/blob/master/README.md)

# EOS-1808
EOS-1808项目是基于Cocos-BCX 1808设计理念在EOS主网上的一个Demo实现。示例实际上是一个合约，开发者可以基于该合约开发属于自己的项目。

开发者可以调用的公开的合约接口如下所示：

* 权限相关
  * addadmin(name admin)
  * deladmin(name admin)

* 非同质资产
  * create(name creator, name owner, std::string explain, std::string worldview)
  * createother(name creator, name owner, std::string explain, std::string worldview, id_type chainid, id_type targetid)
  * addnftattr(name owner, id_type nftid, std::string key, std::string value)
  * editnftattr(name owner, id_type nftid, std::string key, std::string value)
  * delnftattr(name owner, id_type nftid, std::string key)
  * addaccauth(name owner, name auth)
  * delaccauth(name owner)
  * addnftauth(name owner, name auth, id_type id)
  * delnftauth(name owner, id_type id)
  * transfernft(name from, name to, id_type id, std::string memo)
  * burn(name owner, id_type nftid)

* 跨链
  * addchain(name owner, std::string chain)
  * setchain(name owner, id_type chainid, id_type status)

* 组合资产
  * addcompattr(name owner, id_type id)
  * delcompattr(name owner, id_type id)
  * setcompose(name owner, id_type firid, id_type secid)
  * delcompose(name owner, id_type firid, id_type secid)

* 跨链资产映射
  * addmapping(name owner, id_type fromid, id_type targetid, id_type chainid)
  * delmapping(name owner, id_type fromid, id_type chainid)

* 游戏示例
  * addgame(name owner, std::string gamename, std::string introduces)
  * editgame(name owner, id_type gameid, std::string gamename, std::string introduces)
  * setgame(name owner, id_type gameid, id_type status)
  * delgame(name owner, id_type gameid)
  * addgameattr(name owner, id_type gameid, std::string key, std::string value)
  * editgameattr(name owner, id_type gameid, std::string key, std::string value)
  * delgameattr(name owner, id_type gameid, std::string key)
