sub Main()
    dim d = DateFromParts(2015, 4, 5)
    print AddMonths(d, $1)
end sub
--output--
$2
--cases--
1|1|2015-05-05
2|-1|2015-03-05
3|0|2015-04-05
4|-12|2014-04-05
5|12|2016-04-05
