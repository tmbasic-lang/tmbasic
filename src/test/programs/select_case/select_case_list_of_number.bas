sub Main()
    dim x = [2, 3, 4]
    select case x
        case [1, 2, 3]
            print "no"
        case [2, 3, 4]
            print "yes"
        case else
            print "no"
    end select
end sub
--output--
yes
