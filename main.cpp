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
#include <stdlib.h>
#ifdef __linux
#include <bsd/stdlib.h>
#endif


#include <sqlite3.h>
#include <sstream>

#include <gmp.h>
#include <gmpxx.h>
#include <openssl/aes.h>
#include <openssl/rand.h>


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
        remove(table, name);
        
        std::string command = "insert into " + table + " values(\"" + name + "\", \"" + key + "\");";
        
        rc = sqlite3_prepare_v2(this->db, command.c_str(), -1, &this->res, 0);
        
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(this->db));
            return;
        }
        
        this->rc = sqlite3_step(this->res);
        
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

unsigned char* encrypt(const char* input, const char* key, int* size)
{
    size_t inputslength = std::strlen(input) + 1;

    const size_t encslength = ((inputslength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* aes_input = new unsigned char[encslength];
    memcpy(aes_input, input, inputslength+1);
    
    unsigned char* aes_key = (unsigned char*)key;
    
    int keylength = 128;
    
    
    unsigned char iv_dec[AES_BLOCK_SIZE], iv_enc[AES_BLOCK_SIZE];
    memset(iv_enc, 1, AES_BLOCK_SIZE);
    memcpy(iv_dec, iv_enc, AES_BLOCK_SIZE);
    
    unsigned char* enc_out = (unsigned char*)malloc( sizeof(unsigned char) * (encslength) );
    memset(enc_out, 0, encslength);
    
    AES_KEY enc_key;
    AES_set_encrypt_key(aes_key, keylength, &enc_key);
    AES_cbc_encrypt(aes_input, enc_out, encslength, &enc_key, iv_enc, AES_ENCRYPT);
    
    *size = encslength;
    
    delete [] aes_input;
    
    return enc_out;
}

const char* decrypt(unsigned char* input, int inputslength, const char* key)
{
    int length = ((inputslength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* aes_input = new unsigned char[length];
    memcpy(aes_input, input, inputslength);

    unsigned char* aes_key = (unsigned char*)key;
    
    int keylength = 128;
    
    unsigned char iv_dec[AES_BLOCK_SIZE], iv_enc[AES_BLOCK_SIZE];
    memset(iv_enc, 1, AES_BLOCK_SIZE);
    memcpy(iv_dec, iv_enc, AES_BLOCK_SIZE);
    
    const size_t dec_length = length; //((inputslength / AES_BLOCK_SIZE) * AES_BLOCK_SIZE) + AES_BLOCK_SIZE;
    unsigned char* dec_out =  (unsigned char*)malloc( sizeof( unsigned char) * dec_length );
    memset(dec_out, 0, dec_length);
    
    AES_KEY dec_key;
    
    AES_set_decrypt_key(aes_key, keylength, &dec_key);
    AES_cbc_encrypt(aes_input, dec_out, dec_length, &dec_key, iv_dec, AES_DECRYPT);
    
    delete [] aes_input;
    
    return (const char*)dec_out;
}

std::string toHex(const char* str, int l)
{
    std::string mystr = "";
    
    for(int i=0; i < l; i++) {
        unsigned int v = (unsigned char)str[i];
        
        std::stringstream ss(str);
        ss << std::hex << std::setfill('0') << std::setw(2) << v;
        std::string t = ss.str();
        mystr += t[0];
        mystr += t[1];
    }
    
    return mystr;
}

std::string fromHex(const char* str)
{
    unsigned int x;
    
    std::string mystr = "";
    
    size_t l = strlen(str);
    
    for(int i=0; i < l; i+=2) {
        std::stringstream ss;
        
        const char t[2] = {str[i], str[i+1]};
        ss << std::hex << std::setfill('0') << std::setw(2) << t;
        ss >> x;
        char y = x;
        mystr += y;
    }
    
    return mystr;
}

std::string hex_encrypt(std::string input, std::string key)
{
    int size;
    unsigned char* x = encrypt(input.c_str(), key.c_str(), &size);
    std::string hex = toHex((const char*)x, size);
    free(x);
    return hex;
}

std::string hex_decrypt(std::string input, std::string key)
{
    std::string y = fromHex(input.c_str());
    const char* x = decrypt((unsigned char*)y.data(), (int)y.size(), key.c_str());
    std::string x_str = std::string(x);
    free((char*)x);
    return x_str;
}



mpz_class gmp_random(int n)
{
    mpz_class result;
    mpz_t randNum;
    gmp_randstate_t gmpRandState;
    mpz_init(randNum);
    gmp_randinit_default(gmpRandState);
    gmp_randseed_ui(gmpRandState, arc4random());
    mpz_urandomb(randNum, gmpRandState, n);

    result = mpz_class(randNum);

    return result;
}

std::string gen_key(mpz_class n, mpz_class g, mpz_class m, mpz_class& m1)
{
    mpz_class r1;
    mpz_class r2;
    
    mpz_class y = gmp_random(512);
    
    mpz_powm(r1.get_mpz_t(), g.get_mpz_t(), y.get_mpz_t(), n.get_mpz_t());
    mpz_powm(r2.get_mpz_t(), m.get_mpz_t(), y.get_mpz_t(), n.get_mpz_t());
    
    m1 = r1;
    
    return r2.get_str();
}

bool authenticate(std::string c_key, std::string u_key, std::string nonce, std::string sig, std::string timestamp)
{
    std::string request = "curl -s --get 'https://api.twitter.com/1.1/account/verify_credentials.json' --header 'Authorization: OAuth oauth_consumer_key=\"" + c_key + "\", oauth_nonce=\"" + nonce + "\", oauth_signature=\"" + sig + "\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\"" + timestamp + "\", oauth_token=\"" + u_key + "\", oauth_version=\"1.0\"' --fail --silent --show-error 2>&1 -o /dev/null";
    
    FILE *f = popen(request.c_str(), "r");
    
    char r[512];
    
    if (f == NULL) {
        return false;
    }
    while ( fgets(r, sizeof(r), f) != NULL) {
        return false;
    }
    
    pclose(f);
    
    return true;
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

int authorise(database& d, std::string name, std::string c_key, std::string u_key, std::string sig, std::string nonce, std::string timestamp)
{
    bool success = authenticate(c_key, u_key, nonce, sig, timestamp);
    
    if (success) {
        std::pair<std::string, std::string> r = d.select("unverified", name);
        d.insert("verified", name, r.second);
        d.remove("unverified", name);
        return 1;
    }
    else {
        return 0;
    }
    
    return 1;
}


int main(int argc, const char * argv[])
{
    database d = database("/home/seanlth/Documents/C++/Skrambled-Back-End/users.db");
    
    int s = atoi( argv[1] );
    std::string name = argv[2];
    std::string n = argv[3];
    std::string g = argv[4];
    std::string m = argv[5];
    std::string c_key = argv[6];
    std::string u_key = argv[7];
    std::string sig = argv[8];
    std::string nonce = argv[9];
    std::string timestamp = argv[10];
    std::string t = argv[11];

    switch (s) {
        case 0:
        {
            std::pair<std::string, std::string> user = d.select("member", name);
            if (user.first.compare( name ) == 0) {
                std::cout << handshake(d, name, n, g, m);
            }
            else {
                std::cout << "0";
            }
            
            break;
        }
        case 1:
        {
            std::pair<std::string, std::string> user = d.select("unverified", name);
            
            std::string key = user.second;
            
            std::string consumer_key = hex_decrypt(c_key.c_str(), key.c_str());
            std::string user_key = hex_decrypt(u_key.c_str(), key.c_str());
            std::string dehexed_sig = fromHex(sig.c_str());
            
            std::cout << authorise(d, name, consumer_key, user_key, dehexed_sig, nonce, timestamp);
            break;
        }
        case 2:
        {
            std::pair<std::string, std::string> user = d.select("verified", name);
            std::pair<std::string, std::string> group = d.select("member", name);
            std::string group_key = group.second;
            
            std::string key = user.second;
            
            std::string tweet = hex_decrypt(t.c_str(), key.c_str());
            std::string encrypted_tweet = hex_encrypt(tweet.c_str(), group_key.c_str());
            
            std::cout << encrypted_tweet;
            break;
        }
        case 3:
        {
            std::pair<std::string, std::string> user = d.select("verified", name);
            std::pair<std::string, std::string> group = d.select("member", name);
            std::string group_key = group.second;
            
            std::string key = user.second;
            
            std::string tweet = hex_decrypt(t.c_str(), group_key.c_str());
            std::string new_tweet = hex_encrypt(tweet.c_str(), key.c_str());
            
            std::cout << new_tweet;
            break;
        }
        case 4:
        {
            std::pair<std::string, std::string> user = d.select("verified", name);
            
            std::string msg = hex_decrypt(t.c_str(), user.second.c_str());
            
            if (user.first == name && msg.compare("verified") == 0) {
                std::cout << 1;
            }
            else {
                std::cout << 0;
            }
            
            break;
        }

        default:
            break;
    }
    
    return 0;
}
