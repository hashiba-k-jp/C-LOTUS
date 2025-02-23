### memo

<hr>

#### The order of path
The order of the displayed path and the path in the internal data structure are **REVERSED**,
because when using the C++ vector type as a path data structure, it takes less time to add to the end (using the push_back function) rather than adding to the head.

<hr>

#### pathの順序

表示されるpathと、内部データ構造のpathでは順序が**逆**になっている。
これはC++のvector型を扱う際に、先頭に追加するのではなく後ろに追加（push_back関数）する方が実行時間が短いためである。
