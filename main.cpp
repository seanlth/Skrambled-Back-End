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
    
    //std::cout << "Maybe" << std::endl;

    
    if (success) {
        //std::cout << "Success" << std::endl;
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
//    std::stringstream str;
//    std::string s1 = "lololol";
//    str << s1;
//    int value;
//    str >> std::hex >> value
    
    
    
    
//    const char* hello = "H";
//
//    
//    std::stringstream ss(hello);
//    
//    for(int i=0; i < strlen(hello); i++) {
//        ss << std::hex << (int)hello[i];
//    }
//    std::string mystr = ss.str();
//    
//    std::cout << mystr << std::endl;
    
    
//    auto x = hex_encrypt("moooo mogfhfhfhfhfhfhgfoo", "31317972834947517761498882765646248434");
//    auto y = hex_decrypt(x.c_str(), "31317972834947517761498882765646248434");
    
    
//    auto x = encrypt("mooo mooo", "31317972834947517761498882765646248434");
//    auto x_p = toHex((const char*)x);
//    auto x_pp = x_p.c_str();
//    auto y_p = fromHex(x_p.c_str());
//    auto y_pp = y_p.c_str();
//    
//    auto y = decrypt((unsigned char*)y_p.c_str(), "31317972834947517761498882765646248434");
    
    

//    const unsigned char in[5] = {0xb2, 7, 'c', 'q', 0};
//    
//    auto x = toHex((const char*)in);
//    auto y = fromHex(x.c_str());
//    
//    auto p = x.c_str();
    
//    int c = 0xb2;
//    
//    std::stringstream ss;
//    ss << std::hex << std::setfill('0') << std::setw(2) << c;
//
//    std::cout << ss.str();
    
    
    //std::cout << y << std::endl;
    
    
//    std::string h = toHex("hello");
//    std::cout << fromHex(h.c_str());
    
    
    
//    bool r = authenticate("cfaI934yhEnjyGP9dwGTWseMy", "339861025-ZIRz9U0POUaWlW40UsMXZOyEXaCVAHbfQpkAuG37", "21C90CD3-4531-4513-ABB2-8E7DF076F772", "1sMQF94ftTIad33gKlAVgyp%2Bh5M%3D", "1427029172");
//    
//    std::cout << (r == true ? "pass" : "fail") << std::endl;
    
    
//    const char* s = "636F6E737420636861722A2073";
//    
//    //char* s_dec = new char[strlen(s)/2];
//    std::cout <<  from_hex_string(s, strlen(s)) << std::endl;
    
    
//    std::string input = "123123123123121231231231231222312";
//    
//    
//    from_hex_string(to_hex_string(input.c_str(), input.size()), 2);
//    
//    
    
    
    //e78cb7bd01077236742da18268de43eedfd415b1da8783df82bf37d0cc8fe115faa83da15c9e35d83cb7a0673b5887c2
    //e78cb7bd01077236742da18268de43eedfd415b1da8783df82bf37d0cc8fe115ac9aa60899d4f465d1b0facdac49a12d
    
    
    //std::string cons = "Alll is well in the land of twitter           ";
    std::string cons = "Alll is well in the land of twitter";
    std::cout << cons.size() << std::endl;
    int size = 0;
    auto x = encrypt(cons.c_str(), "51924831484719428842179641947340427938", &size);
    std::cout << size << std::endl;

    std::cout << toHex((const char*)x, size) << std::endl;
    auto y = decrypt(x, size, "51924831484719428842179641947340427938");
    std::cout << y << std::endl;

    
    return 1;
//    std::cout << cons.size() << std::endl;
//    std::cout << AES_BLOCK_SIZE << std::endl;
//    
 //   std::string x = hex_encrypt(cons.data(), "51924831484719428842179641947340427938");
//    
//    //std::string x = "1423220A0076C83BEF67C2BCE2A4137B9A65F687E776C8CC441FF31456CA8D4788124153F7EEA11DB9165728DD351F38";
//    
//    std::string y = hex_decrypt(x, "51924831484719428842179641947340427938");
//    
    std::cout << y << std::endl;
//    std::cout << x << std::endl;
//
    return 1;
    
    
//
//    free((char*)x);
//    free((char*)y);
    
    
//    std::string x = (const char*)encrypt(input.c_str(), "31317972834947517761498882765646248434");
//    std::string y = to_hex_string(x.c_str(), x.size());
//    std::string y_p = from_hex_string(y.c_str(), y.size());
//    std::string x_p = (const char*)decrypt((unsigned char*)x.c_str(), "31317972834947517761498882765646248434");
//    std::cout << x_p << std::endl;
    
    
    //auto asd = gmp_random(128);
    //std::cout << asd.get_str() << std::endl;
    
    database d = database("/home/seanlth/Documents/C++/Skrambled-Back-End/users.db");
    //database d = database("test.db");
    
    //d.insert("unverified", "JhaygoreDiego", "51924831484719428842179641947340427938");
    
    
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
            std::cout << handshake(d, name, n, g, m);
            break;
        }
        case 1:
        {
            std::pair<std::string, std::string> user = d.select("unverified", name);
            
            //std::string key = "51924831484719428842179641947340427938";
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
            
            std::string group_key = "51924831484719428842179641947340427938";
            std::string key = user.second;
            
            std::string tweet = hex_decrypt(t.c_str(), key.c_str());
            std::string encrypted_tweet = hex_encrypt(tweet.c_str(), group_key.c_str());
            
            std::cout << encrypted_tweet;
            break;
        }
        case 3:
        {
            std::pair<std::string, std::string> user = d.select("verified", name);
            
            std::string group_key = "51924831484719428842179641947340427938";
            std::string key = user.second;
            
            std::string tweet = hex_decrypt(t.c_str(), group_key.c_str());
            std::string new_tweet = hex_encrypt(tweet.c_str(), key.c_str());
            
            std::cout << new_tweet;
            break;
        }
        case 4:
        {
            std::pair<std::string, std::string> user = d.select("verified", name);
            
            if (user.first == name) {
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
