# Map Reduce Perfect Powers

## 🚀 What is it?

This project is a C algorithm using pthreads that iterates through thousands of files and computes how many non-zero positive [perfect powers](https://en.wikipedia.org/wiki/Perfect_power) there are in said files.

It does this using the MapReduce programming model popularized by Google and used to process very large data sets in distributed systems, as explained in detail in [this](http://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf) paper.

I worked on this for three days during November 2022 and learned a lot about multithreading while doing so.

## 🔧 How to run it?

### Method 1)

1.  Clone this repository.
2.  Run the automated testing script using `./test.sh` (I did not make the script myself). It will run the tests present in `test0/` - `test4/` and compare the results to those present in `out` files.
3.  Enjoy!

### Method 2)

1.  Clone this repository.
2.  Compile the code manually using `gcc -o tema1 tema1.c -lpthread -lm` and run it using `./tema1 M R test.txt`, where `M` is the number of mapper threads, `R` is the number of reducer threads and `test.txt` is the file that lists the files where the numbers are stored.
3.  For example, if you use `./test 4 2 test0/test.txt`, it will create 2 files, `out2.txt` and `out3.txt`, where it will compute how many positive perfect squares and perfect cubes there are in files `test0/in1.txt` - `test0/in4.txt`, using 4 mapper threads (each mapper thread will read and select the numbers in one file). In this case, it will output `3` `(1, 9, 81)` and `2` `(1, 27)`.
4.  Additionally, you can create your own test and input files for yourself.
5.  Enjoy!

## 📏 What does it do?

As explained in the paper mentioned above, it uses two types of threads, mappers and reducers, to do very specific tasks.

Each mapper thread opens files that have not yet been opened, reads all the numbers inside and adds the perfect powers found inside to dynamically allocated arrays, one array for each number from `2` to `R`, where `R` is the number of reducer threads.

After the mappers are done, each reducer starts combining the partial lists mappers have computed for a certain value, removing the duplicates and counting up the values left. So if there are 3 reducers, the first one will compute the number of 2-perfect powers, the second one will compute the number of 3-perfect powers, and the last one will compute the number of 4-perfect powers.

## 💡 How did I do it?

There were two challenges to bringing the idea to life.

The first one was how to synchronize the threads, and what I did was use only one thread function for both mappers and reducers, check with two if's whether the thread is a mapper or reducer for it to do mapper or reducer stuff, and have a barrier between the two if's that all threads have to reach. This way, when both mappers and reducers reach the barrier (that is, when mapper processes have finished), reducer processes can start.

The second one was detecting n-perfect powers. The way I was initially doing it was starting from `1`, calculating `1^n`, checking whether it is equal to the number checked, and if it is not incrementing `1` to `2`, computing `2^n`, and so on until I either find the checked number or a number larger than the checked number.

However, I quickly realized that on large numbers it is very, very slow. This totally changed my approach, and I got the idea to compute all perfect powers smaller than `2^31` in what I termed a power matrix, and when I wanted to check whether a number is an n-perfect power, simply binary search the number in the `n - 2`'th line of the matrix.

This approach is much faster and works as intended.

## 🤔 Did you know?

It can only compute up to 33-perfect powers and only numbers that are smaller than `2^31`, so be careful.
