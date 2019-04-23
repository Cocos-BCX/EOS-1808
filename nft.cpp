#include <algorithm>
#include "nft.hpp"

using namespace eosio;

ACTION nft::addadmin(name admin) {
    require_auth(_self);
    check(is_account(admin), "admin account does not exist");

    auto admin_one = admin_tables.find(admin.value);
    check(admin_one == admin_tables.end(), "admin account already authed");

    admin_tables.emplace(_self, [&](auto& admin_data) {
        admin_data.admin = admin;
    });
}

ACTION nft::deladmin(name admin) {
    require_auth(_self);
    check(is_account(admin), "admin account does not exist");

    auto admin_one = admin_tables.find(admin.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    admin_tables.erase(admin_one);
}

ACTION nft::create(name creator, name owner, std::string explain, std::string worldview) {
    check(is_account(creator), "creator account does not exist");
    check(is_account(owner), "owner account does not exist");
    check(explain.size() <= 256, "explain has more than 256 bytes");
    check(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");
    require_auth(creator);

    auto admin_one = admin_tables.find(creator.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    id_type index_id = index_tables.available_primary_key();
    index_tables.emplace(creator, [&](auto& index_data) {
        index_data.id = index_id;
        index_data.status = 1;
    });

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = time_point_sec(current_time_point());
    nft_tables.emplace(creator, [&](auto& nft_data) {
        nft_data.id = index_id;
        nft_data.creator = creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        nft_data.worldview = worldview;
    });

    auto nft_num = nftnumber_tables.find(owner.value);
    if(nft_num != nftnumber_tables.end()) {
        nftnumber_tables.modify(nft_num, creator, [&](auto& nft_num_data) {
            nft_num_data.number = nft_num->number+1;
        });
    } else {
        nftnumber_tables.emplace(creator, [&](auto& nft_num_data) {
            nft_num_data.owner = owner;
            nft_num_data.number = 1;
        });
    }
}

ACTION nft::createother(name creator, name owner, std::string explain, std::string worldview, id_type chainid, id_type targetid) {
    check(is_account(creator), "creator account does not exist");
    check(is_account(owner), "owner account does not exist");
    check(explain.size() <= 256, "explain has more than 64 bytes");
    check(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");
    require_auth(creator);
    
    auto admin_one = admin_tables.find(creator.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto chain_find = nftchain_tables.find(chainid);
    check(chain_find != nftchain_tables.end(), "chainid is not exist");

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = time_point_sec(current_time_point());
    id_type indexid = index_tables.available_primary_key();
    index_tables.emplace(creator, [&](auto& index_data) {
        index_data.id = indexid;
        index_data.status = 1;
    });

    //id_type newid = nft_tables.available_primary_key();
    nft_tables.emplace(creator, [&](auto& nft_data) {
        nft_data.id = indexid;
        nft_data.creator=creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        nft_data.worldview = worldview;
    });

    assetmap_tables.emplace(creator, [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = game_tables.available_primary_key();
        assetmapping_data.fromid = indexid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });

    auto nftnum = nftnumber_tables.find(owner.value);
    if(nftnum != nftnumber_tables.end()){
        nftnumber_tables.modify(nftnum, creator, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number+1;
        });
     } else {
        nftnumber_tables.emplace(creator, [&](auto& nftnum_data) {
            nftnum_data.owner = owner;
            nftnum_data.number = 1;
        });   
    }

    //print(time_now);
}

ACTION nft::addaccauth(name owner, name auth) {
    require_auth(owner);
    check(is_account(auth), "account auth does not exist");

    auto auth_find = accauth_tables.find(owner.value);
    check(auth_find == accauth_tables.end(), "owner account already authed");

    accauth_tables.emplace(owner, [&](auto& auth_data) {
        auth_data.owner = owner;
        auth_data.auth = auth;
    });
}

ACTION nft::addnftattr(name owner, id_type nftid, std::string key, std::string value) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    // auto admin_one = admin_tables.find(owner.value);
    // check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    check(nft_find != nft_tables.end(), "nft id is not exist");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close, does not add nft attr");

    std::map<string, string> atrrmap;
    atrrmap = nft_find->attr;
    auto iter = atrrmap.find(key);
    check(iter == atrrmap.end(), "key is exist");
    atrrmap.insert(std::pair<string, string>(key, value));  
    nft_tables.modify(nft_find, owner, [&](auto& attr_data) {
        attr_data.attr = atrrmap;
    });
}

ACTION nft::editnftattr(name owner, id_type nftid, std::string key, std::string value) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    // auto admin_one = admin_tables.find(owner.value);
    // check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    check(nft_find != nft_tables.end(), "nft id is not exist");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close, does not edit nft attr");

    std::map<string, string> atrrmap;
    atrrmap = nft_find->attr;
    auto iter = atrrmap.find(key);
    check(iter != atrrmap.end(), "key is not exist");
    atrrmap[key] = value;  
    nft_tables.modify(nft_find, owner, [&](auto& attr_data) {
        attr_data.attr = atrrmap;
    }); 
}

ACTION nft::delnftattr(name owner, id_type nftid, string key) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    // auto admin_one = admin_tables.find(owner.value);
    // check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    check(nft_find != nft_tables.end(), "nft id is not exist");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close, does not delete nft attr");

    std::map<string, string> atrrmap;
    atrrmap = nft_find->attr;
    auto iter = atrrmap.find(key);
    check(iter != atrrmap.end(), "key is not exist");
    atrrmap.erase(key); 
    nft_tables.modify(nft_find, owner, [&](auto& attr_data) {
        attr_data.attr = atrrmap;
    }); 
}

ACTION nft::delaccauth(name owner) {
    require_auth(owner);
    check(is_account(owner), "account auth does not exist");

    auto auth_find = accauth_tables.find(owner.value);
    check(auth_find != accauth_tables.end(), "owner has not auth");

    accauth_tables.erase(auth_find);
}

ACTION nft::addnftauth(name owner, name auth, id_type id) {
    require_auth(owner);
    check(is_account(owner), "owner account auth does not exist");
    check(is_account(auth), "auth account auth does not exist");

    auto nft_find_id = nft_tables.find(id);
    check(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner) {
        auto nft_accauth_find = accauth_tables.find(owner.value);
        check(nft_accauth_find != accauth_tables.end(), "account has not auth");
        check(nft_accauth_find->auth != owner, "account has not auth");
    }

    nft_tables.modify(nft_find_id, owner, [&](auto& nft_data) {
        nft_data.auth = auth;
    });
}

ACTION nft::delnftauth(name owner, id_type id) {
    require_auth(owner);
    check(is_account(owner), "account auth does not exist");

    auto nft_find_id = nft_tables.find(id);
    check(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner) {
        auto nft_accauth_find = accauth_tables.find(owner.value);
        check(nft_accauth_find != accauth_tables.end(), "account has not auth"); 
        check(nft_accauth_find->auth != owner, "account has not auth");
    }

    nft_tables.modify(nft_find_id, owner, [&](auto& nft_data) {
        nft_data.auth = owner;
    });
}

ACTION nft::transfernft(name from, name to, id_type id, std::string memo) {
    require_auth(from);
    check(is_account(from), "from auth does not exist");
    check(is_account(to), "to auth does not exist");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    auto nft_find_id = nft_tables.find(id);
    check(nft_find_id != nft_tables.end(), "nft id is not exist");

    name owner_nft = nft_find_id->owner;
    if(nft_find_id->owner != from) {
        if(nft_find_id->auth != from) {
            auto nft_accauth_find = accauth_tables.find(nft_find_id->owner.value);
            check(nft_accauth_find != accauth_tables.end(), "from has not auth"); 
            check(nft_accauth_find->auth != from, "from has not auth"); 
        }       
    }

    nft_tables.modify(nft_find_id, from, [&](auto& nft_data) {
        nft_data.auth = to;
        nft_data.owner = to;
     });

    auto nftnum = nftnumber_tables.find(owner_nft.value);
    if(nftnum->number != 1) {
        nftnumber_tables.modify(nftnum,from, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    } else {
        nftnumber_tables.erase(nftnum);
    }

    auto nfttonum = nftnumber_tables.find(to.value);
    if(nfttonum != nftnumber_tables.end()) {
        nftnumber_tables.modify(nfttonum, from, [&](auto& nftnum_data) {
            nftnum_data.number = nfttonum->number+1;
        });
    } else {
        nftnumber_tables.emplace(from, [&](auto& nftnum_data) {
            nftnum_data.owner = to;
            nftnum_data.number = 1;
        });   
    }  
}

ACTION nft::burn(name owner, id_type nftid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    auto nft_find = nft_tables.find(nftid);
    check(nft_find != nft_tables.end(), "nft asset is not exist");
    check(nft_find->owner == owner, "owner account insufficient privilege");
    nft_tables.erase(nft_find);

    auto nftnum = nftnumber_tables.find(nft_find->owner.value);
    if(nftnum->number != 1) {
        nftnumber_tables.modify(nftnum,owner, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    } else {
        nftnumber_tables.erase(nftnum);
    }

    auto index_id = index_tables.find(nftid);
    check(index_id != index_tables.end(), "nft index does not exist");
    index_tables.modify(index_id, _self, [&](auto& index_data) {
        index_data.status = 0;
    }); 

    auto compose_find = composeattr_tables.find(nftid);
    if(nft_find == nft_tables.end()) {
        composeattr_tables.erase(compose_find);
    }

    auto compose_firid = compose_tables.get_index<"byfir"_n>();
    auto fir_iter = compose_firid.lower_bound(nftid);
    for(; fir_iter != compose_firid.end() && fir_iter->firid == nftid; ++fir_iter) {
        auto fir_one = compose_tables.find(fir_iter->id);
        compose_tables.erase(fir_one);
    }

    auto compose_secid = compose_tables.get_index<"bysec"_n>();
    auto sec_iter = compose_secid.lower_bound(nftid);
    for(; sec_iter != compose_secid.end() && sec_iter->secid == nftid; ++sec_iter) {
        auto sec_one = compose_tables.find(sec_iter->id);
        compose_tables.erase(sec_one);
    }

    auto assetmap_nft = assetmap_tables.get_index<"byfromid"_n>();
    auto iter = assetmap_nft.lower_bound(nftid);
    for( ; iter != assetmap_nft.end() && iter->fromid == nftid; ++iter) {
        auto asset_one = assetmap_tables.find(iter->mappingid);
        assetmap_tables.erase(asset_one);
    }
}

ACTION nft::addchain(name owner, std::string chain) {
    check(is_account(owner), "issuer account does not exist");
    check(chain.size() <= 64, "chain has more than 64 bytes");
    require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nftchains_data = nftchain_tables.get_index<"bystatus"_n>();
    auto iter = nftchains_data.lower_bound(1);

    bool found = true;
    id_type id = 0;
    for(; iter != nftchains_data.end() && iter->status == 1; ++iter) {
        if(iter->chain == chain) {
            id = iter->chainid;
            found = false;
            break;
        }
    }
    check(found, "chain is exists");

    nftchain_tables.emplace(owner, [&](auto& nftchain_data) {
        nftchain_data.chainid = nftchain_tables.available_primary_key();
        nftchain_data.chain = chain;
        nftchain_data.status = 1;  
    });
}

ACTION nft::setchain(name owner, id_type chainid, id_type status) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
   
    auto nftchain_find = nftchain_tables.find(chainid);
    check(nftchain_find != nftchain_tables.end(), "chainid is not exists");

    bool statusOk = ((status == 0 || status == 1) ? true : false);
    check(statusOk, "status must eq 0 or 1");

    nftchain_tables.modify(nftchain_find, owner, [&](auto& nftchain_data) {
        nftchain_data.status = status;
    });
}

ACTION nft::addcompattr(name owner, id_type id) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = nft_tables.find(id);
    check(nft_find_id != nft_tables.end(), "nft id is not exist");

    auto nft_find = composeattr_tables.find(id);
    check(nft_find == composeattr_tables.end(), "id already support compose");
    
    composeattr_tables.emplace(owner, [&](auto& composeattr_data) {
        composeattr_data.nftid = id;
    });
}

ACTION nft::delcompattr(name owner, id_type id) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = composeattr_tables.find(id);
    check(nft_find_id != composeattr_tables.end(), "id can not support compose");
    composeattr_tables.erase(nft_find_id);
}

