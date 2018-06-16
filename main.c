//#ifdef STANDARD
///* STANDARD is defined, don't use any mysql functions */
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#ifdef __WIN__
//typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
//typedef __int64 longlong;
//#else
//typedef unsigned long long ulonglong;
//typedef long long longlong;
//#endif /*__WIN__*/
//#else
//#include <my_global.h>
//#include <my_sys.h>
//#if defined(MYSQL_SERVER)
//#include <m_string.h>		/* To get strmov() */
//#else
///* when compiled as standalone */
//#include <string.h>
//#define strmov(a,b) stpcpy(a,b)
//#endif
//#endif
//#include <mysql.h>
//#include <ctype.h>
//
//#ifdef _WIN32
///* inet_aton needs winsock library */
//#pragma comment(lib, "ws2_32")
//#endif

#include <stdio.h>
#include <stdlib.h>

/* Require homomorphic cryptosystem and gmp library*/
#include <libhcs.h>
#include <gmp.h>

#ifdef HAVE_DLOPEN

#if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
static pthread_mutex_t LOCK_hostname;
#endif

#endif /* HAVE_DLOPEN */


/*
 * Mark : Declaration of keys and global variables
 *
 */

struct CryptoKey {
    pcs_public_key *pubKey;
    pcs_private_key *privKey;
    hcs_random *hcsRandom;
};

mpz_t e_balanceSum;

int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9 ,10, 11, 12, 13, 14, 15, 16 ,17 ,18, 19};


/*
 * Mark : Declaration of User-Defined Functions (UDF)
 *
 */

void udf_init(struct CryptoKey** cryptoKey);
void udf_add(int i, struct CryptoKey** cryptoKey);
//void udf_clear();
//void udf_reset();



/*
 * Mark : Definition of User-Defined Functions (UDF)
 *
 */

void udf_init(struct CryptoKey** cryptoKey){

// Step 1 : Initialize the one Crypto-System Key
    struct CryptoKey *CSKeys = (struct CryptoKey*)malloc(sizeof(struct CryptoKey));
    CSKeys->pubKey = pcs_init_public_key();
    CSKeys->privKey = pcs_init_private_key();
    CSKeys->hcsRandom = hcs_init_random();


    // Step 3 :
    //  *cryptoKey = CSKeys
    //  pcs_generate_key_pair((*cryptoKey)->pubKey, (*cryptoKey)->privKey, (*cryptoKey)->hcsRandom, 2048);

    pcs_generate_key_pair(CSKeys->pubKey, CSKeys->privKey, CSKeys->hcsRandom, 2048);


// Step 2 :
    mpz_set_ui(e_balanceSum,0);
    gmp_printf("init e_balance to zero: %Zd", e_balanceSum);
    pcs_encrypt(CSKeys->pubKey, CSKeys->hcsRandom, e_balanceSum, e_balanceSum);


// Step 3 :
    // *cryptoKey = (struct CryptoKey*)CSKeys
    *cryptoKey = CSKeys;
}


void udf_add(int i, struct CryptoKey** cryptoKey){

    int balance = arr[i];

// Setup a 2048-bit for each balance
    mpz_t e_Balance;
    mpz_init(e_Balance);
    mpz_set_ui(e_Balance, balance);

// Encrypt each balance
    pcs_encrypt((*cryptoKey)->pubKey, (*cryptoKey)->hcsRandom, e_Balance, e_Balance);

// Operation of addition
    pcs_ee_add((*cryptoKey)->pubKey, e_balanceSum, e_balanceSum, e_Balance);
    gmp_printf("Current Sum = %Zd\n",e_balanceSum);

    mpz_clear(e_Balance);
}


int main(void){

// Step 1 : Initialize crypto-system Key
    struct CryptoKey* cryptoKey = NULL;
    mpz_init(e_balanceSum);

    udf_init(&cryptoKey); // Initialize data structure using udf_init


// Step 2 : Starts operation
    for(int i = 0; i<20; i++) {
        udf_add(i, &cryptoKey);
    }


// Step 3 : Display operation results
        pcs_decrypt(cryptoKey->privKey, e_balanceSum, e_balanceSum);
        gmp_printf("Final Sum = %Zd\n", e_balanceSum);


// Step 4 : Cleanup all Data and Key
    mpz_clear(e_balanceSum);
    pcs_free_public_key(cryptoKey->pubKey);
    pcs_free_private_key(cryptoKey->privKey);
    hcs_free_random(cryptoKey->hcsRandom);
    free((struct CryptoKey*)cryptoKey);

    return 0;
}
