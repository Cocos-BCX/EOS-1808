[English](https://github.com/Cocos-BCX/EOS-1808/blob/master/README.md)

# Cocos 1808 Overview
[1808 Standard & Multiverse System](https://github.com/Cocos-BCX/1808/blob/master/README.md)

# EOS-1808
The EOS-1808 is a Demo implementation on the EOS mainnet based on the design concept of COCOS 1808. The Demo is actually a contract based on which developers can develop their own projects.

The public contract interfaces that developers can call are as follows:

* Authority related
  * addadmin(name admin)
  * deladmin(name admin)

* Non-homogenous assets
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

* Cross-chain
  * addchain(name owner, std::string chain)
  * setchain(name owner, id_type chainid, id_type status)

* Nested assets
  * addcompattr(name owner, id_type id)
  * delcompattr(name owner, id_type id)
  * setcompose(name owner, id_type firid, id_type secid)
  * delcompose(name owner, id_type firid, id_type secid)

* Cross-chain asset mapping
  * addmapping(name owner, id_type fromid, id_type targetid, id_type chainid)
  * delmapping(name owner, id_type fromid, id_type chainid)

* Demo games
  * addgame(name owner, std::string gamename, std::string introduces)
  * editgame(name owner, id_type gameid, std::string gamename, std::string introduces)
  * setgame(name owner, id_type gameid, id_type status)
  * delgame(name owner, id_type gameid)
  * addgameattr(name owner, id_type gameid, std::string key, std::string value)
  * editgameattr(name owner, id_type gameid, std::string key, std::string value)
  * delgameattr(name owner, id_type gameid, std::string key)