ACTION nft::setcompose(name owner, id_type firid, id_type secid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    if(firid == secid) {
        check(false, "can not group self");
    }
    auto nft_find_firid = composeattr_tables.find(firid);
    check(nft_find_firid != composeattr_tables.end(), "firid can not support compose");

    auto nft_find_secid = composeattr_tables.find(secid);
    check(nft_find_secid != composeattr_tables.end(), "secid can not support compose");

    auto compose_data = compose_tables.get_index<"byfir"_n>();
    auto iter = compose_data.lower_bound(firid);
    bool found = true;
    id_type id = 0;
    for(; iter != compose_data.end() && iter->firid == firid; ++iter) {
        if(iter->secid == secid) {
            id = iter->id;
            found = false;
            break;
        }
    }
    check(found, "group is exists");

    compose_tables.emplace(owner, [&](auto& compose_data) {
        compose_data.id = compose_tables.available_primary_key();
        compose_data.firid = firid;
        compose_data.secid = secid;
        compose_data.status = 1;  
    });
}

ACTION nft::delcompose(name owner, id_type firid, id_type secid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    auto nft_find_firid = nft_tables.find(firid);
    check(nft_find_firid != nft_tables.end(), "firid is not exist");

    auto nft_find_secid = nft_tables.find(secid);
    check(nft_find_secid != nft_tables.end(), "secid is not exist");

    auto compose_data = compose_tables.get_index<"byfir"_n>();
    auto it = compose_data.lower_bound(firid);
    bool found = false;
    id_type id = 0;
    for(; it!= compose_data.end() && it->firid==firid; ++it) {
        if(it->secid == secid) {
            id = it->id;
            found = true;
            break;
        }
    }

    check(found, "group is not exists");
    auto group_find_id = compose_tables.find(id);
    compose_tables.erase(group_find_id);
}

