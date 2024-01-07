sub Main()
    dim d = DateFromParts(2015, 4, 5)
    print AddDays(d, $1)
end sub
--output--
$2
--cases--
1|1|2015-04-06
2|-1|2015-04-04
3|0|2015-04-05
4|-5|2015-03-31
