#include <algorithm>
#include "nft.hpp"

using namespace eosio;

ACTION nft::addadmin(name account, uint32_t flag) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintable.find(account.value);
    check(iter == admintable.end(),  " has been added");
    admintable.emplace(_self, [&](auto& d) {
        d.account = account;
        d.flag = flag;
    });
}

ACTION nft::updateperm(name account, uint32_t flag) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintable.find(account.value);
    check(iter != admintable.end(), account.to_string() + " account does not exist in table");
    admintable.modify(iter, _self, [&](auto& d) {
        d.flag = flag;
    });
}

ACTION nft::deladmin(name account) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintable.find(account.value);
    check(iter != admintable.end(), account.to_string() + " account does not exist in table");
    admintable.erase(iter);
}

ACTION nft::addworldview(name creator, const std::string& worldview) {
    check(is_account(creator), creator.to_string() + " account does not exist");
    require_auth(creator);

    // permission check
    auto admin_iter = admintable.find(creator.value);
    check(admin_iter != admintable.end() && admin_iter->flag & nft_permission_flags::worldview_operation, 
        creator.to_string() + " should have worldview operation permission");

    // worldview check
    uint64_t key = string_to_uint64(worldview.c_str());
    auto worldviewkeys = worldviewstable.get_index<"bykey"_n>();
    auto lower_iter = worldviewkeys.lower_bound(key-1);
    auto upper_iter = worldviewkeys.upper_bound(key);
    for( ; lower_iter != upper_iter; ++lower_iter) {
        check(lower_iter->worldview != worldview, worldview + " worldview already exists");
    }

    worldviewstable.emplace(_self, [&](auto& d) {
        d.id = worldviewstable.available_primary_key();
        d.worldviewkey = key;
        d.creator = creator;
        d.worldview = worldview;
    });
}

ACTION nft::delworldview(uint64_t id, name owner) {
    require_auth(owner);
    check(is_account(owner), owner.to_string() + " account does not exist");

    auto iter = worldviewstable.find(id);
    check(iter != worldviewstable.end(), "worldview not exist");
    check(iter->creator == owner, "don't delete other worldview");
    worldviewstable.erase(iter);
}

//symbols
ACTION nft::addsymbol(const symbol& sym, name creator, const std::string& nick, const std::string& desc) {
    require_auth(_self);
    check(sym.is_valid(), "invalid symbol name");
    check(is_account(creator), "creator account does not exist");
    check(nick.size() <= 40, "nick name has more than 40 bytes");
    check(desc.size() <= 256, "symbol describe has more than 256 bytes");

    auto existing = symbolstable.find( sym.code().raw() );
    check( existing == symbolstable.end(), "token with symbol already exists" );

    symbolstable.emplace(_self, [&](auto& d) {
        d.sym = sym;
        d.creator = creator;
        d.nick = nick;
        d.desc = desc;
    });
}

ACTION nft::delsymbol(const symbol& sym) {
    require_auth(_self);
    check(sym.is_valid(), "invalid symbol name");
    auto existing = symbolstable.find( sym.code().raw() );
    check( existing != symbolstable.end(), "token with symbol does not exist" );
    symbolstable.erase(existing);
}

