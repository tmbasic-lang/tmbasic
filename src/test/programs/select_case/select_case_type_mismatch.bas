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
This "case" expression is of type Number, which does not match the "select case" expression type String.
