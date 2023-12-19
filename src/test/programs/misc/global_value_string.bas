#global
dim x as String
#procedure
sub Main()
    print x
    x = "hello"
    x = x + " world"
    print x
end sub
--output--

hello world
