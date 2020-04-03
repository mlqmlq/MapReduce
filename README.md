# MapReduce
In 2004, engineers at Google introduced a new paradigm for large-scale parallel data processing known as MapReduce. One key aspect of MapReduce is that it makes programming tasks on large-scale clusters easy for developers; instead of worrying about how to manage parallelism, handle machine crashes, and many other complexities common within clusters of machines, the developer can instead just focus on writing little bits of code (described below) and the infrastructure handles the rest.

In this project, I built a simplified version of MapReduce for just a single machine.

## General Idea
The MapReduce infrastructure I built supports the execution of user-defined `Map()` and `Reduce()` functions.

As from the original paper: “`Map()`, written by the user, takes an input pair and produces a set of intermediate key/value pairs. The MapReduce library groups together all intermediate values associated with the same intermediate key K and passes them to the `Reduce()` function.”

“The `Reduce()` function, also written by the user, accepts an intermediate key K and a set of values for that key. It merges together these values to form a possibly smaller set of values; typically just zero or one output value is produced per `Reduce()` invocation. The intermediate values are supplied to the user’s reduce function via an iterator.”

A classic example, written here in pseudocode, shows how to count the number of occurrences of each word in a set of documents:

```
map(String key, String value):
    // key: document name
    // value: document contents
    for each word w in value:
        EmitIntermediate(w, "1");

reduce(String key, Iterator values):
    // key: a word
    // values: a list of counts
    int result = 0;
    for each v in values:
        result += ParseInt(v);
    print key, value;
```
Apart from the `Map()` and `Reduce()` functions, there is an option to provide a third user-defined `Combine()` function, if the `Reduce()` function is commutative and associative.

The `Combine()` function does partial merging of the data emitted by a single `Mapper()`, before it is sent to the `Reduce()` function. More specifically, a `Combine()` function is executed as many times as the number of unique keys that its respective `Map()` function will produce.

Typically the functionality of `Combine()` and `Reduce()` functions can be very similar. The main difference between a `Combine()` and a `Reduce()` function, is that the former merges data from a single `Map()` function before it is forwarded to a reducer, while the latter from multiple mappers.
We can extend the previous example, by adding a `Combine()` function, as follows:
```
map(String key, String value):
    // key: document name
    // value: document contents
    for each word w in value:
        EmitPrepare(w, "1");

combine(String key, Iterator values):
    // key: a word
    // values: list of counts
    int result = 0;
    for each v in values:
        result += ParseInt(v);
    EmitIntermediate(w, result);

reduce(String key, Iterator values):
    // key: a word
    // values: a list of counts
    int result = 0;
    for each v in values:
        result += ParseInt(v);
    print key, value;
```
What’s fascinating about MapReduce is that so many different kinds of relevant computations can be mapped onto this framework. The original paper lists many examples, including word counting (as above), a distributed grep, a URL frequency access counters, a reverse web-link graph application, a term-vector per host analysis, and others.

What’s also quite interesting is how easy it is to parallelize: many mappers can be running at the same time, and, many reducers can be running at the same time. Users don’t have to worry about how to parallelize their application; rather, they just write `Map()`, `Combine()` and `Reduce()` functions and the infrastructure does the rest.

## My Code
The code I implemented is in `mapreduce.c`, which include `MR_EmitToCombiner()`, `MR_EmitToReducer()`, and `MR_Run()`. You should also have implemented two versions of `get_next()` function (one for the `Combiner()` and one for the `Reducer()`).

## Compile
```
gcc -o mapreduce test.c mapreduce.c mapreduce.h -Wall -Werror -pthread -O
```

## Example
```
./mapreduce 1.txt 2.txt 3.txt 4.txt
```
It will launch the MapReduce infracture and do the wordcount using multiple threads.