//nh asset 
ACTION nft::createnh(name creator, name owner, const symbol& sym, const std::string& worldview, const std::string& basedesc) {
    check(is_account(creator), "creator account does not exist");
    check(is_account(owner), "owner account does not exist");
    check(sym.is_valid(), "invalid symbol name");
    check(basedesc.size() <= 256, "base describe has more than 256 bytes");
    check(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");
    require_auth(creator);

    // permission check
    auto admin_iter = admintable.find(creator.value);
    check(admin_iter != admintable.end() && admin_iter->flag & nft_permission_flags::nft_operation, 
        creator.to_string() + " should have worldview operation permission");

    // Create new nft
    auto time_now = time_point_sec(current_time_point());
    nhassetstable.emplace(_self, [&](auto& d) {
        d.id = nhassetstable.available_primary_key();
        d.creator = creator;
        d.owner = owner;
        d.auth = owner;
        d.sym = sym;
        d.explain = basedesc;
        d.createtime = time_now;
        d.worldview = worldview;
        d.conrel = map<std::string, id_type>();
    });
}

// ACTION nft::delnh(uint64_t id) {
//     auto iter = nhassetstable.find(id);
//     check(iter != nhassetstable.end(), "not exist in nhassetstable table ");
//     nhassetstable.erase(iter);
// }

// ACTION nft::delrel(uint64_t id) {
//     auto iter = nhrelatestable.find(id);
//     check(iter != nhrelatestable.end(), "not exist in nhrelatestable table ");
//     nhrelatestable.erase(iter); 
// }

ACTION nft::relatenh(name owner, uint64_t pid, uint64_t cid, const std::string& contractid, bool relate) {
    require_auth(owner);
    check(is_account(owner), "owner account does not exist");

    //check parent and nft asset id 
    auto piter = nhassetstable.find(pid);
    check(piter != nhassetstable.end(), " parent nft asset does not exist. asset id = " + std::to_string(pid));
    check(piter->owner == owner, "You're not the parent nh asset's owner, so you can't relate it.");

    auto citer = nhassetstable.find(cid);
    check(citer != nhassetstable.end(), "child nft asset does not exist. asset id = " + std::to_string(cid));
    check(citer->owner == owner, "You're not the child nh asset's owner, so you can't relate it");
    check(piter->worldview == citer->worldview, "different worldviews");
    const auto &iter = piter->conrel.find(contractid);
    if(relate) {
        check(citer->conrel.size() == 0, "child asset had be related");
        if(iter != piter->conrel.end()) {
            auto relateID = iter->second;
            auto relateIter = nhrelatestable.find(relateID);
            check(relateIter != nhrelatestable.end(), "relate id does not find");
            nhrelatestable.modify(relateIter, _self, [&](auto& d) { d.child.emplace_back(cid); });
            nhassetstable.modify(citer, _self, [&](auto &d) { d.conrel[contractid] = relateID; });
        } else {
            auto relateID = nhrelatestable.available_primary_key();
            nhassetstable.modify(piter, _self, [&](auto &d) { d.conrel[contractid] = relateID; });
            nhassetstable.modify(citer, _self, [&](auto &d) { d.conrel[contractid] = relateID; });
            vector<id_type> pv; pv.push_back(pid);
            vector<id_type> cv; cv.push_back(cid);
            nhrelatestable.emplace(_self, [&](auto& d) {
                d.id = relateID;
                d.contractkey = string_to_uint64(contractid.c_str());
                d.contract = contractid;
                d.parent = pv;
                d.child = cv;
            });
        }
    } else {
        check( iter != piter->conrel.end(), "The parent nh asset's parent dosen't contain this contract.");
        auto relateID = iter->second;
        auto relateIter = nhrelatestable.find(relateID);
        check(relateIter != nhrelatestable.end(), "relate id does not find"); 
        auto rpIter = find(relateIter->parent.begin(), relateIter->parent.end(), pid);
        check( rpIter != relateIter->parent.end(), "The parent nh asset and child nh asset did not relate.");
        auto rcIter = find(relateIter->child.begin(), relateIter->child.end(), cid);
        check( rcIter != relateIter->child.end(), "The parent nh asset and child nh asset did not relate.");
        if(relateIter->parent.size() == 1 && relateIter->child.size() == 1) {
            nhrelatestable.erase(relateIter);
            nhassetstable.modify(piter, _self, [&](auto &d) { d.conrel.erase(contractid); });
            nhassetstable.modify(citer, _self, [&](auto &d) { d.conrel.erase(contractid); });
        } else {
            if(relateIter->parent.size() < relateIter->child.size()) {
                nhrelatestable.modify(relateIter, _self, [&](auto& d) { d.child.erase(rcIter); });
            } else {
                nhrelatestable.modify(relateIter, _self, [&](auto& d) { d.parent.erase(rpIter); d.child.erase(rcIter); });
            }
        }
    }
}

ACTION nft::burn(uint64_t id, name owner) {
    require_auth(owner);
    check(is_account(owner), "owner account does not exist");

    auto iter = nhassetstable.find(id);
    check(iter != nhassetstable.end(), " parent nft asset does not exist. asset id = " + std::to_string(id));
    check(iter->owner == owner, "You're not the parent nh asset's owner, so you can't relate it.");
    check(iter->conrel.empty(), "Please disconnect first ");
    nhassetstable.erase(iter);

    // if(!iter->conrel.empty()) {
    //     for(auto it = iter->conrel.begin(); it != iter->conrel.end(); it++) {
    //         auto relateID = it->second;
    //         auto rIter = nhrelatestable.find(it->second);
    //         if(rIter != nhrelatestable.end()) {
    //             //解除关联
    //             auto contract = rIter->contract;
    //             for(auto pIter = rIter->parent.begin(); pIter != rIter->parent.end(); pIter++) {
    //                 auto nhIter = nhassetstable.find(*pIter);
    //                 check(nhIter != nhassetstable.end(), " parent nft asset does not exist. asset id = " + std::to_string(id));
    //                 check(nhIter->owner == owner, "You're not the parent nh asset's owner, so you can't relate it.");
    //                 if(nhIter->conrel.find(contract) != nhIter->conrel.end()) {
    //                     nhassetstable.modify(nhIter, _self, [&](auto &d) { d.conrel.erase(contract); });
    //                 }
    //             }
    //             for(auto pIter = rIter->child.begin(); pIter != rIter->child.end(); pIter++) {
    //                 auto nhIter = nhassetstable.find(*pIter);
    //                 check(nhIter != nhassetstable.end(), " parent nft asset does not exist. asset id = " + std::to_string(id));
    //                 check(nhIter->owner == owner, "You're not the parent nh asset's owner, so you can't relate it.");
    //                 if(nhIter->conrel.find(contract) != nhIter->conrel.end()) {
    //                     nhassetstable.modify(nhIter, _self, [&](auto &d) { d.conrel.erase(contract); });
    //                 }
    //             }
    //             nhrelatestable.erase(rIter);
    //         }
    //     }
    // }
}

void nft::transfernh(const name& from, const name& to, uint64_t asset) {
    require_auth(from);
    check(is_account(from), "from account does not exist");
    check(is_account(to), "to account does not exist");
    check(from != to, "can't transfer to oneself");
    auto iter = nhassetstable.find(asset);
    check(iter != nhassetstable.end(), "NH asset does not exist. asset id = " + std::to_string(asset));
    check(iter->owner == from, "from account is not equal to asset.owner");
    nhassetstable.modify(iter, _self, [&](auto& d) {
        d.owner = to;
     });
}

void nft::transfer(const name& from, const name& to, const asset& quantity, const std::string& memo) {
    print("test transfer");
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

EOSIO_DISPATCH(nft, (addadmin)(updateperm)(deladmin)(addworldview)(delworldview)(addsymbol)(delsymbol)
    (createnh)(relatenh)(burn)
    //(delnh)(delrel)
 )

