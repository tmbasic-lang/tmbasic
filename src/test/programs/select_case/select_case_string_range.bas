sub Main()
    dim x = "A"
    select case x
        case "A" to "B"
            print "yes"
        case else
            print "no"
    end select
end sub
--output--
Compiler error
kTypeMismatch
Main
4:14
Ranges in "case" expressions must be numbers, but this range's start value is of type String.
