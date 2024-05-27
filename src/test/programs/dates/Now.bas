' It's possible for time to go backwards in real life, since we are using the system clock.
' But just assume it won't happen during tests.
' Also assume we won't run this test at midnight.

function TimeToMilliseconds(d as DateTimeOffset) as Number
    return Hour(d) * 3600000 + Minute(d) * 60000 + Second(d) * 1000 + Millisecond(d)
end function

sub Main()
    dim a = TimeToMilliseconds(Now())

    for i = 1 to 100
        dim b = TimeToMilliseconds(Now())
        if a > b then print "Time went backwards!"
    next

    print "OK"
end sub

--output--
OK
