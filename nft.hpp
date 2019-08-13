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

//Refer to string_to_name, mainly for the secondary index in the table, 
//because the secondary index can be repeated, so here is not a strict hash
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
    // 0x04 0x08 0x10 0x20 0x40 0x80 0x100
};

CONTRACT nft : public eosio::contract {
    public:
        using contract::contract;

        nft(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds), 
            admintable(receiver, receiver.value),
            worldviewstable(receiver, receiver.value),
            symbolstable(receiver, receiver.value),
            nhassetstable(receiver, receiver.value),
            nhrelatestable(receiver, receiver.value)
        {}

        void transfer(const name& from, const name& to, const asset& quantity, const std::string& memo);

        ACTION addadmin(name account, uint32_t flag);
        ACTION updateperm(name account, uint32_t flag);
        ACTION deladmin(name account);

        ACTION addworldview(name creator, const std::string& worldview);
        ACTION delworldview(uint64_t id, name owner);

        ACTION addsymbol(const symbol& sym, name creator, const std::string& nick, const std::string& desc);
        ACTION delsymbol(const symbol& sym);

        ACTION createnh(name creator, name owner, const symbol& sym, const std::string& worldview, const std::string& basedesc);
        ACTION relatenh(name owner, uint64_t pid, uint64_t cid, const std::string& contractid, bool relate);
        ACTION burn(uint64_t id, name owner);

        // //test
        // ACTION delnh(uint64_t id);
        // ACTION delrel(uint64_t id);

        void transfernh(const name& from, const name& to, uint64_t asset);

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
            uint64_t get_key() const { return worldviewkey; }
        };

        TABLE symbols {
            symbol          sym;
            name            creator;
            std::string     nick;
            std::string     desc;

            uint64_t primary_key() const { return sym.code().raw(); }
        };

        TABLE nhassets {
            id_type         id;
            name            creator;
            name            owner;
            symbol          sym;
            name            auth;
            std::string     explain;
            time_point_sec  createtime;
            std::string     worldview;
            map<std::string, id_type> conrel;  //contractID - relateID

            uint64_t primary_key() const { return id; }
            uint64_t get_owner() const { return owner.value; }
            uint64_t get_creator() const { return creator.value; }
        };

        TABLE nhrelates {
            id_type         id;
            id_type         contractkey;
            std::string     contract;  //contract_id or game_id
            vector<id_type> parent;    //asset id
            vector<id_type> child;     //nh asset id

            uint64_t primary_key() const { return id; }
            uint64_t get_key() const { return contractkey; }
        };

        using adminsindex = eosio::multi_index<"admins"_n, admins>;

        using worldviewsindex = eosio::multi_index<"worldviews"_n, worldviews,
            indexed_by< "bycreator"_n, const_mem_fun< worldviews, uint64_t, &worldviews::get_creator> >,
            indexed_by< "bykey"_n, const_mem_fun< worldviews, uint64_t, &worldviews::get_key> >>;

        using symbolsindex = eosio::multi_index< "symbols"_n, symbols >;

        using nhassetsindex = eosio::multi_index<"nhassets"_n, nhassets,
            indexed_by< "byowner"_n, const_mem_fun< nhassets, uint64_t, &nhassets::get_owner> >,
            indexed_by< "bycreator"_n, const_mem_fun< nhassets, uint64_t, &nhassets::get_creator> >>;

        using nhrelatesindex = eosio::multi_index<"nhrelates"_n, nhrelates,
            indexed_by< "bykey"_n, const_mem_fun< nhrelates, uint64_t, &nhrelates::get_key> >>;

    private:
        adminsindex        admintable;
        worldviewsindex    worldviewstable;
        symbolsindex       symbolstable;
        nhassetsindex      nhassetstable;
        nhrelatesindex     nhrelatestable;

};

