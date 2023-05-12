# Introduction

- This repo contains a simple client and server KeyValue application.
- IPC is done via shared-memory and semaphore.

# Requirements
  
- C++20

# How to use

- make sure that you're in the project directory.
- run the following command line to build the whole project

```
make -j8
```

- to run unittests, run

```
./unittests
```

- run the following command to start `KvServer` with 100 buckets
```
./KvServer -s 100
```

- run the following command to start `KvClient`
```
./KvClient -n 1000 -k 111
```

- this command will run `KvClient` 1000 rounds, each round will operate with key "111" and will do the following:
  - insert a random value to key "111"
  - get value stored at key "111" and compare
  - insert another random value to key "111"
  - get value stored at key "111" and compare
  - delete key "111"
  - get value stored at key "111" and compare (shoud be empty)

- to run multiple clients simultanously, run e.g. the following line
```
for i in {1..100};
do
    ./KvClient -k $i -n 1000 &
done
```

- After all tests are done, run the following command to signal `KvServer` to exit 
```
./KvClient -e
```


# Usage

- `KvServer` usage:
```
Key Value server
Usage:
  ./KvServer [OPTION...]

  -s, --size arg  size of HashTable
  -h, --help      Help
```

- `KvClient` usage:
```
Key Value client
Usage:
  ./KvClient [OPTION...]

  -n, --numRound arg  number of rounds the client will run
  -k, --key arg       key that this client will operate on
  -e, --exit          signal the server to exit. This proram will terminate 
                      immediately.
  -h, --help          Help
```

# References

- https://spcl.inf.ethz.ch/Teaching/2020-pp/lectures/PP-l18-BeyondLocksIII.pdf
- art of multiprocessor programming: chapter 8.3 (https://cs.ipm.ac.ir/asoc2016/Resources/Theartofmulticore.pdf)
- https://stackoverflow.com/questions/57706952/does-stdshared-mutex-favor-writers-over-readers
- https://opensource.com/sites/default/files/gated-content/inter-process_communication_in_linux.pdf