#procedure
sub Main()
    dim poopEmoji = "💩"  ' this is a surrogate pair with two code units
    dim poopCodeUnits = CodeUnits(poopEmoji)
    print Len(poopCodeUnits)
    print StringFromCodeUnits(poopCodeUnits)
end sub
--output--
2
💩
