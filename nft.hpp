#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>

#include <string>
#include <vector>

using namespace eosio;
using std::string;
using std::vector;
typedef uint128_t uuid;
typedef uint64_t id_type;
typedef string uri_type;

CONTRACT nft : public eosio::contract 
{
    public:
	    using contract::contract;

        nft(name receiver, name code, datastream<const char*> ds)
		    : contract(receiver, code, ds), 
            nft_tables(receiver, receiver.value),
            accauth_tables(receiver, receiver.value),
            nftchain_tables(receiver, receiver.value),
            compose_tables(receiver, receiver.value),
            game_tables(receiver, receiver.value),
            assetmap_tables(receiver, receiver.value),
            admin_tables(receiver, receiver.value),
            composeattr_tables(receiver, receiver.value),
            index_tables(receiver, receiver.value),
            nftnumber_tables(receiver, receiver.value)
        {}
       
        ACTION addadmin(name admin);
        ACTION deladmin(name admin);

        ACTION create(name creator, name owner, std::string explain, std::string worldview);
        ACTION createother(name creator, name owner, std::string explain, std::string worldview, id_type chainid, id_type targetid);

        ACTION addaccauth(name owner, name auth);
        ACTION delaccauth(name owner);
        
        ACTION addnftauth(name owner, name auth, id_type id);
        ACTION delnftauth(name owner, id_type id);
        ACTION transfer(name from, name to, id_type id, string memo);
        ACTION addchain(name owner, string chain);
        ACTION setchain(name owner, id_type chainid, id_type status);
        ACTION addcompattr(name owner, id_type id);
        ACTION delcompattr(name owner, id_type id);
        ACTION setcompose(name owner, id_type firid, id_type secid);
        ACTION delcompose(name owner, id_type firid, id_type secid);
        ACTION addgame(name owner, std::string gamename, std::string introduces);
        ACTION editgame(name owner, id_type gameid, std::string gamename, std::string introduces);
        ACTION setgame(name owner, id_type gameid, id_type status);
        ACTION delgame(name owner, id_type gameid);
        ACTION addgameattr(name owner, id_type gameid, string key, string value);
        ACTION editgameattr(name owner, id_type gameid, string key, string value);
        ACTION delgameattr(name owner, id_type gameid, string key);
        ACTION addmapping(name owner, id_type fromid, id_type targetid, id_type chainid);
        ACTION delmapping(name owner, id_type fromid, id_type chainid);
        ACTION burn(name owner, id_type nftid);

        TABLE admins
        {
            name admin;
            uint64_t primary_key() const { return admin.value; }
        };

        TABLE nftindexs
        {
            id_type id;
            id_type status;
            uint64_t primary_key() const { return id; }
            uint64_t get_status() const { return status; }
        };

        TABLE nftnumber
        {
            name owner;
            id_type number;
            uint64_t primary_key() const { return owner.value; }
        };

        TABLE nftts
        {
            id_type id;
            name creator;
            name owner;
            name auth;
            std::string explain;
            time_point_sec createtime;
            //std::map<string, string> attr;
            //id_type composeattr;
            std::string worldview;            
            uint64_t primary_key() const { return id; }
            uint64_t get_owner() const { return owner.value; }
            uint64_t get_creator() const { return creator.value; }
            
        };

        TABLE composeattr
        {
            id_type nftid;
            uint64_t primary_key() const { return nftid; }
        };
        
        TABLE accauth 
        {
            name owner;
            name auth;
            uint64_t primary_key() const { return owner.value; }
            uint64_t get_auth() const { return auth.value; }
        };

        TABLE nftchains{
            id_type chainid;
            string chain;
            id_type status;
            uint64_t primary_key() const { return chainid; }
            uint64_t get_status() const { return status; }
        };

        TABLE composes
        {
            id_type id;
            id_type firid;
            id_type secid;
            id_type status;
            uint64_t primary_key() const { return id; }
            uint64_t get_fir() const { return firid; }
            uint64_t get_sec() const { return secid; }
            uint64_t get_status() const { return status; }
        };

        TABLE nftgame
        {
            id_type gameid;
            std::string gamename;
            std::string introduces;
            id_type status;
            id_type index;
            time_point_sec createtime;
            std::map<string, string> gameattr;
            uint64_t primary_key() const { return gameid; }
            uint64_t get_status() const { return status; }
            uint64_t get_index() const { return index; }
        };

        TABLE assetmapes 
        {
            id_type mappingid;
            id_type fromid;
            id_type targetid;
            id_type chainid;
            uint64_t primary_key() const { return mappingid; }
            uint64_t get_fromid() const { return fromid; }
            uint64_t get_targetid() const { return targetid; }
            uint64_t get_chainid() const { return chainid; }
        };

        using admins_index = eosio::multi_index<"admins"_n, admins>;

        using nftindex_index = eosio::multi_index<"nftindexs"_n, nftindexs,
            indexed_by< "bystatus"_n, const_mem_fun< nftindexs, uint64_t, &nftindexs::get_status> > >;

        using nftnumber_index = eosio::multi_index<"nftnumber"_n, nftnumber>;

        using composeattr_index = eosio::multi_index<"composeattr"_n, composeattr>;

        using nfts_index = eosio::multi_index<"nftts"_n, nftts,
            indexed_by< "byowner"_n, const_mem_fun< nftts, uint64_t, &nftts::get_owner> >,
            indexed_by< "bycreator"_n, const_mem_fun< nftts, uint64_t, &nftts::get_creator> >>;

        using accauth_index = eosio::multi_index<"accauth"_n, accauth,
            indexed_by< "byauth"_n, const_mem_fun< accauth, uint64_t, &accauth::get_auth> > >;

        using nftchain_index = eosio::multi_index<"nftchains"_n, nftchains,
            indexed_by< "bystatus"_n, const_mem_fun< nftchains, uint64_t, &nftchains::get_status> > >;

        using compose_index = eosio::multi_index<"composes"_n, composes,
            indexed_by< "byfir"_n, const_mem_fun< composes, uint64_t, &composes::get_fir> >,
            indexed_by< "bysec"_n, const_mem_fun< composes, uint64_t, &composes::get_sec> >,
            indexed_by< "bystatus"_n, const_mem_fun< composes, uint64_t, &composes::get_status> > >;

        using nftgame_index = eosio::multi_index<"nftgame"_n, nftgame,
            indexed_by< "byindex"_n, const_mem_fun< nftgame, uint64_t, &nftgame::get_index> >,
            indexed_by< "bystatus"_n, const_mem_fun< nftgame, uint64_t, &nftgame::get_status> > >;
            
        using assetmapp_index = eosio::multi_index<"assetmapes"_n, assetmapes,
            indexed_by< "byfromid"_n, const_mem_fun< assetmapes, uint64_t, &assetmapes::get_fromid> >,
            indexed_by< "bytargetid"_n, const_mem_fun< assetmapes, uint64_t, &assetmapes::get_targetid> >,
            indexed_by< "bychainid"_n, const_mem_fun< assetmapes, uint64_t, &assetmapes::get_chainid> > >;

    private:
        admins_index        admin_tables;
        nftnumber_index     nftnumber_tables;
        nftindex_index      index_tables;
        composeattr_index   composeattr_tables;
        nfts_index          nft_tables;
        accauth_index       accauth_tables;
        nftchain_index      nftchain_tables;
        compose_index       compose_tables;
        nftgame_index       game_tables;
        assetmapp_index     assetmap_tables;
};

