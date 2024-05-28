sub Main()
    print Days(1)
    print Days(-1)

    print Hours(1)
    print Hours(-1)

    print Minutes(1)
    print Minutes(-1)

    print Seconds(1)
    print Seconds(-1)

    print Milliseconds(1)
    print Milliseconds(-1)
end sub

--output--
1:00:00:00.000
-1:00:00:00.000
0:01:00:00.000
-0:01:00:00.000
0:00:01:00.000
-0:00:01:00.000
0:00:00:01.000
-0:00:00:01.000
0:00:00:00.001
-0:00:00:00.001
