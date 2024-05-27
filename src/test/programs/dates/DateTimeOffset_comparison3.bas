sub Main()
    ' These are the same UTC time!
    dim a = DateTimeOffsetFromParts(2000, 5, 6, 13, 30, 45, 999, Hours(-5))
    dim b = DateTimeOffsetFromParts(2000, 5, 6, 12, 30, 45, 999, Hours(-6))
    
    if a < a then print "a<a"
    if a < b then print "a<b"
    if b < a then print "b<a"
    
    if a <= a then print "a<=a"
    if a <= b then print "a<=b"
    if b <= a then print "b<=a"

    if a > a then print "a>a"
    if a > b then print "a>b"
    if b > a then print "b>a"

    if a >= a then print "a>=a"
    if a >= b then print "a>=b"
    if b >= a then print "b>=a"

    if a = a then print "a=a"
    if a = b then print "a=b"
    if b = a then print "b=a"

    if a <> a then print "a<>a"
    if a <> b then print "a<>b"
    if b <> a then print "b<>a"
end sub

--output--
a<=a
a<=b
b<=a
a>=a
a>=b
b>=a
a=a
a=b
b=a
