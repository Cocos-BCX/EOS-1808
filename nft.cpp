#include <algorithm>
#include "nft.hpp"

using namespace eosio;

ACTION nft::addadmin(name account, uint32_t flag) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintables.find(account.value);
    check(iter == admintables.end(),  " has been added");
    admintables.emplace(_self, [&](auto& data) {
        data.account = account;
        data.flag = flag;
    });
}

ACTION nft::updateperm(name account, uint32_t flag) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintables.find(account.value);
    check(iter != admintables.end(), account.to_string() + " account does not exist in table");
    admintables.modify(iter, _self, [&](auto& data) {
        data.flag = flag;
    });
}

ACTION nft::deladmin(name account) {
    require_auth(_self);
    check(is_account(account), account.to_string() + " account does not exist");
    auto iter = admintables.find(account.value);
    check(iter != admintables.end(), account.to_string() + " account does not exist in table");
    admintables.erase(iter);
}

ACTION nft::addworldview(name creator, const std::string& worldview) {
    check(is_account(creator), creator.to_string() + " account does not exist");
    require_auth(creator);

    // permission check
    auto admin_iter = admintables.find(creator.value);
    check(admin_iter != admintables.end() && admin_iter->flag & nft_permission_flags::worldview_operation, 
        creator.to_string() + " should have worldview operation permission");

    // worldview check
    uint64_t key = string_to_uint64(worldview.c_str());
    auto worldviewkeys = worldviewstables.get_index<"bywvkey"_n>();
    auto lower_iter = worldviewkeys.lower_bound(key-1);
    auto upper_iter = worldviewkeys.upper_bound(key);
    for( ; lower_iter != upper_iter; ++lower_iter) {
        check(lower_iter->worldview != worldview, worldview + " worldview already exists");
    }

    worldviewstables.emplace(_self, [&](auto& data) {
        data.id = worldviewstables.available_primary_key();
        data.worldviewkey = key;
        data.creator = creator;
        data.worldview = worldview;
    });
}

ACTION nft::delworldview(uint64_t id, name owner) {
    require_auth(owner);
    check(is_account(owner), owner.to_string() + " account does not exist");

    auto iter = worldviewstables.find(id);
    check(iter != worldviewstables.end(), "worldview not exist");
    check(iter->creator == owner, "don't delete other worldview");
    worldviewstables.erase(iter);
}

//symbols
ACTION nft::addsymbol(const symbol& sym, name creator, const std::string& nick, const std::string& desc) {
    require_auth(_self);
    check(sym.is_valid(), "invalid symbol name");
    check(is_account(creator), "creator account does not exist");
    check(nick.size() <= 40, "nick name has more than 40 bytes");
    check(desc.size() <= 256, "symbol describe has more than 256 bytes");

    auto existing = symbolstables.find( sym.code().raw() );
    check( existing == symbolstables.end(), "token with symbol already exists" );

    symbolstables.emplace(_self, [&](auto& data) {
        data.sym = sym;
        data.creator = creator;
        data.nick = nick;
        data.desc = desc;
    });
}

ACTION nft::delsymbol(const symbol& sym) {
    require_auth(_self);
    check(sym.is_valid(), "invalid symbol name");
    auto existing = symbolstables.find( sym.code().raw() );
    check( existing != symbolstables.end(), "token with symbol does not exist" );
    symbolstables.erase(existing);
}

/*
//nh asset 
ACTION nft::createnh(name creator, name owner, const symbol& sym, std::string worldview, std::string basedesc) {
    check(is_account(creator), "creator account does not exist");
    check(is_account(owner), "owner account does not exist");
    check(sym.is_valid(), "invalid symbol name");
    check(basedesc.size() <= 256, "base describe has more than 256 bytes");
    check(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");
    require_auth(creator);

    // permission check
    auto admin_iter = admintables.find(creator.value);
    check(admin_iter != admintables.end() && admin_iter->flag & nft_permission_flags::nft_operation, 
        creator.to_string() + " should have worldview operation permission");

    id_type index_id = index_tables.available_primary_key();
    index_tables.emplace(_self, [&](auto& index_data) {
        index_data.id = index_id;
        index_data.status = 1;
    });

    // Create new nft
    auto time_now = time_point_sec(current_time_point());
    nft_tables.emplace(_self, [&](auto& nft_data) {
        nft_data.id = index_id;
        nft_data.creator = creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.sym = sym;
        nft_data.explain = basedesc;
        nft_data.createtime = time_now;
        nft_data.worldview = worldview;
    });

    auto nft_num = nftnumber_tables.find(owner.value);
    if(nft_num != nftnumber_tables.end()) {
        nftnumber_tables.modify(nft_num, creator, [&](auto& nft_num_data) {
            nft_num_data.number = nft_num->number+1;
        });
    } else {
        nftnumber_tables.emplace(_self, [&](auto& nft_num_data) {
            nft_num_data.owner = owner;
            nft_num_data.number = 1;
        });
    }
}

// ACTION nft::delnh(uint64_t id) {
//     auto iter = nft_tables.find(id);
//     check(iter != nft_tables.end(), "not exist in table");
//     nft_tables.erase(iter);
// }

// ///////// test end

ACTION nft::relatenh(name owner, uint64_t pid, uint64_t cid, const std::string& contractid, bool relate) {
    require_auth(owner);
    check(is_account(owner), "owner account does not exist");

    //check parent and nft asset id 
    auto piter = nft_tables.find(pid);
    check(piter != nft_tables.end(), " parent nft asset does not exist. asset id = " + std::to_string(pid));
    check(piter->owner == owner, "You're not the parent nh asset's owner, so you can't relate it.");

    auto citer = nft_tables.find(pid);
    check(citer != nft_tables.end(), " child nft asset does not exist. asset id = " + std::to_string(pid));
    check(citer->owner == owner, "You're not the child nh asset's owner, so you can't relate it.");

    const auto &iter = piter->child.find(contractid);
    if(relate) {
        if(iter != piter->child.end()) {
            check(find(iter->second.begin(), iter->second.end(), cid) == iter->second.end(), 
                "The parent item and child item had be related.");
            nft_tables.modify(piter, _self, [&](auto& d) { d.child[contractid].emplace_back(cid); });
            nft_tables.modify(citer, _self, [&](auto& d) { d.parent[contractid].emplace_back(pid); });
        } else {
            nft_tables.modify(piter, _self, [&](auto &d) { d.child[contractid] = vector<id_type> { cid }; });
            nft_tables.modify(citer, _self, [&](auto &d) { d.parent[contractid] = vector<id_type> {pid}; });
        }
    } else {
        check( iter != piter->child.end(), "The parent nh asset's parent dosen't contain this contract.");
        check(find(iter->second.begin(), iter->second.end(), cid) != iter->second.end(), 
            "The parent nh asset and child nh asset did not relate.");
        nft_tables.modify(piter, _self, [&](auto &d) {
            d.child[contractid].erase(find(d.child[contractid].begin(), d.child[contractid].end(), cid));
        });
        nft_tables.modify(citer, _self, [&](auto &d) {
            d.parent[contractid].erase(find(d.parent[contractid].begin(), d.parent[contractid].end(), pid));
        });
    }
}
*/

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

EOSIO_DISPATCH(nft, (addadmin)(updateperm)(deladmin)(addworldview)(delworldview)(addsymbol)(delsymbol) )