ACTION nft::addgame(name owner, std::string gamename, std::string introduces) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    check(gamename.size() <= 32, "gamename has more than 32 bytes");
    check(introduces.size() <= 256, "introduces has more than 256 bytes");
    
    auto game_data = game_tables.get_index<"bystatus"_n>();
    auto iter = game_data.lower_bound(0);
    bool found = true;
    for(; iter != game_data.end() && iter->status == 1; ++iter) {
        if(iter->gamename == gamename) {
            found = false;
            break;
        }
    }
    check(found, "gamename is exists");

    //auto time_now = time_point_sec(now());
    auto time_now = time_point_sec(current_time_point());
    game_tables.emplace(owner, [&](auto& game_data) {
        game_data.gameid = game_tables.available_primary_key();
        game_data.gamename = gamename;
        game_data.introduces = introduces;
        game_data.createtime = time_now;
        game_data.status = 1;
        game_data.index = 0;
    });
}

ACTION nft::editgame(name owner, id_type gameid, std::string gamename, std::string introduces) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    check(introduces.size() <= 256, "introduces has more than 256 bytes");
    check(is_account(owner), "issuer account does not exist");
    
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "game id is not exist");

    game_tables.modify(game_find, owner, [&](auto& game_data) {
        game_data.gamename = gamename;
        game_data.introduces = introduces;
    });
}

