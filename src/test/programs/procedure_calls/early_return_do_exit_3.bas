#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    do
        ' This inner loop is a red herring. It doesn't matter what happens in this loop...
        do
            ' ... in particular this Exit Do doesn't matter ...
            exit do
        loop while x = 6

        ' ... because eventually it makes it to this unconditional throw.
        throw 999, "blah"
    loop while x = 5
end function

--output--
Error
999
blah
