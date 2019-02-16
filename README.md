
# SuffixTree

A C++ implementation of Ukkonen's algorithm.  With sincerest thanks to:

- Brenden Kokoszka for an excellent [ELM implementation](https://github.com/brenden/ukkonen-animation) and [hosted animation](http://brenden.github.io/ukkonen-animation/).
- A very thorough explanation on [Stack Overflow](https://stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english/9513423#9513423).


## Stack Size Tech Debt

Heavy use of recursion.  Larger inputs may require increased stack size.  Eg on Mac `ulimit -s hard`.

### Build and run

An out-of-source CMake build.  On *nix...

```bash
cd cmake-build
cmake ..
make
./SuffixTree
```