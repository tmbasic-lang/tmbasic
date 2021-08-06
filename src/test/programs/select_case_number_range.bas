#procedure
sub Main()
    for i = -1 to 6
        select case i
            case -1 to 1, 4 to 5
                print i; ": -1 to 1, 4 to 5"
            case 2 to 3
                print i; ": 2 to 3"
            case else
                print i; ": else"
        end select
    next
end sub
--output--
-1: -1 to 1, 4 to 5
0: -1 to 1, 4 to 5
1: -1 to 1, 4 to 5
2: 2 to 3
3: 2 to 3
4: -1 to 1, 4 to 5
5: -1 to 1, 4 to 5
6: else