ACTION nft::setgame(name owner, id_type gameid, id_type status) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    bool statusOk = ((status == 0 || status == 1) ? true : false);
    check(statusOk, "status must eq 0 or 1");
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "gameid is not exist");
    game_tables.modify(game_find, owner, [&](auto& game_data) {
        game_data.status = status;
    });
}

ACTION nft::delgame(name owner, id_type gameid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "gameid is not exist");
    game_tables.erase(game_find);
}

ACTION nft::addgameattr(name owner, id_type gameid, std::string key, std::string value) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "gameid is not exist");

    std::map<string, string> introducesmap;
    introducesmap = game_find->gameattr;
    auto iter = introducesmap.find(key);
    check(iter == introducesmap.end(), "key is exist");
    introducesmap.insert(std::pair<string, string>(key, value));  
    game_tables.modify(game_find, owner, [&](auto& attr_data) {
        attr_data.gameattr = introducesmap;
    });
}

ACTION nft::editgameattr(name owner, id_type gameid, std::string key, std::string value) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "gameid is not exist");
    std::map<string, string> introducesmap;
    introducesmap = game_find->gameattr;
    auto iter = introducesmap.find(key);
    check(iter != introducesmap.end(), "key is not exist");
    introducesmap[key] = value;  
    game_tables.modify(game_find, owner, [&](auto& attr_data) {
        attr_data.gameattr = introducesmap;
    }); 
}

