sub Main()
    dim x = "A"
    select case x
        case else
            print "0"
        case "A"
            print "1"
        case "B"
            print "2"
        case else
            print "3"
    end select
end sub
--output--
Compiler error
kMultipleSelectCaseDefaults
Main
10:9
