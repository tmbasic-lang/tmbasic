#procedure
sub Main()
    dim myList = [1, 2, 3, 4, 5, 6, 7]
    dim newList = Skip(myList, 3)
    for each element in newList
        print element
    next
end sub
--output--
4
5
6
7
