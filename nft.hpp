#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>

#include <string>
#include <vector>
#include <map>
#include <utility>

using namespace eosio;
using std::string;
using std::vector;
using std::map;

#define N(X)        eosio::name{#X}
#define S(P,X)      eosio::symbol(#X,P)
#define EOS_SYMBOL  S(4, EOS)

typedef uint128_t uuid;
typedef uint64_t id_type;
typedef string uri_type;

int64_t FEE = 1;  //0.0001 EOS

size_t sub2sep( const std::string& input, std::string& output, const char& separator,
    const size_t& first_pos = 0, const bool& required = false ) {

    check(first_pos != std::string::npos, "invalid first pos");
    auto pos = input.find(separator, first_pos);
    if (pos == std::string::npos) {
        check(!required, "parse memo error");
        return std::string::npos;
    }
    output = input.substr(first_pos, pos - first_pos);
    return pos;
}

//参照string_to_name，主要是给table中二级索引使用的，由于二级索引可以重复，所以这里并不是严格的hash
static constexpr uint64_t string_to_uint64( const char* str )
{   
    uint64_t key = 0;
    int i = 0;
    for ( ; str[i] && i < 12; ++i) {
        key |= (uint64_t(str[i]) & 0x1f) << (64 - 5 * (i + 1));
    }
    return key;
}

enum nft_permission_flags
{
    worldview_operation = 0x01,   
    nft_operation = 0x02
    // white_list = 0x02,                  // 0000 0010
    // override_authority = 0x04,          // 0000 0100
    // transfer_restricted = 0x08,         // 0000 1000
    // disable_force_settle = 0x10,        // 0001 0000
    // global_settle = 0x20,               // 0010 0000
    // disable_confidential = 0x40,        // 0100 0000
    // witness_fed_asset = 0x80,           // 1000 0000
    // committee_fed_asset = 0x100         // 1 0000 0000
};

CONTRACT nft : public eosio::contract {
    public:
        using contract::contract;

        nft(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds), 
            admintables(receiver, receiver.value),
            worldviewstables(receiver, receiver.value),
            symbolstables(receiver, receiver.value)

            //nfttables(receiver, receiver.value),
        {}
       
        // ACTION addadmin(name admin);
        // ACTION deladmin(name admin);

        // ACTION create(name creator, name owner, std::string explain, std::string worldview);

        void transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);

        ACTION addadmin(name account, uint32_t flag);
        ACTION updateperm(name account, uint32_t flag);
        ACTION deladmin(name account);

        ACTION addworldview(name creator, const std::string& worldview);
        ACTION delworldview(uint64_t id, name owner);

        ACTION addsymbol(const symbol& sym, name creator, const std::string& nick, const std::string& desc);
        ACTION delsymbol(const symbol& sym);

        TABLE admins {
            name            account;
            uint32_t        flag;

            uint64_t primary_key() const { return account.value; }
        };

        TABLE worldviews {
            id_type         id;
            id_type         worldviewkey;  //worldview --> uint64_t
            name            creator;
            std::string     worldview;

            uint64_t primary_key() const { return id; }
            uint64_t get_creator() const { return creator.value; }
            uint64_t get_wvkey() const { return worldviewkey; }
        };

        TABLE symbols {
            symbol          sym;
            name            creator;
            std::string     nick;
            std::string     desc;

            uint64_t primary_key() const { return sym.code().raw(); }
        };

        // TABLE nftts {
        //     id_type         id;
        //     name            creator;
        //     name            owner;
        //     name            auth;
        //     std::string     explain;
        //     time_point_sec  createtime;
        //     std::string     worldview;
        //     std::map<std::string, std::string> attr;
        //     //id_type composeattr;

        //     uint64_t primary_key() const { return id; }
        //     uint64_t get_owner() const { return owner.value; }
        //     uint64_t get_creator() const { return creator.value; }
        // };
        


        using adminsindex = eosio::multi_index<"admins"_n, admins>;

        using worldviewsindex = eosio::multi_index<"worldviews"_n, worldviews,
            indexed_by< "bycreator"_n, const_mem_fun< worldviews, uint64_t, &worldviews::get_creator> >,
            indexed_by< "bywvkey"_n, const_mem_fun< worldviews, uint64_t, &worldviews::get_wvkey> >>;

        using symbolsindex = eosio::multi_index< "symbols"_n, symbols >;
 

        // using nfts_index = eosio::multi_index<"nftts"_n, nftts,
        //     indexed_by< "byowner"_n, const_mem_fun< nftts, uint64_t, &nftts::get_owner> >,
        //     indexed_by< "bycreator"_n, const_mem_fun< nftts, uint64_t, &nftts::get_creator> >>;

    private:
        adminsindex        admintables;
        worldviewsindex    worldviewstables;
        symbolsindex       symbolstables;

        //nftsindex          nft_tables;

};

