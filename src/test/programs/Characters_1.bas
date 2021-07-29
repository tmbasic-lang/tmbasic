#procedure
sub Main()
    dim s = "n" + "e" + Chr(769)
    dim x = Characters(s)
    print Len(x(0))  ' x(0) is one code unit: "n"
    print Len(x(1))  ' x(1) is two code units: "e" + Chr(769)
end sub
--output--
1
2
