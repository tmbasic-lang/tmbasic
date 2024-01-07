sub Main()
    dim d = DateFromParts(2015, 4, 5)
    print AddYears(d, $1)
end sub
--output--
$2
--cases--
1|1|2016-04-05
2|-1|2014-04-05
3|0|2015-04-05
