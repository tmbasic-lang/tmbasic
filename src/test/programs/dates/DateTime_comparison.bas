sub Main()
    dim a = DateTimeFromParts(2000, 5, 6, 12, 30, 45, 998)
    dim b = DateTimeFromParts(2000, 5, 6, 12, 30, 45, 999)
    
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
a<b
a<=a
a<=b
b>a
a>=a
b>=a
a=a
a<>b
b<>a
