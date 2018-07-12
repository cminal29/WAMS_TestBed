/*
 * enc.h
 *
 *  Created on: 16-Mar-2018
 *      Author: minal
 */

#ifndef ENC_H_
#define ENC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
/*NID_X9_62_prime256v1*/
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sys/time.h>

#define AES_BLOCK_SIZE 16


void die(char *reason)
{
    fprintf(stderr, reason);
    fflush(stderr);
    exit(1);
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

int EC_DH(unsigned char **secret, EC_KEY *key, const EC_POINT *pPub)
{
    int secretLen;

    secretLen = EC_GROUP_get_degree(EC_KEY_get0_group(key));
    secretLen = (secretLen + 7) / 8;

    *secret = (unsigned char *)malloc(secretLen);
    if (!(*secret))
        die("Failed to allocate memory for secret.\n");
    secretLen = ECDH_compute_key(*secret, secretLen, pPub, key, NULL);

    return secretLen;
}

/*Key generation function for throwaway keys.*/
EC_KEY* gen_key(void)
{
    EC_KEY *key;

    key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (key == NULL)
        die("Failed to create lKey object.\n");

    if (!EC_KEY_generate_key(key))
        die("Failed to generate EC key.\n");

    return key;
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  EVP_CIPHER_CTX_set_padding(&ctx, 0);
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

unsigned char *decrypt_new (unsigned char *key,
         unsigned char *iv,
         unsigned char *encryptedData,
               int encryptedLength)
{
    // Initialisation
    EVP_CIPHER_CTX *cryptCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(cryptCtx);
    int decryptedLength = 0;
	printf("\n Encrypted length is :-%d",encryptedLength);
    int allocateSize = encryptedLength * sizeof(unsigned char);
    int lastDecryptLength = 0;
   // int plaintext_len=0;
    unsigned char *decryptedData = (unsigned char *) malloc (allocateSize);
	printf("\n Allocate size for decrypted data is:- %d",allocateSize);
    memset(decryptedData, 0x00, allocateSize);
    int i;
   
    int decryptResult = EVP_DecryptInit_ex(cryptCtx,
        EVP_aes_256_cbc(), NULL, key, iv);
    
    if (decryptResult == 1)
    {
        decryptResult = EVP_DecryptUpdate(cryptCtx, decryptedData,
            &decryptedLength, encryptedData, encryptedLength);
	printf ("Decrypted size: %d\n", decryptedLength);
    
        if (decryptResult == 1)
        {
            // Stick the final data at the end of the last
            // decrypted data.
	    //plaintext_len=decryptedLength;
	    //lastDecryptLength += decryptedLength;
            //printf("\n Last decrypted length :-%d",lastDecryptLength);
            
	        EVP_DecryptFinal_ex(cryptCtx,
                decryptedData + lastDecryptLength,
                &lastDecryptLength);
		 decryptedLength = decryptedLength + lastDecryptLength;
            
	    //int len=decryptedData[lastDecryptLength-1] = '\0';	
           
	
        }
        else
        {
            printf ("EVP_DeccryptUpdate failure.\n");
        }
    }
    else
    {
        printf ("EVP_DecryptInit_ex failure.\n");
    }
    EVP_CIPHER_CTX_free(cryptCtx);
    EVP_cleanup();
    return decryptedData;
}



#endif /* ENC_H_ */
