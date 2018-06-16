# Paillier-Encryption_Test

This Test for Paillier code is built for further implementation on UDF (mysql user-defined functions).

## Dependencies

### GMP library
First, there is a dependency on the [GMP](https://gmplib.org/) library. This also
means that some familiarity with this library is required. A useful manual can
be found on the website linked above.

The [CMake](http://www.cmake.org/) build system is used to generate the
required build files. CMake 2.8 or greater is currently required.

The [Catch](https://github.com/philsquared/Catch) unit testing framework is
used to test all functions. This however is packaged in this repository, so it
is not required to obtain yourself.

To obtain the needed requirements on Ubuntu 15.10, one may run the following
command:

    sudo apt-get install libgmp-dev cmake

### HCS Library 
Secondly, this implementation also uses libhcs , a C library implementing a number of partially homormophic encryption
schemes. 
The auto-generated documentation and source code for **libhcs** by [tiehuis](https://github.com/tiehuis) can be found [here](https://tiehuis.github.io/libhcs).
Currently the following are implemented:

* Paillier
* Damgard-Jurik
* El-Gamal

Assuming all dependencies are on your system, the following will work on a typical linux system.

    git clone https://github.com/Tiehuis/libhcs.git
    cmake .
    make
    sudo make install # Will install to /usr/local by default

To uninstall all installed files, one can run the following command:

    sudo xargs rm < install_manifest.txt

## Code 
```c
#include <stdio.h>
#include <stdlib.h>

/* Require homomorphic cryptosystem and gmp library*/
#include <libhcs.h>
#include <gmp.h>

/*
 * Mark : Declaration of keys and global variables
 */

struct CryptoKey {	// a structure definition to store Cryptosystem Key
    pcs_public_key *pubKey;
    pcs_private_key *privKey;
    hcs_random *hcsRandom;
};

mpz_t e_balanceSum; // global variable for saving encrypted Sum of all balance

int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9 ,10, 11, 12, 13, 14, 15, 16 ,17 ,18, 19}; // balance value for testing

/*
 * Mark : Declaration of User-Defined Functions (UDF)
 */

void udf_init(struct CryptoKey** cryptoKey);
void udf_add(int i, struct CryptoKey** cryptoKey);
//void udf_clear();
//void udf_reset();


/*
 * Mark : Definition of User-Defined Functions (UDF)
 */

void udf_init(struct CryptoKey** cryptoKey){

// Step 1 : Initialize the one Crypto-System Key
    struct CryptoKey *CSKeys = (struct CryptoKey*)malloc(sizeof(struct CryptoKey));
    CSKeys->pubKey = pcs_init_public_key();
    CSKeys->privKey = pcs_init_private_key();
    CSKeys->hcsRandom = hcs_init_random();

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

```

To run this example, we need only need to link against libhcs and libgmp:

    gcc -o example example.c -lhcs -lgmp
    ./example

