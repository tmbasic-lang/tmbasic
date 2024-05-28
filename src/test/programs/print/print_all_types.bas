type MyType
    Foo as Number
end type

sub Main()
    dim x as $1
    print x
end sub

--output--
$2
--cases--
1|Boolean|false
2|Number|0
3|Date|0000-00-00
4|DateTime|0000-00-00 00:00:00.000
5|DateTimeOffset|0000-00-00 00:00:00.000 +00:00
6|TimeSpan|0:00:00:00.000
7|TimeZone|UTC
8|String|
9|List of Number|[]
10|List of String|[]
11|Map from Number to Number|Map {}
12|Map from Number to String|Map {}
13|Map from String to Number|Map {}
14|Map from String to String|Map {}
15|Set of Number|Set []
16|Set of String|Set []
17|Record (Foo as Number)|{ Foo: 0 }
18|MyType|{ Foo: 0 }
19|Optional Number|
20|Optional String|
