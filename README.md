# Dictionary Implementation in C
This project is a simple implementation of a dictionary data structure in C. The dictionary is implemented using an array of rows, with each row containing a number of key-value pairs.

### How to Use
To use this dictionary in your own C program, simply include the libDict.h header file in your code and link against the libDict.a library.

### How it Works
The dictionary is implemented using an array of rows, where each row is a hash table containing a number of key-value pairs. The hash function used is a simple algorithm that hashes a sequence of bytes by multiplying each byte by a large prime number and summing the results.

The dictionary can grow dynamically as new keys are added to it. If a row becomes too full, a new row is added to the array and the keys are rehashed into the new row. If the entire dictionary becomes too full, it is resized by increasing the number of rows.

