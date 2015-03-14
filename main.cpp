//
//  main.cpp
//  SkrambledBackend
//
//  Created by Seán Hargadon on 14/03/2015.
//  Copyright (c) 2015 Seán Hargadon. All rights reserved.
//

#include <iostream>
#include <sqlite3.h>
#include <stdio.h>
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
        
        if (this->rc == SQLITE_ROW) {
            printf("%s\n", sqlite3_column_text(this->res, 0));
            printf("%s\n", sqlite3_column_text(this->res, 1));
        }
        sqlite3_finalize(this->res);
    }
    
};


std::string gen_key(mpz_class n, mpz_class g, mpz_class m, mpz_class& m1)
{
    int y = 3;
    mpz_class r;
    
    mpz_pow_ui(r.get_mpz_t(), m.get_mpz_t(), y);
    
    
    return "";
}

void handshake(database& d, std::string name, std::string n, std::string g, std::string m)
{
    mpz_class n_num;
    mpz_class g_num;
    mpz_class m_num;
    mpz_class m1_num;

    mpz_set_str(n_num.get_mpz_t(), n.c_str(), 10);
    mpz_set_str(n_num.get_mpz_t(), n.c_str(), 10);
    mpz_set_str(n_num.get_mpz_t(), n.c_str(), 10);
    mpz_set_str(n_num.get_mpz_t(), n.c_str(), 10);

    
    std::string key = gen_key(n_num, g_num, m_num, m1_num);
    
    d.insert("unverified", name, key);
}







int main(int argc, const char * argv[])
{
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
    
    database d = database("test.db");
    
    d.insert("verified", "conor", "12312312312312");
    
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
    
    printf("Goodbyte");
    
    return 0;
}
