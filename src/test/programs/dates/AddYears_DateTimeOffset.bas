sub Main()
    dim d = DateTimeOffsetFromParts(2021, 3, 12, 14, 30, 59, 488, Hours(-4))
    print AddYears(d, $1)
end sub
--output--
$2
--cases--
1|1|2022-03-12 14:30:59.488 -04:00
2|-1|2020-03-12 14:30:59.488 -04:00
3|0|2021-03-12 14:30:59.488 -04:00
