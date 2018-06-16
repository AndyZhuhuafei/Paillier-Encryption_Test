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
The auto-generated documentation for **libhcs** can be found [here](https://tiehuis.github.io/libhcs).
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
#include <gmp.h>    // gmp is included implicitly
#include <libhcs.h> // master header includes everything

int main(void)
{
    // initialize data structures
    pcs_public_key *pk = pcs_init_public_key();
    pcs_private_key *vk = pcs_init_private_key();
    hcs_random *hr = hcs_init_random();

    // Generate a key pair with modulus of size 2048 bits
    pcs_generate_key_pair(pk, vk, hr, 2048);

    // libhcs works directly with gmp mpz_t types, so initialize some
    mpz_t a, b, c;
    mpz_inits(a, b, c, NULL);

    mpz_set_ui(a, 50);
    mpz_set_ui(b, 76);

    pcs_encrypt(pk, hr, a, a);  // Encrypt a (= 50) and store back into a
    pcs_encrypt(pk, hr, b, b);  // Encrypt b (= 76) and store back into b
    gmp_printf("a = %Zd\nb = %Zd\n", a, b); // can use all gmp functions still

    pcs_ee_add(pk, c, a, b);    // Add encrypted a and b values together into c
    pcs_decrypt(vk, c, c);      // Decrypt c back into c using private key
    gmp_printf("%Zd\n", c);     // output: c = 126

    // Cleanup all data
    mpz_clears(a, b, c, NULL);
    pcs_free_public_key(pk);
    pcs_free_private_key(vk);
    hcs_free_random(hr);

    return 0;
}
```

To run this example, we need only need to link against libhcs and libgmp:

    gcc -o example example.c -lhcs -lgmp
    ./example

