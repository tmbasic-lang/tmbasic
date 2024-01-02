sub Main()
    dim x = "D"
    select case x
        case "A", "B"
            print "no"
        case "C", "D"
            print "yes"
        case else
            print "no"
    end select
end sub
--output--
yes
