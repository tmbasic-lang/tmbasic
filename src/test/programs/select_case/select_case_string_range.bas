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