ACTION nft::delgameattr(name owner, id_type gameid, string key) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    check(game_find != game_tables.end(), "gameid is not exist");
    std::map<string, string> introducesmap;
    introducesmap = game_find->gameattr;
    auto iter = introducesmap.find(key);
    check(iter != introducesmap.end(), "key is not exist");
    introducesmap.erase(key); 
    game_tables.modify(game_find, owner, [&](auto& attr_data) {
        attr_data.gameattr = introducesmap;
    }); 
}

ACTION nft::addmapping(name owner, id_type fromid, id_type targetid, id_type chainid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(fromid);
    check(nft_find != nft_tables.end(), "fromid is not exist,nft asset is not exist");

    auto chain_find = nftchain_tables.find(chainid);
    check(chain_find != nftchain_tables.end(), "chainid is not exist");

    auto assetmapping_tar_data = assetmap_tables.get_index<"bytargetid"_n>();
    auto iter = assetmapping_tar_data.lower_bound(targetid);
    bool targetfound = true;
    for(; iter != assetmapping_tar_data.end() && iter->targetid == targetid; ++iter) {
        if(iter->chainid == chainid) {
            print(iter->chainid);
            print(iter->targetid);
            print(iter->fromid);
            targetfound = false;
            break;
        }
    }

    // check(founds, "nftmapping_target is exists");
    auto assetmapping_data = assetmap_tables.get_index<"byfromid"_n>();
    auto it = assetmapping_data.find(fromid);

    bool fromfound = true;
    for(; it != assetmapping_data.end() && it->fromid == fromid; ++it) {
        if(it->chainid == chainid) {
            fromfound = false;
            break;
        }
    }
    check(fromfound, "nft mapping from is exists");

    assetmap_tables.emplace(owner, [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = assetmap_tables.available_primary_key();
        assetmapping_data.fromid = fromid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });
}

ACTION nft::delmapping(name owner, id_type fromid, id_type chainid) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    check(admin_one != admin_tables.end(), "admin account is not auth");

    auto nft_find = nft_tables.find(fromid);
    check(nft_find != nft_tables.end(), "fromid is not exist,nft asset is not exist");

    auto chain_find = nftchain_tables.find(chainid);
    check(chain_find != nftchain_tables.end(), "chainid is not exist");

    auto assetmapping_data = assetmap_tables.get_index<"byfromid"_n>();
    auto iter = assetmapping_data.lower_bound(fromid);
    bool found = false;
    id_type assetmap_id = 0;
    for( ; iter != assetmapping_data.end() && iter->fromid == fromid; ++iter) {
        if(iter->chainid == chainid) {
            found = true;
            assetmap_id = iter->mappingid;
            break;
        }
    }
    check(found, "nftmapping is not exists");

    auto nftmap_find = assetmap_tables.find(assetmap_id);
    assetmap_tables.erase(nftmap_find);
}

ACTION nft::orderclean(id_type orderid) {
    auto iter = order_tables.find(orderid);
    check(iter != order_tables.end(), std::to_string(orderid) + ", orderclean failed, order is not exist");
    order_tables.erase(iter);
}

