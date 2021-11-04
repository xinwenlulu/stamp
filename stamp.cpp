#include <iostream>
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <fstream>
#include "stamp.h"

using namespace std;

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

// helper function for internal use only
// transforms raw binary hash value into human-friendly hexademical form
void convert_hash(const unsigned char *str, char *output, int hash_length) {
  char append[16];
  strcpy (output, "");
  for (int n=0; n<hash_length; n++) {
    sprintf(append,"%02x",str[n]);
    strcat(output, append);
  }
}

// pre-supplied helper function
// generates the SHA1 hash of input string text into output parameter digest
// ********************** IMPORTANT **************************
// ---> remember to include -lcrypto in your linking step <---
// ---> so that the definition of the function SHA1 is    <---
// ---> included in your program                          <---
// ***********************************************************
void text_to_SHA1_digest(const char *text, char *digest) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1( (const unsigned char *) text, strlen(text), hash);
  convert_hash(hash, digest, SHA_DIGEST_LENGTH);
}

/* add your function definitions here */

int leading_zeros(const char* digest){
  bool leading = true;
  int count = 0;

  for (int i = 0; i < strlen(digest); i++){
    if (isxdigit(digest[i])==0){ return -1; }
    if (i == 0 && *digest != '0'){ leading = false; }
    if (digest[i] == '0'){
      if (leading == true){ count++; }
    }else{ 
      leading = false;
    }
  }
  return count;
}

bool file_to_SHA1_digest(const char* filename, char* digest){
  ifstream in(filename);
  if(!in){ 
    cout << "Failed opening file! " << endl;
    strcpy(digest,"error");
    return false; 
  }
  char buffer[100000];
  char c;
  int i = 0;
  while(in.get(c)){
    buffer[i] = c;
    i++;
  }
  in.close();
  text_to_SHA1_digest(buffer, digest);
  return true;
}


bool make_header(const char* recipient, const char* filename, char* header){
  int counter = 313889;
  strcpy(header,recipient);
  strcat(header,":");
  char digest[41];
  if (!file_to_SHA1_digest(filename, digest)){ return false; }
  strcat(header,digest);
  strcat(header,":");
  char counterchar[100];
  sprintf(counterchar, "%d", counter);
  strcat(header,counterchar);

  char headerdigest[41];
  text_to_SHA1_digest(header, headerdigest);
  
  while (leading_zeros(headerdigest)!=5 && counter <= 10000000){ 
    counter++;
    strcpy(header,recipient);
    strcat(header,":");
    strcpy(digest,"");
    if (!file_to_SHA1_digest(filename, digest)){ return false; }
    strcat(header,digest);
    strcat(header,":");
    strcpy(counterchar,"");
    sprintf(counterchar, "%d", counter);
    strcat(header,counterchar);
    strcpy(headerdigest,"");
    text_to_SHA1_digest(header, headerdigest);
  }
  if (counter >= 10000000){ return false; }
  return true;
}

MessageStatus check_header(const char* email, const char* header, const char* filename){
  int count = 0, sep[2] = {0,0};
  for(int i = 0; i < strlen(header); i++){
    if (header[i]==':'){
      sep[count] = i;
      count++;
    }
  }
  if (count != 2){ return INVALID_HEADER; }

  char buffer[41];
  for(int i = 0; i<sep[0]; i++){
    buffer[i] = header[i];
  }
  buffer[sep[0]]= '\0';
  int compare = strcmp(buffer,email);
  if (compare!=0){ return WRONG_RECIPIENT; }

  
  for(int i = sep[0]+1; i<sep[1]; i++){
    buffer[i-sep[0]-1] = header[i];
  }
  buffer[sep[1]-sep[0]-1]= '\0';

  char thisdigest[41];
  //cout << filename << endl;; 
  file_to_SHA1_digest(filename, thisdigest);
  if(strlen(thisdigest) == 41){
    compare = strcmp(buffer,thisdigest);
    if (compare!=0){ return INVALID_MESSAGE_DIGEST; }
  }
  

  strcpy(thisdigest,"");
  text_to_SHA1_digest(header, thisdigest);
  if (leading_zeros(thisdigest)!=5){ return INVALID_HEADER_DIGEST; }

  return VALID_EMAIL;
}
