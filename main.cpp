//
//  main.cpp
//  SkrambledBackend
//
//  Created by Seán Hargadon on 14/03/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <stdio.h>

#include <sqlite3.h>

#include <gmp.h>
#include <gmpxx.h>

struct database {
    sqlite3* db;
    sqlite3_stmt* res;
    int rc;
    
    database(std::string name)
    {
        this->rc = sqlite3_open(name.c_str(), &this->db);
        
        if (this->rc != SQLITE_OK) {
            printf("Error opening database\n");
            sqlite3_close(this->db);
        }
    }
    
    ~database()
    {
        sqlite3_close(this->db);
    }
    
    void insert(std::string table, std::string name, std::string key)
    {
        std::string command = "insert into " + table + " values(\"" + name + "\", \"" + key + "\");";
        
        rc = sqlite3_prepare_v2(this->db, command.c_str(), -1, &this->res, 0);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(this->db));
            return;
        }
        
        this->rc = sqlite3_step(this->res);
        
//        if (this->rc == SQLITE_ROW) {
//            printf("%s\n", sqlite3_column_text(this->res, 0));
//            printf("%s\n", sqlite3_column_text(this->res, 1));
//        }
        sqlite3_finalize(this->res);
    }
    
    void remove(std::string table, std::string name)
    {
        std::string command = "delete from " + table + " where name =\"" + name + "\";";
        
        rc = sqlite3_prepare_v2(this->db, command.c_str(), -1, &this->res, 0);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(this->db));
            return;
        }
        
        this->rc = sqlite3_step(this->res);
        
//        if (this->rc == SQLITE_ROW) {
//            printf("%s\n", sqlite3_column_text(this->res, 0));
//            printf("%s\n", sqlite3_column_text(this->res, 1));
//        }
        sqlite3_finalize(this->res);
    }

    std::pair<std::string, std::string> select(std::string table, std::string name)
    {
        std::pair<std::string, std::string> p;

        std::string command = "select * from " + table + " where name = \"" + name + "\";";
        
        rc = sqlite3_prepare_v2(this->db, command.c_str(), -1, &this->res, 0);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(this->db));
            return p;
        }
        
        this->rc = sqlite3_step(this->res);
        
        if (this->rc == SQLITE_ROW) {
            p.first = (char*)sqlite3_column_text(this->res, 0);
            p.second = (char*)sqlite3_column_text(this->res, 1);
        }
        sqlite3_finalize(this->res);
        
        return p;
    }
};


mpz_class gmp_random(int n)
{
    mpz_class result;
    mpz_t randNum;
    gmp_randstate_t gmpRandState;
    mpz_init(randNum);
    gmp_randinit_default(gmpRandState);
    gmp_randseed_ui(gmpRandState, time(NULL) );
    mpz_urandomb(randNum, gmpRandState, n);

    result = mpz_class(randNum);

    return result;
}

std::string gen_key(mpz_class n, mpz_class g, mpz_class m, mpz_class& m1)
{
    mpz_class r1;
    mpz_class r2;

    mpz_class y = gmp_random(128);
    
    mpz_powm(r1.get_mpz_t(), g.get_mpz_t(), y.get_mpz_t(), n.get_mpz_t());
    mpz_powm(r2.get_mpz_t(), m.get_mpz_t(), y.get_mpz_t(), n.get_mpz_t());
    
    m1 = r1;
    
    return r2.get_str();
}

std::string handshake(database& d, std::string name, std::string n, std::string g, std::string m)
{
    mpz_class n_num = mpz_class( n );
    mpz_class g_num = mpz_class( g );
    mpz_class m_num = mpz_class( m );
    mpz_class m1_num;

    std::string key = gen_key(n_num, g_num, m_num, m1_num);
    
    d.insert("unverified", name, key);
    
    return m1_num.get_str();
}

int authorise(database& d, std::string name, std::string c_key, std::string u_key, std::string sig)
{

    return 1;
}

int main(int argc, const char * argv[])
{
    int s = atoi( argv[1] );
    std::string name = argv[2];
    std::string n = argv[3];
    std::string g = argv[4];
    std::string m = argv[5];
    std::string c_key = argv[6];
    std::string u_key = argv[7];
    std::string sig = argv[8];
    std::string t = argv[9];

    
    database d = database("/home/seanlth/Documents/C++/Skrambled-Back-End/users.db");

    
    switch (s) {
        case 0:
            std::cout << handshake(d, name, n, g, m);
            break;
        case 1:
            authorise(d, name, c_key, u_key, sig);
            break;

        default:
            break;
    }
    
    
    
//    mpz_class n;
//    mpz_nextprime( n.get_mpz_t(), gmp_random(256).get_mpz_t() );
//
//    mpz_class g = gmp_random(256);
//    mpz_class x = gmp_random(256);
//    mpz_class m;
//    mpz_class m1;
//
//    mpz_powm(m.get_mpz_t(), g.get_mpz_t(), x.get_mpz_t(), n.get_mpz_t());
//
//    mpz_class m2;
//    
//    std::cout << gen_key(n, gmp_random(256), m, m1) << std::endl;
    
    
    
//    int s = atoi( argv[1] );
//    
//    switch (s) {
//        case 0:
//            handshake(argv[2], argv[3], argv[4], argv[5]);
//            
//            
//            break;
//            
//        default:
//            break;
//    }
    
//    mpz_class a = 2;
//    mpz_class b = 10;
//    mpz_class c;
//    mpz_class n = 100;
//
//    mpz_powm(c.get_mpz_t(), a.get_mpz_t(), b.get_mpz_t(), n.get_mpz_t());
//    
//    std::cout << c.get_str() << std::endl;
//    
//    database d = database("test.db");
//    
//    d.insert("verified", "conor", "12312312312312");
//    auto p = d.select("verified", "conor");
//    
//    std::cout << p.first << std::endl;
//    std::cout << p.second << std::endl;
//    
//    d.remove("verified", "conor");
//
//    p = d.select("verified", "conor");
//    
//    std::cout << p.first << std::endl;
//    std::cout << p.second << std::endl;
    
//    int rc = sqlite3_open("test.db", &d.db);
//    
//    
//    if (rc != SQLITE_OK) {
//        printf("Error opening database\n");
//        sqlite3_close(d.db);
//    }
//    
//    rc = sqlite3_prepare_v2(d.db, "insert into verified values(\"Lo2l\", 123123123123);", -1, &d.res, 0);
//    
//    if (rc != SQLITE_OK) {
//        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(d.db));
//        sqlite3_close(d.db);
//        
//        return 1;
//    }
//    
//    rc = sqlite3_step(d.res);
//    
//    if (rc == SQLITE_ROW) {
//        printf("%s\n", sqlite3_column_text(d.res, 0));
//        printf("%s\n", sqlite3_column_text(d.res, 1));
//    }
//    
//    sqlite3_finalize(d.res);
//    sqlite3_close(d.db);
    
    return 0;
}
