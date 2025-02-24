### memo

<hr>

#### The order of path
The order of the displayed path and the path in the internal data structure are **REVERSED**,
because when using the C++ vector type as a path data structure, it takes less time to add to the end (using the push_back function) rather than adding to the head.

#### ASPA data of exported YAML file
When exporting to a file, ASPA information is **not** included by default. Thus, it will not work if imported in the original LOTUS implementation (by han9umeda).
It will work by putting ``ASPA: {}`` to the .yml file to indicate that there is no ASPA.

#### parallel processing
Parallel processing is available using OpenMP.
This is useful when creating multiple instances of LOTUS, giving each one an initial condition, and executing them with ``LOTUS.run()``.

<hr>

#### pathの順序
表示されるpathと、内部データ構造のpathでは順序が**逆**になっている。
これはC++のvector型を扱う際に、先頭に追加するのではなく後ろに追加（push_back関数）する方が実行時間が短いためである。


#### 出力YAMLファイルのASPA
このプログラムでファイルに出力する際、デフォルトではASPA情報を出力しない。そのため（han9umedaによる）元のLOTUSの実装においてインポートしても動作**しない**。
.ymlファイルにASPAが無いことを示す ``ASPA: {}`` と入れると動作する。

#### 並列処理
OpenMPによる並列処理ができる。
LOTUSのインスタンスを複数作成し、それぞれに初期条件を与えて ``LOTUS.run()`` で実行するなどの処理を行う際に有用。