void nft::createorder(name owner, id_type nftid, asset amount, std::string side, std::string memo) {
    check(is_account(owner), "issuer account does not exist");
    require_auth(owner);

    //param check
    check(side == "buy" || side == "sell", "side must eq buy or sell");
    check(memo.size() <= 256, "memo has more than 256 bytes");
    check(amount.is_valid(), "invalid symbol name");
    check(amount.amount > 0, "amount must be positive");
    check(amount.symbol.code().to_string() == "EOS", "currency must be EOS");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close, can't place order");

    //nft check
    auto asset_iter = nft_tables.find(nftid);
    check(asset_iter != nft_tables.end(), "asset does not exist");

    if (side == "sell") {
        check(asset_iter->owner == owner, "can't sell other nft asset");
        auto order_data = order_tables.get_index<"byowner"_n>();
        auto iter = order_data.lower_bound(owner.value);
        bool isValid = true;
        for( ; iter != order_data.end() && iter->nftid == nftid; ++iter) {
            if(iter->side == "sell") {
                isValid = false;  //one nft one sell
                break;
            }
        }
        check(isValid, "nft sell order is not valid");
    } else {
        check(asset_iter->owner != owner, "Can't buy your own nft asset");
    }

    order_tables.emplace(_self, [&](auto& order) {
        order.id = order_tables.available_primary_key();
        order.nftid = nftid;
        order.owner = owner;
        order.price = amount;
        order.side = side;
        order.memo = memo;
        order.createtime = time_point_sec(current_time_point());
    });
}

void nft::cancelorder(name owner, id_type id, std::string memo) {
    check(is_account(owner), "issuer account does not exist");

    auto iter = order_tables.find(id);
    check(iter != order_tables.end(), std::to_string(id) + ", cancel failed, order is not exist, " + memo);
    check(iter->owner == owner, "owner is not equal");

    auto status_iter = index_tables.find(iter->nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close, can't place order");

    order_tables.erase(iter);

    //transfer token
    if(iter->side == "buy") {
        action(permission_level{ _self, "active"_n },
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, owner, iter->price, std::string("cancel order")))
        .send();
    }
}

void nft::trade(const name& from, const name& to, id_type orderid, const std::string& side, const std::string& memo) {
    check(is_account(from), "from account does not exist");
    check(is_account(to), "to account does not exist");
    check(side == "buy" || side == "sell", "side must be buy or sell");

    auto iter = order_tables.find(orderid);
    check(iter != order_tables.end(), std::to_string(orderid) + ", trade failed, order is not exist");
    order_tables.erase(iter);

    auto status_iter = index_tables.find(iter->nftid);
    check(status_iter != index_tables.end(), "nft index does not exist");
    check(status_iter->status == 1, "nft status is close");

    auto nft_iter = nft_tables.find(iter->nftid);
    check(nft_iter != nft_tables.end(), "nft asset is not exist");
    if (iter->side == "buy" && side == "sell") {
        check(nft_iter->owner == to, "buy order, owner is not equal to account");
    } else if (iter->side == "sell" && side == "buy") {
        check(nft_iter->owner == iter->owner, "sell order, owner is not equal from account");
    } else {
        check(false, "invalid side");
    }
    nft_tables.modify(nft_iter, _self, [&](auto& nft_data) {
        nft_data.auth = to;
        nft_data.owner = to;
     });

    //from nft number -1
    auto nftnum = nftnumber_tables.find(from.value);
    if(nftnum->number != 1) {
        nftnumber_tables.modify(nftnum, _self, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number - 1;
        });
    } else {
        nftnumber_tables.erase(nftnum);
    }

    //to nft number +1
    auto nfttonum = nftnumber_tables.find(to.value);
    if(nfttonum != nftnumber_tables.end()) {
        nftnumber_tables.modify(nfttonum, _self, [&](auto& nftnum_data) {
            nftnum_data.number = nfttonum->number + 1;
        });
    } else {
        nftnumber_tables.emplace(to, [&](auto& nftnum_data) {
            nftnum_data.owner = to;
            nftnum_data.number = 1;
        });   
    }

    auto amount = iter->price;
    amount.amount = amount.amount - FEE;
    if (amount.amount > 0) {
        action(permission_level{ _self, "active"_n },
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, from, amount, memo))
        .send();
    }
}

