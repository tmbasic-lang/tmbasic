sub Main()
    dim a = DateFromParts(2000, 5, 6)
    dim b = DateFromParts(2000, 5, 7)
    
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
