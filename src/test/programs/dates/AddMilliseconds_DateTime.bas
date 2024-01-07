sub Main()
    dim d = DateTimeFromParts(2015, 4, 5, 14, 30, 59, 488)
    print AddMilliseconds(d, $1)
end sub
--output--
$2
--cases--
1|1|2015-04-05 14:30:59.489
2|-1|2015-04-05 14:30:59.487
3|0|2015-04-05 14:30:59.488
4|-1000|2015-04-05 14:30:58.488