void nft::parse_memo(std::string memo, std::string& action, std::map<std::string, std::string>& params) {
    // remove space
    memo.erase(std::remove_if(memo.begin(), memo.end(), [](unsigned char x) { 
            return std::isspace(x); }), memo.end());

    size_t pos;
    std::string container;
    pos = sub2sep(memo, container, '-', 0, true);
    check(!container.empty(), "no action");
    action = container;
    if (container == "order") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no owner");
        params["owner"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no nft id");
        params["nftid"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no price");
        params["price"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no side");
        check(container == "buy" || container == "sell", "side must be buy or sell");
        params["side"] = container;
    } else if (container == "trade") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no from account");
        params["from"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no to account");
        params["to"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no order id");
        params["id"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no side");
        check(container == "buy" || container == "sell", "side must be buy or sell");
        params["side"] = container;
    } else if (container == "cancel") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no from account");
        params["owner"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        check(!container.empty(), "no order id");
        params["id"] = container;
    }
}

void nft::transfer(const name& from, const name& to, const asset& quantity, const std::string& memo) {
    check(is_account(from), "from account does not exist");
    check(is_account(to), "to account does not exist");
    check(quantity.symbol.is_valid(), "invalid symbol name");
    //check(quantity.symbol.code() == "EOS", "transfer must be EOS");
    check(quantity.amount >= FEE, "transfer fee too small");
    check(memo.size() <= 256 && !memo.empty(), "memo has more than 256 bytes or is empty");
    if (from == _self || to != _self) {
        return;
    }

    std::map<std::string, std::string> params;
    std::string action;
    parse_memo(memo, action, params);
    check(action == "order" || action == "trade" || action == "cancel", "memo action error");
    check(params.size() > 0, "memo error");

    if(action == "order") {
        check(params.find("owner") != params.end() && params.find("nftid") != params.end()
            && params.find("price") != params.end() && params.find("side") != params.end(), 
            "create order param error");
        std::string owner = params["owner"];
        std::string id = params["nftid"];
        uint64_t nftid = stoi(id);
        std::string price = params["price"];
        uint64_t nPrice = stoi(price);
        std::string side = params["side"];
        auto amount = quantity;
        if (side == "buy") {
            check(amount.amount == nPrice, "buy nft, transfer quantity not equal price");
        } else {
            amount.amount = nPrice;
        }
        createorder(name(owner), nftid, amount, side, memo);
    } else if(action == "trade") {
        check(params.find("from") != params.end() && params.find("to") != params.end()
            && params.find("id") != params.end() && params.find("side") != params.end(),
            "trade param error");
        std::string id = params["id"];
        uint64_t order_id = stoi(id);
        if(params["side"] == "buy") {
            check(from.to_string() == params["to"], "transfer from not equal memo to account");
        } else {
            check(from .to_string() == params["from"], "transfer from not equal memo from account");
        }
        trade(name(params["from"]), name(params["to"]), order_id, params["side"], memo);
    } else if(action == "cancel") {
        check(params.find("owner") != params.end() && params.find("id") != params.end(), 
            "cancel order param error");
        check(quantity.amount >= FEE, "cancel order, fee not enough");
        check(from.to_string() == params["owner"], "transfer from not equal memo owner");
        std::string id = params["id"];
        uint64_t order_id = stoi(id);
        cancelorder(name(params["owner"]), order_id, memo);
    }
}

#ifdef EOSIO_DISPATCH
#undef EOSIO_DISPATCH
#endif
#define EOSIO_DISPATCH( TYPE, MEMBERS )                                         \
extern "C" {                                                                    \
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {             \
        if ( code == receiver ) {                                               \
            switch( action ) {                                                  \
                EOSIO_DISPATCH_HELPER( TYPE, MEMBERS )                          \
            }                                                                   \
        }                                                                       \
        if (code == N(eosio.token).value && action == N(transfer).value) {      \
            execute_action(name(receiver), name(receiver), &nft::transfer);     \
            return;                                                             \
        }                                                                       \
        if (action == N(transfer).value) {                                      \
            check(false, "only support EOS token");                             \
        }                                                                       \
    }                                                                           \
}

EOSIO_DISPATCH(nft, (addadmin)(deladmin)(create)(createother)(addnftattr)(editnftattr)(delnftattr)(addaccauth)
    (delaccauth)(addnftauth)(delnftauth)(transfer)(addchain)(setchain)(addcompattr)(delcompattr)(setcompose)
    (delcompose)(addgame)(setgame)(editgame)(delgame)(addgameattr)(editgameattr)(delgameattr)(addmapping)
    (delmapping)(burn)(orderclean))