# C-LOTUS

![](https://img.shields.io/badge/-C++-00599C.svg?logo=cplusplus)
![](https://img.shields.io/badge/-Work%20In%20Progress-orange)

<hr>

**[LOTUS](https://github.com/han9umeda/LOTUS) written in C++.**

LOTUS stands for Lightweight rOuTing simUlator with aSpa, and is originally implemented by [han9umeda](https://github.com/han9umeda).

In this implementation,
- the almost all objects (e.g. routes, connections, paths, etc...) are defined as structures, not dict.
- all features is implemented as classes, and interactive use is **not** implemented as in the original one.
- may be faster than the implementation in python.


<hr>

**[LOTUS](https://github.com/han9umeda/LOTUS) のC++による再実装**

LOTUS は Lightweight rOuTing simUlator with aSpa の略であり、元は [han9umeda](https://github.com/han9umeda) によって実装されている。

この実装では、
- ほとんどすべてのオブジェクト（ルート、接続、経路情報など）は、pythonのdict型ではなく構造体として定義されている。
- すべての機能はクラスとして定義され、元の実装のようにインタラクティブな使用は**できない**。
- C++なのでpythonより実行時間が短い。
