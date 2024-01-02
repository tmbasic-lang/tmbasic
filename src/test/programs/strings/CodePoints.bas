sub Main()
    dim x = CodePoints("💩")
    print Len(x)
    print x(0)

    print StringFromCodePoints([9819, 128169])

    ' CodePoints doc example
    dim y = CodePoints("💎")
    print Len(y)
    print y(0)

    ' StringFromCodePoints doc example
    print StringFromCodePoints([65, 128142])

    ' StringFromCodeUnits doc example
    print StringFromCodeUnits([65, 240, 159, 146, 142])
end sub
--output--
1
128169
♛💩
1
128142
A💎
A💎
