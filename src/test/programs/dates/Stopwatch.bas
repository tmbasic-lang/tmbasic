sub Main()
    dim a = Stopwatch()
    dim b = Stopwatch()
    dim c as TimeSpan
    c = b - a
    if b >= a then print "ok"
end sub

--output--
ok
