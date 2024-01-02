sub Main()
    dim x = "A"
    select case x
        case "A"
            print "yes"
        case 5
            print "no"
        case else
            print "no"
    end select
end sub
--output--
Compiler error
kTypeMismatch
Main
6:14
